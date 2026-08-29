// https://syzkaller.appspot.com/bug?id=b7b8331f5e26b5b36ae64b9632b6040c4879bb17
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#ifndef IFLA_LINKINFO
#define IFLA_LINKINFO 18
#endif
#ifndef IFLA_INFO_KIND
#define IFLA_INFO_KIND 1
#endif
#ifndef IFLA_INFO_DATA
#define IFLA_INFO_DATA 2
#endif

#ifndef IFLA_VTI_LINK
#define IFLA_VTI_LINK 1
#define IFLA_VTI_IKEY 2
#define IFLA_VTI_OKEY 3
#define IFLA_VTI_LOCAL 4
#define IFLA_VTI_REMOTE 5
#endif

void addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data, int alen) {
    int len = RTA_LENGTH(alen);
    struct rtattr *rta;
    if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) return;
    rta = (struct rtattr *)(((char *)n) + NLMSG_ALIGN(n->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;
    if (data)
        memcpy(RTA_DATA(rta), data, alen);
    else
        memset(RTA_DATA(rta), 0, alen);
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
}

int create_vti_device(const char *name, const char *local_ip, const char *remote_ip, uint32_t key) {
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) {
        printf("[-] Failed to create netlink socket: %s\n", strerror(errno));
        return -1;
    }

    struct {
        struct nlmsghdr n;
        struct ifinfomsg i;
        char buf[1024];
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK;
    req.n.nlmsg_type = RTM_NEWLINK;
    req.i.ifi_family = AF_UNSPEC;

    addattr_l(&req.n, sizeof(req), IFLA_IFNAME, name, strlen(name) + 1);

    struct rtattr *linkinfo = (struct rtattr *)(((char *)&req.n) + NLMSG_ALIGN(req.n.nlmsg_len));
    addattr_l(&req.n, sizeof(req), IFLA_LINKINFO, NULL, 0);
    addattr_l(&req.n, sizeof(req), IFLA_INFO_KIND, "vti", 4);

    struct rtattr *data = (struct rtattr *)(((char *)&req.n) + NLMSG_ALIGN(req.n.nlmsg_len));
    addattr_l(&req.n, sizeof(req), IFLA_INFO_DATA, NULL, 0);
    
    uint32_t local = inet_addr(local_ip);
    uint32_t remote = inet_addr(remote_ip);
    uint32_t key_be = htonl(key);

    addattr_l(&req.n, sizeof(req), IFLA_VTI_LOCAL, &local, 4);
    addattr_l(&req.n, sizeof(req), IFLA_VTI_REMOTE, &remote, 4);
    addattr_l(&req.n, sizeof(req), IFLA_VTI_IKEY, &key_be, 4);
    addattr_l(&req.n, sizeof(req), IFLA_VTI_OKEY, &key_be, 4);

    data->rta_len = (char *)(((char *)&req.n) + NLMSG_ALIGN(req.n.nlmsg_len)) - (char *)data;
    linkinfo->rta_len = (char *)(((char *)&req.n) + NLMSG_ALIGN(req.n.nlmsg_len)) - (char *)linkinfo;

    struct sockaddr_nl nladdr;
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
        printf("[-] Failed to send netlink message: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    char resp[1024];
    int len = recv(fd, resp, sizeof(resp), 0);
    if (len < 0) {
        printf("[-] Failed to receive netlink response: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    struct nlmsghdr *nh = (struct nlmsghdr *)resp;
    if (nh->nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nh);
        if (err->error != 0 && err->error != -EEXIST) {
            printf("[-] Netlink error: %s\n", strerror(-err->error));
            close(fd);
            return -1;
        }
    }

    close(fd);
    return 0;
}

int set_if_up(const char *ifname) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return -1;

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        close(fd);
        return -1;
    }

    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;

    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int set_if_ip(const char *ifname, const char *ip, const char *mask) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return -1;

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

    struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = inet_addr(ip);

    if (ioctl(fd, SIOCSIFADDR, &ifr) < 0) {
        close(fd);
        return -1;
    }

    if (mask) {
        sin->sin_addr.s_addr = inet_addr(mask);
        if (ioctl(fd, SIOCSIFNETMASK, &ifr) < 0) {
            close(fd);
            return -1;
        }
    }

    close(fd);
    return 0;
}

int add_route(const char *dst, int prefixlen, int ifindex) {
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) {
        printf("[-] Failed to create netlink socket for route: %s\n", strerror(errno));
        return -1;
    }

    struct {
        struct nlmsghdr n;
        struct rtmsg r;
        char buf[1024];
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
    req.n.nlmsg_type = RTM_NEWROUTE;
    
    req.r.rtm_family = AF_INET;
    req.r.rtm_dst_len = prefixlen;
    req.r.rtm_table = RT_TABLE_MAIN;
    req.r.rtm_protocol = RTPROT_BOOT;
    req.r.rtm_scope = RT_SCOPE_UNIVERSE;
    req.r.rtm_type = RTN_UNICAST;

    uint32_t daddr = inet_addr(dst);
    addattr_l(&req.n, sizeof(req), RTA_DST, &daddr, 4);
    addattr_l(&req.n, sizeof(req), RTA_OIF, &ifindex, 4);

    struct sockaddr_nl nladdr;
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
        printf("[-] Failed to send netlink message for route: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    char resp[1024];
    int len = recv(fd, resp, sizeof(resp), 0);
    if (len < 0) {
        printf("[-] Failed to receive netlink response for route: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    struct nlmsghdr *nh = (struct nlmsghdr *)resp;
    if (nh->nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nh);
        if (err->error != 0 && err->error != -EEXIST) {
            printf("[-] Netlink error for route: %s\n", strerror(-err->error));
            close(fd);
            return -1;
        }
    }

    close(fd);
    return 0;
}

void enable_tracepoint(void) {
    int fd = open("/sys/kernel/tracing/events/icmp/icmp_send/enable", O_WRONLY);
    if (fd < 0) {
        fd = open("/sys/kernel/debug/tracing/events/icmp/icmp_send/enable", O_WRONLY);
    }
    if (fd >= 0) {
        if (write(fd, "1\n", 2) < 0) {
            printf("[-] Failed to write to tracepoint enable file: %s\n", strerror(errno));
        } else {
            printf("[+] Tracepoint enabled successfully.\n");
        }
        close(fd);
    } else {
        printf("[-] Failed to open tracepoint enable file: %s\n", strerror(errno));
    }
}

void disable_icmp_ratelimit(void) {
    int fd = open("/proc/sys/net/ipv4/icmp_ratemask", O_WRONLY);
    if (fd >= 0) {
        if (write(fd, "0\n", 2) < 0) {
            printf("[-] Failed to write to icmp_ratemask: %s\n", strerror(errno));
        } else {
            printf("[+] ICMP ratelimit disabled successfully.\n");
        }
        close(fd);
    } else {
        printf("[-] Failed to open icmp_ratemask: %s\n", strerror(errno));
    }
}

int main(void) {
    enable_tracepoint();
    disable_icmp_ratelimit();

    if (set_if_up("lo") < 0) {
        printf("[-] Failed to bring up lo\n");
    } else {
        printf("[+] lo brought up successfully.\n");
    }

    if (create_vti_device("vti0", "127.0.0.1", "127.0.0.2", 1) < 0) {
        printf("[-] Failed to create vti0\n");
        return 1;
    }
    printf("[+] vti0 created successfully.\n");

    if (set_if_ip("vti0", "10.0.0.1", "255.255.255.0") < 0) {
        printf("[-] Failed to set IP for vti0\n");
        return 1;
    }
    printf("[+] vti0 IP set successfully.\n");

    if (set_if_up("vti0") < 0) {
        printf("[-] Failed to bring up vti0\n");
        return 1;
    }
    printf("[+] vti0 brought up successfully.\n");

    int lo_ifindex = if_nametoindex("lo");
    if (lo_ifindex == 0) {
        printf("[-] Failed to get ifindex for lo: %s\n", strerror(errno));
        return 1;
    }

    if (add_route("8.0.0.0", 8, lo_ifindex) < 0) {
        printf("[-] Failed to add route 8.0.0.0/8 dev lo\n");
        return 1;
    }
    printf("[+] Route 8.0.0.0/8 dev lo added successfully.\n");

    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (fd < 0) {
        printf("[-] Failed to create AF_PACKET socket: %s\n", strerror(errno));
        return 1;
    }
    printf("[+] AF_PACKET socket created successfully.\n");

    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_IP);
    sll.sll_ifindex = if_nametoindex("vti0");

    if (sll.sll_ifindex == 0) {
        printf("[-] Failed to get ifindex for vti0: %s\n", strerror(errno));
        return 1;
    }

    char buf[20];
    memset(buf, 0, sizeof(buf));

    struct iphdr *ip = (struct iphdr *)buf;
    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    ip->tot_len = htons(20);
    ip->id = htons(0x0001);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_IPIP; /* 4 */
    ip->saddr = inet_addr("8.8.8.8");
    ip->daddr = inet_addr("10.0.0.2");

    /* Calculate IP checksum */
    ip->check = 0;
    unsigned int sum = 0;
    unsigned short *ptr = (unsigned short *)ip;
    for (int i = 0; i < 10; i++) {
        sum += ptr[i];
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    ip->check = ~sum;

    if (sendto(fd, buf, sizeof(buf), 0, (struct sockaddr *)&sll, sizeof(sll)) < 0) {
        printf("[-] Failed to send packet: %s\n", strerror(errno));
        return 1;
    }
    printf("[+] Packet sent successfully.\n");

    sleep(1);

    return 0;
}
