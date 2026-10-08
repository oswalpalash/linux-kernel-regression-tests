// https://syzkaller.appspot.com/bug?id=d6076b8ae627051784b4cd77ea811b142aadde58
#define _GNU_SOURCE
#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <linux/genetlink.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_link.h>
#include <net/if.h>

#define HWSIM_CMD_NEW_RADIO 4
#define HWSIM_CMD_DEL_RADIO 5
#define HWSIM_ATTR_RADIO_NAME 17

#define NL80211_CMD_NEW_INTERFACE 7
#define NL80211_CMD_SET_WIPHY_NETNS 49

#define NL80211_ATTR_WIPHY 1
#define NL80211_ATTR_IFNAME 4
#define NL80211_ATTR_IFTYPE 5
#define NL80211_ATTR_PID 82

#define NL80211_IFTYPE_STATION 2

#define RTM_NEWLINKPROP 108
#define IFLA_PROP_LIST 52
#define IFLA_ALT_IFNAME 53
#define NLA_F_NESTED (1 << 15)

int get_family_id(int fd, const char *family_name) {
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = GENL_ID_CTRL;
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_seq = 1;
    req.g.cmd = CTRL_CMD_GETFAMILY;
    req.g.version = 1;

    size_t name_len = strlen(family_name) + 1;
    struct nlattr *nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    nla->nla_type = CTRL_ATTR_FAMILY_NAME;
    nla->nla_len = NLA_HDRLEN + name_len;
    memcpy((char *)nla + NLA_HDRLEN, family_name, name_len);
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    struct sockaddr_nl sa = { .nl_family = AF_NETLINK };
    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        printf("[-] Failed to sendto in get_family_id: %s\n", strerror(errno));
        return -1;
    }

    char resp[4096];
    ssize_t len = recv(fd, resp, sizeof(resp), 0);
    if (len < 0) {
        printf("[-] Failed to recv in get_family_id: %s\n", strerror(errno));
        return -1;
    }

    int family_id = -1;
    for (struct nlmsghdr *nlh = (struct nlmsghdr *)resp; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
        if (nlh->nlmsg_type == GENL_ID_CTRL) {
            struct genlmsghdr *ghdr = (struct genlmsghdr *)NLMSG_DATA(nlh);
            struct nlattr *attr = (struct nlattr *)((char *)ghdr + GENL_HDRLEN);
            int rem = nlh->nlmsg_len - NLMSG_LENGTH(GENL_HDRLEN);
            while (rem >= (int)sizeof(struct nlattr) &&
                   attr->nla_len >= sizeof(struct nlattr) &&
                   attr->nla_len <= rem) {
                if ((attr->nla_type & NLA_TYPE_MASK) == CTRL_ATTR_FAMILY_ID) {
                    family_id = *(uint16_t *)((char *)attr + NLA_HDRLEN);
                    break;
                }
                int aligned = NLA_ALIGN(attr->nla_len);
                rem -= aligned;
                attr = (struct nlattr *)((char *)attr + aligned);
            }
        }
    }
    return family_id;
}

int create_hwsim_radio(int fd, int family_id, const char *phyname) {
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = family_id;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.n.nlmsg_seq = 2;
    req.g.cmd = HWSIM_CMD_NEW_RADIO;
    req.g.version = 1;

    struct nlattr *nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    nla->nla_type = HWSIM_ATTR_RADIO_NAME;
    nla->nla_len = NLA_HDRLEN + strlen(phyname) + 1;
    memcpy((char *)nla + NLA_HDRLEN, phyname, strlen(phyname) + 1);
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    struct sockaddr_nl sa = { .nl_family = AF_NETLINK };
    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        printf("[-] Failed to sendto in create_hwsim_radio: %s\n", strerror(errno));
        return -1;
    }

    char resp[4096];
    ssize_t len = recv(fd, resp, sizeof(resp), 0);
    if (len < 0) {
        printf("[-] Failed to recv in create_hwsim_radio: %s\n", strerror(errno));
        return -1;
    }

    for (struct nlmsghdr *nlh = (struct nlmsghdr *)resp; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
        if (nlh->nlmsg_type == NLMSG_ERROR) {
            struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
            if (err->error < 0) {
                printf("[-] Error creating radio: %s (%d)\n", strerror(-err->error), err->error);
                return -1;
            }
        }
    }
    return 0;
}

int create_wlan_interface(int fd, int family_id, int wiphy_idx, const char *ifname) {
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = family_id;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.n.nlmsg_seq = 3;
    req.g.cmd = NL80211_CMD_NEW_INTERFACE;
    req.g.version = 1;

    struct nlattr *nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    nla->nla_type = NL80211_ATTR_WIPHY;
    nla->nla_len = NLA_HDRLEN + sizeof(uint32_t);
    *(uint32_t *)((char *)nla + NLA_HDRLEN) = wiphy_idx;
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    nla->nla_type = NL80211_ATTR_IFNAME;
    nla->nla_len = NLA_HDRLEN + strlen(ifname) + 1;
    memcpy((char *)nla + NLA_HDRLEN, ifname, strlen(ifname) + 1);
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    nla->nla_type = NL80211_ATTR_IFTYPE;
    nla->nla_len = NLA_HDRLEN + sizeof(uint32_t);
    *(uint32_t *)((char *)nla + NLA_HDRLEN) = NL80211_IFTYPE_STATION;
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    struct sockaddr_nl sa = { .nl_family = AF_NETLINK };
    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        printf("[-] Failed to sendto in create_wlan_interface: %s\n", strerror(errno));
        return -1;
    }

    char resp[4096];
    ssize_t len = recv(fd, resp, sizeof(resp), 0);
    if (len < 0) {
        printf("[-] Failed to recv in create_wlan_interface: %s\n", strerror(errno));
        return -1;
    }
    for (struct nlmsghdr *nlh = (struct nlmsghdr *)resp; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
        if (nlh->nlmsg_type == NLMSG_ERROR) {
            struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
            if (err->error < 0) {
                printf("[-] create_wlan_interface returned: %s (%d)\n", strerror(-err->error), err->error);
                return -1;
            }
        }
    }
    return 0;
}

int move_wiphy(int fd, int family_id, int wiphy_idx, int pid) {
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = family_id;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.n.nlmsg_seq = 4;
    req.g.cmd = NL80211_CMD_SET_WIPHY_NETNS;
    req.g.version = 1;

    struct nlattr *nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    nla->nla_type = NL80211_ATTR_WIPHY;
    nla->nla_len = NLA_HDRLEN + sizeof(uint32_t);
    *(uint32_t *)((char *)nla + NLA_HDRLEN) = wiphy_idx;
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    nla->nla_type = NL80211_ATTR_PID;
    nla->nla_len = NLA_HDRLEN + sizeof(uint32_t);
    *(uint32_t *)((char *)nla + NLA_HDRLEN) = pid;
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    struct sockaddr_nl sa = { .nl_family = AF_NETLINK };
    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        printf("[-] Failed to sendto in move_wiphy: %s\n", strerror(errno));
        return -1;
    }

    char resp[4096];
    ssize_t len = recv(fd, resp, sizeof(resp), 0);
    if (len < 0) {
        printf("[-] Failed to recv in move_wiphy: %s\n", strerror(errno));
        return -1;
    }
    for (struct nlmsghdr *nlh = (struct nlmsghdr *)resp; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
        if (nlh->nlmsg_type == NLMSG_ERROR) {
            struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
            if (err->error < 0) {
                printf("[-] move_wiphy returned: %s (%d)\n", strerror(-err->error), err->error);
                return -1;
            }
        }
    }
    return 0;
}

int add_altname(int ifindex, const char *altname) {
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) {
        printf("[-] Failed to socket in add_altname: %s\n", strerror(errno));
        return -1;
    }

    struct {
        struct nlmsghdr n;
        struct ifinfomsg i;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.n.nlmsg_type = RTM_NEWLINKPROP;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.i.ifi_family = AF_UNSPEC;
    req.i.ifi_index = ifindex;

    struct rtattr *prop = (struct rtattr *)(((char *)&req) + NLMSG_ALIGN(req.n.nlmsg_len));
    prop->rta_type = IFLA_PROP_LIST | NLA_F_NESTED;
    prop->rta_len = RTA_LENGTH(0);
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + RTA_ALIGN(prop->rta_len);

    struct rtattr *alt = (struct rtattr *)(((char *)&req) + NLMSG_ALIGN(req.n.nlmsg_len));
    alt->rta_type = IFLA_ALT_IFNAME;
    alt->rta_len = RTA_LENGTH(strlen(altname) + 1);
    memcpy(RTA_DATA(alt), altname, strlen(altname) + 1);
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + RTA_ALIGN(alt->rta_len);

    prop->rta_len = (char *)(((char *)&req) + req.n.nlmsg_len) - (char *)prop;

    struct sockaddr_nl sa = { .nl_family = AF_NETLINK };
    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        printf("[-] Failed to sendto in add_altname: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    
    char resp[4096];
    ssize_t len = recv(fd, resp, sizeof(resp), 0);
    if (len >= 0) {
        for (struct nlmsghdr *nlh = (struct nlmsghdr *)resp; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
            if (nlh->nlmsg_type == NLMSG_ERROR) {
                struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
                if (err->error < 0) {
                    printf("[-] Error adding altname: %s (%d)\n", strerror(-err->error), err->error);
                    close(fd);
                    return -1;
                }
            }
        }
    }
    close(fd);
    return 0;
}

int create_dummy_interface(const char *name) {
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (fd < 0) {
        printf("[-] Failed to socket in create_dummy_interface: %s\n", strerror(errno));
        return -1;
    }

    struct {
        struct nlmsghdr n;
        struct ifinfomsg i;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.n.nlmsg_type = RTM_NEWLINK;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE;
    req.i.ifi_family = AF_UNSPEC;

    struct rtattr *rta = (struct rtattr *)(((char *)&req) + NLMSG_ALIGN(req.n.nlmsg_len));
    rta->rta_type = IFLA_IFNAME;
    rta->rta_len = RTA_LENGTH(strlen(name) + 1);
    memcpy(RTA_DATA(rta), name, strlen(name) + 1);
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + RTA_ALIGN(rta->rta_len);

    struct rtattr *linkinfo = (struct rtattr *)(((char *)&req) + NLMSG_ALIGN(req.n.nlmsg_len));
    linkinfo->rta_type = IFLA_LINKINFO | NLA_F_NESTED;
    linkinfo->rta_len = RTA_LENGTH(0);
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + RTA_ALIGN(linkinfo->rta_len);

    struct rtattr *kind = (struct rtattr *)(((char *)&req) + NLMSG_ALIGN(req.n.nlmsg_len));
    kind->rta_type = IFLA_INFO_KIND;
    kind->rta_len = RTA_LENGTH(strlen("dummy") + 1);
    memcpy(RTA_DATA(kind), "dummy", strlen("dummy") + 1);
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + RTA_ALIGN(kind->rta_len);

    linkinfo->rta_len = (char *)(((char *)&req) + req.n.nlmsg_len) - (char *)linkinfo;

    struct sockaddr_nl sa = { .nl_family = AF_NETLINK };
    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        printf("[-] Failed to sendto in create_dummy_interface: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    
    char resp[4096];
    ssize_t len = recv(fd, resp, sizeof(resp), 0);
    if (len < 0) {
        printf("[-] Failed to recv in create_dummy_interface: %s\n", strerror(errno));
        close(fd);
        return -1;
    }
    for (struct nlmsghdr *nlh = (struct nlmsghdr *)resp; NLMSG_OK(nlh, len); nlh = NLMSG_NEXT(nlh, len)) {
        if (nlh->nlmsg_type == NLMSG_ERROR) {
            struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
            if (err->error < 0 && err->error != -EEXIST) {
                printf("[-] Error creating dummy interface: %s (%d)\n", strerror(-err->error), err->error);
                close(fd);
                return -1;
            }
        }
    }

    close(fd);
    return 0;
}

int main(void) {
    printf("[*] Starting reproducer...\n");

    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd < 0) {
        printf("[-] Failed to socket: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] socket successful.\n");

    int hwsim_id = get_family_id(fd, "MAC80211_HWSIM");
    if (hwsim_id < 0) {
        printf("[-] Failed to get MAC80211_HWSIM family ID\n");
        exit(1);
    }
    printf("[+] get_family_id MAC80211_HWSIM successful.\n");

    int nl80211_id = get_family_id(fd, "nl80211");
    if (nl80211_id < 0) {
        printf("[-] Failed to get nl80211 family ID\n");
        exit(1);
    }
    printf("[+] get_family_id nl80211 successful.\n");

    /* 1. Create a hwsim radio in init_net (this automatically creates wlan0). */
    if (create_hwsim_radio(fd, hwsim_id, "hwsim_test0") < 0) {
        printf("[-] Failed to create_hwsim_radio\n");
        exit(1);
    }
    printf("[+] create_hwsim_radio successful.\n");
    
    /* We assume wiphy index is 0 for the first created wiphy. */
    int wiphy_idx = 0;

    /* 2. Create a second interface wlan_test1 on the same radio */
    if (create_wlan_interface(fd, nl80211_id, wiphy_idx, "wlan_test1") < 0) {
        printf("[-] Failed to create_wlan_interface\n");
        exit(1);
    }
    printf("[+] create_wlan_interface successful.\n");

    /* 3. Add an altname to wlan_test1 */
    int wlan1_idx = if_nametoindex("wlan_test1");
    if (wlan1_idx > 0) {
        if (add_altname(wlan1_idx, "myaltname") < 0) {
            printf("[-] Failed to add_altname to wlan_test1\n");
            exit(1);
        }
        printf("[+] add_altname to wlan_test1 successful.\n");
    } else {
        printf("[-] Could not find wlan_test1\n");
        exit(1);
    }

    /* 4. Move the wiphy to a new netns (netns B) */
    int sv_B[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv_B) < 0) {
        printf("[-] Failed to socketpair: %s\n", strerror(errno));
        exit(1);
    }
    pid_t pid_B = fork();
    if (pid_B < 0) {
        printf("[-] Failed to fork: %s\n", strerror(errno));
        exit(1);
    }

    if (pid_B == 0) {
        close(sv_B[0]);
        if (unshare(CLONE_NEWNET) < 0) {
            printf("[-] Failed to unshare: %s\n", strerror(errno));
            exit(1);
        }
        
        /* Tell parent we are ready */
        if (write(sv_B[1], "A", 1) != 1) {
            printf("[-] Failed to write to sv_B[1]: %s\n", strerror(errno));
            exit(1);
        }
        
        /* Wait for parent to finish */
        char buf;
        if (read(sv_B[1], &buf, 1) != 1) {
            printf("[-] Failed to read from sv_B[1]: %s\n", strerror(errno));
            exit(1);
        }
        exit(0);
    }

    close(sv_B[1]);
    char buf;
    if (read(sv_B[0], &buf, 1) != 1) {
        printf("[-] Failed to read from sv_B[0]: %s\n", strerror(errno));
        exit(1);
    }

    if (move_wiphy(fd, nl80211_id, wiphy_idx, pid_B) < 0) {
        printf("[-] Failed to move_wiphy\n");
        exit(1);
    }
    printf("[+] move_wiphy successful.\n");

    /* 5. Create a dummy interface in init_net and add the SAME altname to it */
    if (create_dummy_interface("dummy_test0") < 0) {
        printf("[-] Failed to create_dummy_interface\n");
        exit(1);
    }
    printf("[+] create_dummy_interface successful.\n");

    int dummy_idx = if_nametoindex("dummy_test0");
    if (dummy_idx > 0) {
        if (add_altname(dummy_idx, "myaltname") < 0) {
            printf("[-] Failed to add_altname to dummy_test0\n");
            exit(1);
        }
        printf("[+] add_altname to dummy_test0 successful.\n");
    } else {
        printf("[-] Could not find dummy_test0\n");
        exit(1);
    }

    /* 6. Destroy netns B */
    if (write(sv_B[0], "B", 1) != 1) {
        printf("[-] Failed to write to sv_B[0]: %s\n", strerror(errno));
        exit(1);
    }
    waitpid(pid_B, NULL, 0);
    printf("[+] netns B destroyed.\n");

    /* 7. Wait for netns B to be destroyed */
    sleep(2);

    /* 8. Delete the hwsim radio */
    struct {
        struct nlmsghdr n;
        struct genlmsghdr g;
        char buf[256];
    } req;
    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.n.nlmsg_type = hwsim_id;
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.g.cmd = HWSIM_CMD_DEL_RADIO;
    req.g.version = 1;

    struct nlattr *nla = (struct nlattr *)((char *)&req + NLMSG_ALIGN(req.n.nlmsg_len));
    nla->nla_type = HWSIM_ATTR_RADIO_NAME;
    nla->nla_len = NLA_HDRLEN + strlen("hwsim_test0") + 1;
    memcpy((char *)nla + NLA_HDRLEN, "hwsim_test0", strlen("hwsim_test0") + 1);
    req.n.nlmsg_len = NLMSG_ALIGN(req.n.nlmsg_len) + NLA_ALIGN(nla->nla_len);

    struct sockaddr_nl sa = { .nl_family = AF_NETLINK };
    if (sendto(fd, &req, req.n.nlmsg_len, 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        printf("[-] Failed to sendto HWSIM_CMD_DEL_RADIO: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] sendto HWSIM_CMD_DEL_RADIO successful.\n");

    sleep(2);

    return 0;
}
