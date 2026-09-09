// https://syzkaller.appspot.com/bug?id=c5f9859194afbfc44c7bd4551e36fa3ea05b32db
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/wireless.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <errno.h>

#define GENLMSG_DATA(glh) ((void *)(NLMSG_DATA(glh) + GENL_HDRLEN))
#define NLA_DATA(na) ((void *)((char *)(na) + NLA_HDRLEN))

#ifndef NLA_OK
#define NLA_OK(nla,len) ((len) >= (int)sizeof(struct nlattr) && \
                         (nla)->nla_len >= sizeof(struct nlattr) && \
                         (nla)->nla_len <= (len))
#endif
#ifndef NLA_NEXT
#define NLA_NEXT(nla,attrlen)   ((attrlen) -= NLA_ALIGN((nla)->nla_len), \
                                 (struct nlattr*)(((char*)(nla)) + NLA_ALIGN((nla)->nla_len)))
#endif

static int get_family_id(int sock, const char *family_name) {
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;
    struct nlattr *na;
    int id = -1;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = GENL_ID_CTRL;
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.g.cmd = CTRL_CMD_GETFAMILY;
    req.g.version = 1;

    na = (struct nlattr *)GENLMSG_DATA(&req);
    na->nla_type = CTRL_ATTR_FAMILY_NAME;
    na->nla_len = strlen(family_name) + 1 + NLA_HDRLEN;
    strcpy(NLA_DATA(na), family_name);

    req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    if (send(sock, &req, req.n.nlmsg_len, 0) < 0)
        return -1;

    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[4096];
    } ans;

    int len = recv(sock, &ans, sizeof(ans), 0);
    if (len < 0)
        return -1;

    na = (struct nlattr *)GENLMSG_DATA(&ans);
    int attr_len = ans.n.nlmsg_len - NLMSG_LENGTH(GENL_HDRLEN);
    while (NLA_OK(na, attr_len)) {
        if (na->nla_type == CTRL_ATTR_FAMILY_ID) {
            id = *(uint16_t *)NLA_DATA(na);
            break;
        }
        na = NLA_NEXT(na, attr_len);
    }
    return id;
}

static void create_hwsim_radio_nan(int attr_type) {
    int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (sock < 0) {
        printf("[-] socket(AF_NETLINK) failed: %s\n", strerror(errno));
        return;
    }

    int family_id = get_family_id(sock, "MAC80211_HWSIM");
    if (family_id < 0) {
        printf("[-] get_family_id failed\n");
        close(sock);
        return;
    }

    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = family_id;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.g.cmd = 4; // HWSIM_CMD_NEW_RADIO
    req.g.version = 1;

    struct nlattr *na = (struct nlattr *)GENLMSG_DATA(&req);
    na->nla_type = attr_type;
    na->nla_len = NLA_HDRLEN; // Flag attribute has no data payload

    req.n.nlmsg_len += NLMSG_ALIGN(na->nla_len);

    if (send(sock, &req, req.n.nlmsg_len, 0) < 0) {
        printf("[-] send failed: %s\n", strerror(errno));
    } else {
        struct {
            struct nlmsghdr n;
            char buf[4096];
        } ans;
        if (recv(sock, &ans, sizeof(ans), 0) < 0) {
            printf("[-] recv failed: %s\n", strerror(errno));
        } else {
            printf("[+] create_hwsim_radio_nan(%d) sent successfully.\n", attr_type);
        }
    }

    close(sock);
}

void trigger_bug(int sock, const char *ifname)
{
    struct ifreq ifr = {0};
    struct iwreq iwr = {0};
    int res;

    // 1. Ensure the interface is DOWN before changing its mode
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    res = ioctl(sock, SIOCGIFFLAGS, &ifr);
    if (res < 0) {
        return; // Interface doesn't exist
    }

    ifr.ifr_flags &= ~IFF_UP;
    res = ioctl(sock, SIOCSIFFLAGS, &ifr);
    if (res < 0) {
        printf("[-] ioctl(SIOCSIFFLAGS) DOWN failed on %s: %s\n", ifname, strerror(errno));
        return;
    }

    // 2. Change the interface mode to MONITOR
    memset(&iwr, 0, sizeof(iwr));
    strncpy(iwr.ifr_name, ifname, IFNAMSIZ - 1);
    iwr.u.mode = IW_MODE_MONITOR;
    res = ioctl(sock, SIOCSIWMODE, &iwr);
    if (res < 0) {
        printf("[-] ioctl(SIOCSIWMODE) failed on %s: %s\n", ifname, strerror(errno));
        return;
    }

    // 3. Bring the interface UP
    res = ioctl(sock, SIOCGIFFLAGS, &ifr);
    if (res < 0) {
        printf("[-] ioctl(SIOCGIFFLAGS) failed on %s: %s\n", ifname, strerror(errno));
        return;
    }

    ifr.ifr_flags |= IFF_UP;
    res = ioctl(sock, SIOCSIFFLAGS, &ifr);
    if (res < 0) {
        printf("[-] ioctl(SIOCSIFFLAGS) UP failed on %s: %s\n", ifname, strerror(errno));
        return;
    }

    // 4. Trigger the warning by setting the bitrate
    memset(&iwr, 0, sizeof(iwr));
    strncpy(iwr.ifr_name, ifname, IFNAMSIZ - 1);
    iwr.u.bitrate.value = 1000000000; // Large value to ensure it matches maxrate
    iwr.u.bitrate.fixed = 0;
    res = ioctl(sock, SIOCSIWRATE, &iwr);
    if (res < 0) {
        printf("[-] ioctl(SIOCSIWRATE) failed on %s: %s\n", ifname, strerror(errno));
    } else {
        printf("[+] ioctl(SIOCSIWRATE) successful on %s.\n", ifname);
    }
}

int main(void)
{
    // Create a new radio with NAN support to enable HAS_RATE_CONTROL
    // We try both 29 and 30 because the exact attribute ID might vary slightly
    // between kernel versions (HWSIM_ATTR_SUPPORT_NAN_DEVICE).
    create_hwsim_radio_nan(29);
    create_hwsim_radio_nan(30);

    // Give it a moment to be fully registered
    usleep(200000);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("[-] socket(AF_INET) failed: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] socket(AF_INET) successful.\n");

    // Attempt on wlan0 to wlan100 to ensure we hit the newly created radio
    char ifname[16];
    for (int i = 0; i < 100; i++) {
        snprintf(ifname, sizeof(ifname), "wlan%d", i);
        trigger_bug(sock, ifname);
    }

    close(sock);
    
    // Sleep to allow asynchronous warnings to trigger if any
    sleep(1);
    
    return 0;
}
