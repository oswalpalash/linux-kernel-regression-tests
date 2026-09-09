// https://syzkaller.appspot.com/bug?id=1832bd102c7e96eef0a3edf4a5a65f37c45f15b5
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <linux/nl80211.h>

#define GENLMSG_DATA(glh) ((void *)(NLMSG_DATA(glh) + GENL_HDRLEN))
#define NLA_DATA(na) ((void *)((char *)(na) + NLA_HDRLEN))
#define NLA_NEXT(na, len) \
    ((len) -= NLA_ALIGN((na)->nla_len), \
     (struct nlattr *)((char *)(na) + NLA_ALIGN((na)->nla_len)))
#define NLA_OK(na, len) \
    ((len) >= (int)sizeof(struct nlattr) && \
     (na)->nla_len >= sizeof(struct nlattr) && \
     (na)->nla_len <= (len))

static int get_family_id(int fd, const char *name)
{
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;
    struct nlattr *na;
    struct sockaddr_nl nladdr;
    int id = 0;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = GENL_ID_CTRL;
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.g.cmd = CTRL_CMD_GETFAMILY;
    req.g.version = 1;

    na = (struct nlattr *)GENLMSG_DATA(&req.n);
    na->nla_type = CTRL_ATTR_FAMILY_NAME;
    na->nla_len = strlen(name) + 1 + NLA_HDRLEN;
    strcpy(NLA_DATA(na), name);
    req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
        printf("[-] Failed to sendto in get_family_id: %s\n", strerror(errno));
        exit(1);
    }

    if (recv(fd, &req, sizeof(req), 0) < 0) {
        printf("[-] Failed to recv in get_family_id: %s\n", strerror(errno));
        exit(1);
    }

    struct nlattr *attr = (struct nlattr *)GENLMSG_DATA(&req.n);
    int len = req.n.nlmsg_len - NLMSG_LENGTH(GENL_HDRLEN);
    while (NLA_OK(attr, len)) {
        if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
            id = *(uint16_t *)NLA_DATA(attr);
            break;
        }
        attr = NLA_NEXT(attr, len);
    }
    return id;
}

static int create_hwsim_radio(int fd)
{
    int hwsim_id = get_family_id(fd, "MAC80211_HWSIM");
    if (!hwsim_id) {
        printf("[-] Failed to get MAC80211_HWSIM family id\n");
        return -1;
    }
    printf("[+] Got MAC80211_HWSIM family id: %d\n", hwsim_id);

    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;
    struct nlattr *na;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = hwsim_id;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.g.cmd = 4; // HWSIM_CMD_NEW_RADIO
    req.g.version = 1;

    na = (struct nlattr *)GENLMSG_DATA(&req.n);
    na->nla_type = 25; // HWSIM_ATTR_MLO_SUPPORT
    na->nla_len = NLA_HDRLEN;
    req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    struct sockaddr_nl nladdr;
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
        printf("[-] Failed to sendto in create_hwsim_radio: %s\n", strerror(errno));
        return -1;
    }
    
    if (recv(fd, &req, sizeof(req), 0) < 0) {
        printf("[-] Failed to recv in create_hwsim_radio: %s\n", strerror(errno));
        return -1;
    }

    if (req.n.nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(&req.n);
        if (err->error != 0) {
            printf("[-] HWSIM_CMD_NEW_RADIO error: %d\n", err->error);
            return -1;
        }
    }

    printf("[+] create_hwsim_radio successful.\n");
    return 0;
}

static int set_if_state(int ifindex, int up)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        printf("[-] Failed to socket in set_if_state: %s\n", strerror(errno));
        return -1;
    }
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    if_indextoname(ifindex, ifr.ifr_name);
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
        printf("[-] Failed to ioctl(SIOCGIFFLAGS) in set_if_state: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    if (up)
        ifr.ifr_flags |= IFF_UP;
    else
        ifr.ifr_flags &= ~IFF_UP;
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
        printf("[-] Failed to ioctl(SIOCSIFFLAGS) in set_if_state: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    printf("[+] set_if_state (up=%d) successful for %s.\n", up, ifr.ifr_name);
    return 0;
}

static int set_interface_monitor(int fd, int nl80211_id, int ifindex)
{
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;
    struct nlattr *na;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = nl80211_id;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.g.cmd = NL80211_CMD_SET_INTERFACE;
    req.g.version = 1;

    na = (struct nlattr *)GENLMSG_DATA(&req.n);
    na->nla_type = NL80211_ATTR_IFINDEX;
    na->nla_len = sizeof(uint32_t) + NLA_HDRLEN;
    *(uint32_t *)NLA_DATA(na) = ifindex;
    req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    na = (struct nlattr *)((char *)na + NLMSG_ALIGN(na->nla_len));
    na->nla_type = NL80211_ATTR_IFTYPE;
    na->nla_len = sizeof(uint32_t) + NLA_HDRLEN;
    *(uint32_t *)NLA_DATA(na) = NL80211_IFTYPE_MONITOR;
    req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    struct sockaddr_nl nladdr;
    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
        printf("[-] Failed to sendto in set_interface_monitor: %s\n", strerror(errno));
        return -1;
    }
    
    if (recv(fd, &req, sizeof(req), 0) < 0) {
        printf("[-] Failed to recv in set_interface_monitor: %s\n", strerror(errno));
        return -1;
    }

    if (req.n.nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(&req.n);
        if (err->error != 0) {
            printf("[-] NL80211_CMD_SET_INTERFACE error: %d\n", err->error);
            return -1;
        }
    }

    printf("[+] set_interface_monitor successful for ifindex %d.\n", ifindex);
    return 0;
}

static void send_bitrate_mask(int fd, int nl80211_id, int ifindex)
{
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;
    struct nlattr *na;
    struct sockaddr_nl nladdr;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = nl80211_id;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.g.cmd = NL80211_CMD_SET_TX_BITRATE_MASK;
    req.g.version = 1;

    na = (struct nlattr *)GENLMSG_DATA(&req.n);
    na->nla_type = NL80211_ATTR_IFINDEX;
    na->nla_len = sizeof(uint32_t) + NLA_HDRLEN;
    *(uint32_t *)NLA_DATA(na) = ifindex;
    req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    memset(&nladdr, 0, sizeof(nladdr));
    nladdr.nl_family = AF_NETLINK;

    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&nladdr, sizeof(nladdr)) < 0) {
        printf("[-] Failed to sendto in send_bitrate_mask: %s\n", strerror(errno));
        return;
    }
    
    if (recv(fd, &req, sizeof(req), 0) < 0) {
        printf("[-] Failed to recv in send_bitrate_mask: %s\n", strerror(errno));
        return;
    }

    if (req.n.nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(&req.n);
        if (err->error != 0) {
            printf("[-] NL80211_CMD_SET_TX_BITRATE_MASK error: %d\n", err->error);
            return;
        }
    }

    printf("[+] send_bitrate_mask successful for ifindex %d.\n", ifindex);
}

int main(void)
{
    int fd;
    struct sockaddr_nl local;
    int nl80211_id;

    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd < 0) {
        printf("[-] Failed to socket: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] socket successful.\n");

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    if (bind(fd, (struct sockaddr *)&local, sizeof(local)) < 0) {
        printf("[-] Failed to bind: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] bind successful.\n");

    // Create a new radio with MLO support (sets HAS_RATE_CONTROL)
    create_hwsim_radio(fd);
    sleep(1); // Wait for the new interface to appear

    nl80211_id = get_family_id(fd, "nl80211");
    if (!nl80211_id) {
        printf("[-] Failed to get nl80211 family id\n");
        exit(1);
    }
    printf("[+] Got nl80211 family id: %d\n", nl80211_id);

    // Try all possible wlan interfaces
    for (int i = 0; i < 10; i++) {
        char ifname[16];
        snprintf(ifname, sizeof(ifname), "wlan%d", i);
        int ifindex = if_nametoindex(ifname);
        if (ifindex == 0) continue;
        printf("[+] Found interface %s (ifindex %d)\n", ifname, ifindex);

        // 1. Bring interface DOWN
        set_if_state(ifindex, 0);

        // 2. Change interface type to MONITOR
        if (set_interface_monitor(fd, nl80211_id, ifindex) < 0)
            continue;

        // 3. Bring interface UP (as a passive monitor)
        if (set_if_state(ifindex, 1) < 0)
            continue;

        // 4. Send command while UP
        send_bitrate_mask(fd, nl80211_id, ifindex);
    }

    close(fd);
    printf("[+] Reproducer finished.\n");
    
    // Wait for asynchronous warnings
    sleep(2);
    
    return 0;
}
