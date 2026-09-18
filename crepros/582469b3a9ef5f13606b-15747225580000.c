// https://syzkaller.appspot.com/bug?id=e70bc18a510ec08a3ffc89c4ba5fe081b1f6efa2
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <glob.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <linux/rtnetlink.h>
#include <stdint.h>
#include <sys/mount.h>

#define HWSIM_CMD_NEW_RADIO 4
#define NL80211_CMD_NEW_INTERFACE 7
#define NL80211_ATTR_WIPHY 1
#define NL80211_ATTR_IFNAME 4
#define NL80211_ATTR_IFTYPE 5
#define NL80211_IFTYPE_STATION 2

int resolve_genl_family(int fd, const char *name) {
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;
    struct sockaddr_nl nladdr;
    
    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = GENL_ID_CTRL;
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.g.cmd = CTRL_CMD_GETFAMILY;
    req.g.version = 1;
    
    struct rtattr *rta = (struct rtattr *)(((char *)&req.n) + NLMSG_ALIGN(req.n.nlmsg_len));
    rta->rta_type = CTRL_ATTR_FAMILY_NAME;
    rta->rta_len = RTA_LENGTH(strlen(name) + 1);
    strcpy(RTA_DATA(rta), name);
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + RTA_ALIGN(rta->rta_len);
    
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    
    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
        return -1;
    }
    
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[4096];
    } ans;
    
    int len = recv(fd, &ans, sizeof(ans), 0);
    if (len < 0) return -1;
    
    if (ans.n.nlmsg_type == NLMSG_ERROR) return -1;
    
    struct rtattr *attr = (struct rtattr *)(((char *)&ans.n) + NLMSG_ALIGN(sizeof(struct nlmsghdr)) + GENL_HDRLEN);
    int attrlen = ans.n.nlmsg_len - NLMSG_ALIGN(sizeof(struct nlmsghdr)) - GENL_HDRLEN;
    
    while (RTA_OK(attr, attrlen)) {
        if (attr->rta_type == CTRL_ATTR_FAMILY_ID) {
            return *(uint16_t *)RTA_DATA(attr);
        }
        attr = RTA_NEXT(attr, attrlen);
    }
    return -1;
}

void create_hwsim_radio(void) {
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd < 0) {
        printf("[-] Failed to create netlink socket for hwsim: %s\n", strerror(errno));
        return;
    }
    
    int family = resolve_genl_family(fd, "MAC80211_HWSIM");
    if (family < 0) {
        printf("[-] MAC80211_HWSIM family not found.\n");
        close(fd);
        return;
    }
    
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;
    
    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = family;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.g.cmd = HWSIM_CMD_NEW_RADIO;
    req.g.version = 1;
    
    struct sockaddr_nl nladdr;
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    
    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
        printf("[-] Failed to send HWSIM_CMD_NEW_RADIO: %s\n", strerror(errno));
        close(fd);
        return;
    }
    
    char buf[4096];
    recv(fd, buf, sizeof(buf), 0);
    printf("[+] Sent HWSIM_CMD_NEW_RADIO.\n");
    close(fd);
}

void create_nl80211_interface(const char *name) {
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd < 0) {
        printf("[-] Failed to create netlink socket for nl80211: %s\n", strerror(errno));
        return;
    }
    
    int family = resolve_genl_family(fd, "nl80211");
    if (family < 0) {
        printf("[-] nl80211 family not found.\n");
        close(fd);
        return;
    }
    
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;
    
    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = family;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.g.cmd = NL80211_CMD_NEW_INTERFACE;
    req.g.version = 1;
    
    struct rtattr *rta;
    
    rta = (struct rtattr *)(((char *)&req.n) + NLMSG_ALIGN(req.n.nlmsg_len));
    rta->rta_type = NL80211_ATTR_WIPHY;
    rta->rta_len = RTA_LENGTH(4);
    *(uint32_t *)RTA_DATA(rta) = 0;
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + RTA_ALIGN(rta->rta_len);
    
    rta = (struct rtattr *)(((char *)&req.n) + NLMSG_ALIGN(req.n.nlmsg_len));
    rta->rta_type = NL80211_ATTR_IFNAME;
    rta->rta_len = RTA_LENGTH(strlen(name) + 1);
    strcpy(RTA_DATA(rta), name);
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + RTA_ALIGN(rta->rta_len);
    
    rta = (struct rtattr *)(((char *)&req.n) + NLMSG_ALIGN(req.n.nlmsg_len));
    rta->rta_type = NL80211_ATTR_IFTYPE;
    rta->rta_len = RTA_LENGTH(4);
    *(uint32_t *)RTA_DATA(rta) = NL80211_IFTYPE_STATION;
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + RTA_ALIGN(rta->rta_len);
    
    struct sockaddr_nl nladdr;
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;
    
    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
        printf("[-] Failed to send NL80211_CMD_NEW_INTERFACE: %s\n", strerror(errno));
        close(fd);
        return;
    }
    
    char buf[4096];
    recv(fd, buf, sizeof(buf), 0);
    printf("[+] Sent NL80211_CMD_NEW_INTERFACE for %s.\n", name);
    close(fd);
}

int main(void) {
    if (mount("debugfs", "/sys/kernel/debug", "debugfs", 0, NULL) == 0) {
        printf("[+] Mounted debugfs\n");
    } else {
        printf("[-] Failed to mount debugfs: %s (might already be mounted)\n", strerror(errno));
    }

    create_hwsim_radio();
    sleep(1);
    create_nl80211_interface("sta0");
    sleep(1);

    struct if_nameindex *if_ni, *i;
    int sock;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("[-] socket failed: %s\n", strerror(errno));
        exit(1);
    }

    if_ni = if_nameindex();
    if (if_ni == NULL) {
        printf("[-] if_nameindex failed: %s\n", strerror(errno));
        exit(1);
    }

    for (i = if_ni; ! (i->if_index == 0 && i->if_name == NULL); i++) {
        if (strncmp(i->if_name, "wlan", 4) == 0 || strncmp(i->if_name, "sta", 3) == 0) {
            struct ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, i->if_name, IFNAMSIZ - 1);
            if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
                ifr.ifr_flags &= ~IFF_UP;
                if (ioctl(sock, SIOCSIFFLAGS, &ifr) == 0) {
                    printf("[+] Brought down %s\n", i->if_name);
                } else {
                    printf("[-] Failed to bring down %s: %s\n", i->if_name, strerror(errno));
                }
            }
        }
    }
    if_freenameindex(if_ni);
    close(sock);

    glob_t gl;
    int found = 0;
    if (glob("/sys/kernel/debug/ieee80211/*/netdev:*/active_links", 0, NULL, &gl) == 0) {
        for (size_t j = 0; j < gl.gl_pathc; j++) {
            printf("[+] Found %s\n", gl.gl_pathv[j]);
            found = 1;
            int fd = open(gl.gl_pathv[j], O_WRONLY);
            if (fd >= 0) {
                if (write(fd, "1\n", 2) < 0) {
                    printf("[-] write 1 failed: %s\n", strerror(errno));
                }
                usleep(100000);
                if (write(fd, "2\n", 2) < 0) {
                    printf("[-] write 2 failed: %s\n", strerror(errno));
                }
                usleep(100000);
                if (write(fd, "3\n", 2) < 0) {
                    printf("[-] write 3 failed: %s\n", strerror(errno));
                }
                close(fd);
                printf("[+] Wrote to %s\n", gl.gl_pathv[j]);
            } else {
                printf("[-] Failed to open %s: %s\n", gl.gl_pathv[j], strerror(errno));
            }
        }
        globfree(&gl);
    }
    
    if (!found) {
        printf("[-] No active_links debugfs files found.\n");
    }

    // Give some time for async work to trigger warning
    sleep(2);
    return 0;
}