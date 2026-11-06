// https://syzkaller.appspot.com/bug?id=692011f092ca7b3b724dd5240f45a6e44a92b249
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/genetlink.h>
#include <linux/netlink.h>
#include <linux/devlink.h>

#ifndef DEVLINK_GENL_NAME
#define DEVLINK_GENL_NAME "devlink"
#endif

#ifndef DEVLINK_GENL_VERSION
#define DEVLINK_GENL_VERSION 1
#endif

#ifndef DEVLINK_CMD_RELOAD
#define DEVLINK_CMD_RELOAD 37
#endif

#ifndef DEVLINK_ATTR_BUS_NAME
#define DEVLINK_ATTR_BUS_NAME 1
#endif

#ifndef DEVLINK_ATTR_DEV_NAME
#define DEVLINK_ATTR_DEV_NAME 2
#endif

#ifndef NLA_HDRLEN
#define NLA_HDRLEN ((int)sizeof(struct nlattr))
#endif

#ifndef NLA_ALIGN
#define NLA_ALIGN(len) (((len) + 3) & ~3)
#endif

#define GENLMSG_DATA(glh) ((void *)(((char *)NLMSG_DATA(glh)) + GENL_HDRLEN))
#define NLA_DATA(nla) ((void *)((char *)(nla) + NLA_HDRLEN))
#define NLA_OK(nla, len) \
	((len) >= (int)sizeof(struct nlattr) && \
	 (nla)->nla_len >= sizeof(struct nlattr) && (nla)->nla_len <= (len))
#define NLA_NEXT(nla, len) \
	((len) -= NLA_ALIGN((nla)->nla_len), \
	 (struct nlattr *)(((char *)(nla)) + NLA_ALIGN((nla)->nla_len)))

int main(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	printf("[+] Starting reproducer for WARNING in bpf_offload_dev_destroy\n");

	/* 1. Mount debugfs if not already mounted */
	int res = mount("none", "/sys/kernel/debug", "debugfs", 0, NULL);
	if (res < 0 && errno != EBUSY) {
		printf("[-] mount(/sys/kernel/debug, debugfs) failed: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] debugfs is mounted at /sys/kernel/debug\n");

	/* 2. Open netlink socket to query devlink generic netlink family ID */
	int nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
	if (nl_fd < 0) {
		printf("[-] Failed to create NETLINK_GENERIC socket: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Created NETLINK_GENERIC socket successfully\n");

	struct sockaddr_nl sa;
	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;

	res = bind(nl_fd, (struct sockaddr *)&sa, sizeof(sa));
	if (res < 0) {
		printf("[-] Failed to bind NETLINK_GENERIC socket: %s\n", strerror(errno));
		close(nl_fd);
		exit(1);
	}
	printf("[+] Bound NETLINK_GENERIC socket successfully\n");

	struct sockaddr_nl kernel_sa;
	memset(&kernel_sa, 0, sizeof(kernel_sa));
	kernel_sa.nl_family = AF_NETLINK;

	struct {
		struct nlmsghdr n;
		struct genlmsghdr g;
		char buf[256];
	} getfam_req;

	memset(&getfam_req, 0, sizeof(getfam_req));
	getfam_req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	getfam_req.n.nlmsg_type = GENL_ID_CTRL;
	getfam_req.n.nlmsg_flags = NLM_F_REQUEST;
	getfam_req.n.nlmsg_seq = 1;
	getfam_req.n.nlmsg_pid = getpid();
	getfam_req.g.cmd = CTRL_CMD_GETFAMILY;
	getfam_req.g.version = 1;

	struct nlattr *nla = (struct nlattr *)GENLMSG_DATA(&getfam_req);
	int attrlen = strlen(DEVLINK_GENL_NAME) + 1;
	nla->nla_type = CTRL_ATTR_FAMILY_NAME;
	nla->nla_len = NLA_HDRLEN + attrlen;
	memcpy(NLA_DATA(nla), DEVLINK_GENL_NAME, attrlen);
	getfam_req.n.nlmsg_len += NLA_ALIGN(nla->nla_len);

	res = sendto(nl_fd, &getfam_req, getfam_req.n.nlmsg_len, 0,
		     (struct sockaddr *)&kernel_sa, sizeof(kernel_sa));
	if (res < 0) {
		printf("[-] Failed to send CTRL_CMD_GETFAMILY request: %s\n", strerror(errno));
		close(nl_fd);
		exit(1);
	}
	printf("[+] Sent CTRL_CMD_GETFAMILY request\n");

	char resp[4096];
	int len = recv(nl_fd, resp, sizeof(resp), 0);
	if (len < 0) {
		printf("[-] Failed to recv CTRL_CMD_GETFAMILY response: %s\n", strerror(errno));
		close(nl_fd);
		exit(1);
	}
	printf("[+] Received CTRL_CMD_GETFAMILY response (%d bytes)\n", len);

	int family_id = -1;
	struct nlmsghdr *nlh = (struct nlmsghdr *)resp;
	for (; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
		if (nlh->nlmsg_type == NLMSG_ERROR) {
			printf("[-] Received NLMSG_ERROR in response to GETFAMILY\n");
			close(nl_fd);
			exit(1);
		}
		if (nlh->nlmsg_type == GENL_ID_CTRL) {
			struct genlmsghdr *ghdr = NLMSG_DATA(nlh);
			struct nlattr *attr = (struct nlattr *)((char *)ghdr + GENL_HDRLEN);
			int rem = nlh->nlmsg_len - NLMSG_LENGTH(GENL_HDRLEN);
			while (NLA_OK(attr, rem)) {
				if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
					family_id = *(uint16_t *)NLA_DATA(attr);
					break;
				}
				attr = NLA_NEXT(attr, rem);
			}
		}
	}

	if (family_id < 0) {
		printf("[-] devlink generic netlink family not found\n");
		close(nl_fd);
		exit(1);
	}
	printf("[+] devlink generic netlink family found with ID %d\n", family_id);

	/* 3. Clean up any existing netdevsim0 device */
	int fd = open("/sys/bus/netdevsim/del_device", O_WRONLY);
	if (fd < 0) {
		printf("[-] Failed to open /sys/bus/netdevsim/del_device: %s\n", strerror(errno));
		close(nl_fd);
		exit(1);
	}
	res = write(fd, "0", 1);
	if (res < 0 && errno != ENOENT) {
		printf("[-] Initial cleanup write to del_device failed: %s\n", strerror(errno));
		close(fd);
		close(nl_fd);
		exit(1);
	}
	res = close(fd);
	if (res < 0) {
		printf("[-] Failed to close del_device fd: %s\n", strerror(errno));
		close(nl_fd);
		exit(1);
	}
	printf("[+] Initial netdevsim0 cleanup complete\n");
	usleep(100000);

	/* 4. Create netdevsim device with id 0, port_count 1, num_queues 1 */
	fd = open("/sys/bus/netdevsim/new_device", O_WRONLY);
	if (fd < 0) {
		printf("[-] Failed to open /sys/bus/netdevsim/new_device: %s\n", strerror(errno));
		close(nl_fd);
		exit(1);
	}
	const char *new_dev_str = "0 1 1";
	res = write(fd, new_dev_str, strlen(new_dev_str));
	if (res < 0) {
		printf("[-] Failed to write '%s' to /sys/bus/netdevsim/new_device: %s\n",
		       new_dev_str, strerror(errno));
		close(fd);
		close(nl_fd);
		exit(1);
	}
	res = close(fd);
	if (res < 0) {
		printf("[-] Failed to close /sys/bus/netdevsim/new_device: %s\n", strerror(errno));
		close(nl_fd);
		exit(1);
	}
	printf("[+] Created netdevsim0 device successfully\n");
	usleep(100000);

	/* 5. Arm reload failure in debugfs */
	fd = open("/sys/kernel/debug/netdevsim/netdevsim0/fail_reload", O_WRONLY);
	if (fd < 0) {
		printf("[-] Failed to open /sys/kernel/debug/netdevsim/netdevsim0/fail_reload: %s\n",
		       strerror(errno));
		close(nl_fd);
		exit(1);
	}
	res = write(fd, "1", 1);
	if (res < 0) {
		printf("[-] Failed to write '1' to fail_reload: %s\n", strerror(errno));
		close(fd);
		close(nl_fd);
		exit(1);
	}
	res = close(fd);
	if (res < 0) {
		printf("[-] Failed to close fail_reload: %s\n", strerror(errno));
		close(nl_fd);
		exit(1);
	}
	printf("[+] Configured netdevsim0 fail_reload = 1\n");

	/* 6. Trigger devlink reload: sets devlink->reload_failed = true */
	struct {
		struct nlmsghdr n;
		struct genlmsghdr g;
		char buf[512];
	} reload_req;

	memset(&reload_req, 0, sizeof(reload_req));
	reload_req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
	reload_req.n.nlmsg_type = family_id;
	reload_req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	reload_req.n.nlmsg_seq = 2;
	reload_req.n.nlmsg_pid = getpid();
	reload_req.g.cmd = DEVLINK_CMD_RELOAD;
	reload_req.g.version = DEVLINK_GENL_VERSION;

	char *pos = reload_req.buf;

	const char *bus_name = "netdevsim";
	int bus_len = strlen(bus_name) + 1;
	struct nlattr *nla_bus = (struct nlattr *)pos;
	nla_bus->nla_type = DEVLINK_ATTR_BUS_NAME;
	nla_bus->nla_len = NLA_HDRLEN + bus_len;
	memcpy(NLA_DATA(nla_bus), bus_name, bus_len);
	pos += NLA_ALIGN(nla_bus->nla_len);
	reload_req.n.nlmsg_len += NLA_ALIGN(nla_bus->nla_len);

	const char *dev_name = "netdevsim0";
	int dev_len = strlen(dev_name) + 1;
	struct nlattr *nla_dev = (struct nlattr *)pos;
	nla_dev->nla_type = DEVLINK_ATTR_DEV_NAME;
	nla_dev->nla_len = NLA_HDRLEN + dev_len;
	memcpy(NLA_DATA(nla_dev), dev_name, dev_len);
	pos += NLA_ALIGN(nla_dev->nla_len);
	reload_req.n.nlmsg_len += NLA_ALIGN(nla_dev->nla_len);

	res = sendto(nl_fd, &reload_req, reload_req.n.nlmsg_len, 0,
		     (struct sockaddr *)&kernel_sa, sizeof(kernel_sa));
	if (res < 0) {
		printf("[-] Failed to send DEVLINK_CMD_RELOAD request: %s\n", strerror(errno));
		close(nl_fd);
		exit(1);
	}
	printf("[+] Sent DEVLINK_CMD_RELOAD request\n");

	len = recv(nl_fd, resp, sizeof(resp), 0);
	if (len < 0) {
		printf("[-] Failed to recv reply for DEVLINK_CMD_RELOAD: %s\n", strerror(errno));
		close(nl_fd);
		exit(1);
	}
	printf("[+] Received DEVLINK_CMD_RELOAD response (%d bytes, reload failure expected)\n", len);

	res = close(nl_fd);
	if (res < 0) {
		printf("[-] Failed to close nl_fd: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Closed netlink socket\n");
	usleep(100000);

	/* 7. Add a new port while device is in reload-failed state */
	fd = open("/sys/bus/netdevsim/devices/netdevsim0/new_port", O_WRONLY);
	if (fd < 0) {
		printf("[-] Failed to open /sys/bus/netdevsim/devices/netdevsim0/new_port: %s\n",
		       strerror(errno));
		exit(1);
	}
	res = write(fd, "0", 1);
	if (res < 0) {
		printf("[-] Failed to write '0' to new_port: %s\n", strerror(errno));
		close(fd);
		exit(1);
	}
	res = close(fd);
	if (res < 0) {
		printf("[-] Failed to close new_port: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Added port 0 to netdevsim0 in reload-failed state\n");
	usleep(100000);

	/* 8. Delete netdevsim0 device to trigger nsim_drv_remove -> bpf_offload_dev_destroy */
	fd = open("/sys/bus/netdevsim/del_device", O_WRONLY);
	if (fd < 0) {
		printf("[-] Failed to open /sys/bus/netdevsim/del_device: %s\n", strerror(errno));
		exit(1);
	}
	res = write(fd, "0", 1);
	if (res < 0) {
		printf("[-] Failed to write '0' to del_device: %s\n", strerror(errno));
		close(fd);
		exit(1);
	}
	res = close(fd);
	if (res < 0) {
		printf("[-] Failed to close del_device: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] Triggered device deletion for netdevsim0\n");

	/* 9. Allow kernel warning to trigger and flush before exit */
	printf("[+] Sleeping 1 second to allow kernel warning to be recorded...\n");
	sleep(1);

	printf("[+] Reproducer finished execution successfully\n");
	return 0;
}
