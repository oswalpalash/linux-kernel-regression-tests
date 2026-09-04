// https://syzkaller.appspot.com/bug?id=725950daaf845c1a583e6376dfccdea3aabf527a
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/genetlink.h>
#include <net/if.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdint.h>

#define NLA_F_NESTED (1 << 15)
#define IFLA_PROP_LIST 52
#define IFLA_ALT_IFNAME 53
#ifndef RTM_NEWLINKPROP
#define RTM_NEWLINKPROP 108
#endif

#define NL80211_CMD_SET_WIPHY_NETNS 49
#define NL80211_CMD_NEW_INTERFACE 7
#define NL80211_ATTR_WIPHY 1
#define NL80211_ATTR_IFNAME 4
#define NL80211_ATTR_IFTYPE 5
#define NL80211_ATTR_PID 82
#define NL80211_IFTYPE_STATION 2

#define HWSIM_CMD_NEW_RADIO 4
#define HWSIM_CMD_DEL_RADIO 5
#define HWSIM_ATTR_RADIO_ID 10

int send_and_recv(int fd, void *req, int len, const char *name) {
    if (send(fd, req, len, 0) < 0) {
        printf("[-] send failed for %s: %s\n", name, strerror(errno));
        return -1;
    }
    char buf[4096];
    int rlen = recv(fd, buf, sizeof(buf), 0);
    if (rlen < 0) {
        printf("[-] recv failed for %s: %s\n", name, strerror(errno));
        return -1;
    }
    struct nlmsghdr *nlh = (struct nlmsghdr *)buf;
    if (nlh->nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(nlh);
        if (err->error != 0) {
            // Don't print error for expected failures during brute-force loops
            return err->error;
        }
    }
    return 0;
}

int resolve_family(int fd, const char *name) {
    struct {
        struct nlmsghdr nlh;
        struct genlmsghdr gnlh;
        struct nlattr nla;
        char name[32];
    } req = {0};

    req.nlh.nlmsg_len = sizeof(req);
    req.nlh.nlmsg_type = GENL_ID_CTRL;
    req.nlh.nlmsg_flags = NLM_F_REQUEST;
    req.gnlh.cmd = CTRL_CMD_GETFAMILY;
    req.gnlh.version = 1;
    req.nla.nla_len = sizeof(req.nla) + strlen(name) + 1;
    req.nla.nla_type = CTRL_ATTR_FAMILY_NAME;
    strcpy(req.name, name);
    req.nlh.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN) + RTA_ALIGN(req.nla.nla_len);

    if (send(fd, &req, req.nlh.nlmsg_len, 0) < 0) {
        printf("[-] send failed for resolve_family %s: %s\n", name, strerror(errno));
        return -1;
    }

    char buf[4096];
    int len = recv(fd, buf, sizeof(buf), 0);
    if (len < 0) {
        printf("[-] recv failed for resolve_family %s: %s\n", name, strerror(errno));
        return -1;
    }

    struct nlmsghdr *nlh = (struct nlmsghdr *)buf;
    if (nlh->nlmsg_type == NLMSG_ERROR) {
        printf("[-] netlink error in resolve_family %s\n", name);
        return -1;
    }

    struct genlmsghdr *gnlh = (struct genlmsghdr *)NLMSG_DATA(nlh);
    struct rtattr *nla = (struct rtattr *)((char *)gnlh + GENL_HDRLEN);
    int nla_len = nlh->nlmsg_len - NLMSG_LENGTH(GENL_HDRLEN);

    while (RTA_OK(nla, nla_len)) {
        if (nla->rta_type == CTRL_ATTR_FAMILY_ID) {
            int id = *(uint16_t *)RTA_DATA(nla);
            printf("[+] resolve_family %s successful: %d\n", name, id);
            return id;
        }
        nla = RTA_NEXT(nla, nla_len);
    }
    printf("[-] family %s not found\n", name);
    return -1;
}

int create_radio(int fd, int hwsim_id) {
    struct {
        struct nlmsghdr nlh;
        struct genlmsghdr gnlh;
    } req = {0};

    req.nlh.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.nlh.nlmsg_type = hwsim_id;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.gnlh.cmd = HWSIM_CMD_NEW_RADIO;
    req.gnlh.version = 1;

    return send_and_recv(fd, &req, req.nlh.nlmsg_len, "create_radio");
}

int delete_radio(int fd, int hwsim_id, int radio_id) {
    struct {
        struct nlmsghdr nlh;
        struct genlmsghdr gnlh;
        char buf[256];
    } req = {0};

    req.nlh.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.nlh.nlmsg_type = hwsim_id;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.gnlh.cmd = HWSIM_CMD_DEL_RADIO;
    req.gnlh.version = 1;

    struct rtattr *rta = (struct rtattr *)(((char *)&req) + req.nlh.nlmsg_len);
    rta->rta_type = HWSIM_ATTR_RADIO_ID;
    rta->rta_len = RTA_LENGTH(sizeof(uint32_t));
    memcpy(RTA_DATA(rta), &radio_id, sizeof(uint32_t));
    req.nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

    return send_and_recv(fd, &req, req.nlh.nlmsg_len, "delete_radio");
}

int set_wiphy_netns(int fd, int nl80211_id, int wiphy_id, int pid) {
    struct {
        struct nlmsghdr nlh;
        struct genlmsghdr gnlh;
        char buf[256];
    } req = {0};

    req.nlh.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.nlh.nlmsg_type = nl80211_id;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.gnlh.cmd = NL80211_CMD_SET_WIPHY_NETNS;
    req.gnlh.version = 1;

    struct rtattr *rta = (struct rtattr *)(((char *)&req) + req.nlh.nlmsg_len);
    rta->rta_type = NL80211_ATTR_WIPHY;
    rta->rta_len = RTA_LENGTH(sizeof(uint32_t));
    memcpy(RTA_DATA(rta), &wiphy_id, sizeof(uint32_t));
    req.nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

    rta = (struct rtattr *)(((char *)&req) + req.nlh.nlmsg_len);
    rta->rta_type = NL80211_ATTR_PID;
    rta->rta_len = RTA_LENGTH(sizeof(uint32_t));
    memcpy(RTA_DATA(rta), &pid, sizeof(uint32_t));
    req.nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

    return send_and_recv(fd, &req, req.nlh.nlmsg_len, "set_wiphy_netns");
}

int new_interface(int fd, int nl80211_id, int wiphy_id, const char *name) {
    struct {
        struct nlmsghdr nlh;
        struct genlmsghdr gnlh;
        char buf[256];
    } req = {0};

    req.nlh.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);
    req.nlh.nlmsg_type = nl80211_id;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.gnlh.cmd = NL80211_CMD_NEW_INTERFACE;
    req.gnlh.version = 1;

    struct rtattr *rta = (struct rtattr *)(((char *)&req) + req.nlh.nlmsg_len);
    rta->rta_type = NL80211_ATTR_WIPHY;
    rta->rta_len = RTA_LENGTH(sizeof(uint32_t));
    memcpy(RTA_DATA(rta), &wiphy_id, sizeof(uint32_t));
    req.nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

    rta = (struct rtattr *)(((char *)&req) + req.nlh.nlmsg_len);
    rta->rta_type = NL80211_ATTR_IFTYPE;
    rta->rta_len = RTA_LENGTH(sizeof(uint32_t));
    uint32_t iftype = NL80211_IFTYPE_STATION;
    memcpy(RTA_DATA(rta), &iftype, sizeof(uint32_t));
    req.nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

    rta = (struct rtattr *)(((char *)&req) + req.nlh.nlmsg_len);
    rta->rta_type = NL80211_ATTR_IFNAME;
    rta->rta_len = RTA_LENGTH(strlen(name) + 1);
    memcpy(RTA_DATA(rta), name, strlen(name) + 1);
    req.nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

    return send_and_recv(fd, &req, req.nlh.nlmsg_len, "new_interface");
}

int add_altname(int fd, int ifindex, const char *altname) {
    struct {
        struct nlmsghdr nlh;
        struct ifinfomsg ifm;
        char buf[256];
    } req = {0};

    req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.nlh.nlmsg_type = RTM_NEWLINKPROP;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    req.ifm.ifi_index = ifindex;

    int prop_list_offset = req.nlh.nlmsg_len;
    struct rtattr *prop_list = (struct rtattr *)(((char *)&req) + prop_list_offset);
    prop_list->rta_type = IFLA_PROP_LIST | NLA_F_NESTED;
    prop_list->rta_len = RTA_LENGTH(0);
    req.nlh.nlmsg_len += RTA_ALIGN(prop_list->rta_len);

    struct rtattr *alt = (struct rtattr *)(((char *)&req) + req.nlh.nlmsg_len);
    alt->rta_type = IFLA_ALT_IFNAME;
    alt->rta_len = RTA_LENGTH(strlen(altname) + 1);
    memcpy(RTA_DATA(alt), altname, strlen(altname) + 1);
    
    req.nlh.nlmsg_len += RTA_ALIGN(alt->rta_len);
    prop_list->rta_len = req.nlh.nlmsg_len - prop_list_offset;

    return send_and_recv(fd, &req, req.nlh.nlmsg_len, "add_altname");
}

int create_dummy(int fd, const char *name) {
    struct {
        struct nlmsghdr nlh;
        struct ifinfomsg ifm;
        char buf[256];
    } req = {0};

    req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.nlh.nlmsg_type = RTM_NEWLINK;
    req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | NLM_F_CREATE | NLM_F_EXCL;

    struct rtattr *rta = (struct rtattr *)(((char *)&req) + req.nlh.nlmsg_len);
    rta->rta_type = IFLA_IFNAME;
    rta->rta_len = RTA_LENGTH(strlen(name) + 1);
    memcpy(RTA_DATA(rta), name, strlen(name) + 1);
    req.nlh.nlmsg_len += RTA_ALIGN(rta->rta_len);

    int linkinfo_offset = req.nlh.nlmsg_len;
    struct rtattr *linkinfo = (struct rtattr *)(((char *)&req) + linkinfo_offset);
    linkinfo->rta_type = IFLA_LINKINFO | NLA_F_NESTED;
    linkinfo->rta_len = RTA_LENGTH(0);
    req.nlh.nlmsg_len += RTA_ALIGN(linkinfo->rta_len);
    
    struct rtattr *rta_kind = (struct rtattr *)(((char *)&req) + req.nlh.nlmsg_len);
    rta_kind->rta_type = IFLA_INFO_KIND;
    rta_kind->rta_len = RTA_LENGTH(strlen("dummy") + 1);
    memcpy(RTA_DATA(rta_kind), "dummy", strlen("dummy") + 1);
    
    req.nlh.nlmsg_len += RTA_ALIGN(rta_kind->rta_len);
    linkinfo->rta_len = req.nlh.nlmsg_len - linkinfo_offset;

    return send_and_recv(fd, &req, req.nlh.nlmsg_len, "create_dummy");
}

int main() {
    int genl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (genl_fd < 0) {
        printf("[-] socket failed: %s\n", strerror(errno));
        return 1;
    }
    printf("[+] socket successful.\n");

    int hwsim_id = resolve_family(genl_fd, "MAC80211_HWSIM");
    int nl80211_id = resolve_family(genl_fd, "nl80211");

    if (hwsim_id < 0 || nl80211_id < 0) {
        printf("[-] Failed to resolve families\n");
        return 1;
    }

    // 1. Create radio in init_net
    if (create_radio(genl_fd, hwsim_id) == 0) {
        printf("[+] create_radio successful\n");
    }

    // 2. Create dummy interface in init_net to cause collision
    int rtnl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (rtnl_fd < 0) {
        printf("[-] socket rtnl failed: %s\n", strerror(errno));
        return 1;
    }
    if (create_dummy(rtnl_fd, "myaltname") == 0) {
        printf("[+] create_dummy successful\n");
    }

    int fds_p2c[2];
    int fds_c2p[2];
    if (pipe(fds_p2c) < 0 || pipe(fds_c2p) < 0) {
        printf("[-] pipe failed: %s\n", strerror(errno));
        return 1;
    }

    // 3. Create ns1
    int pid1 = fork();
    if (pid1 < 0) {
        printf("[-] fork failed: %s\n", strerror(errno));
        return 1;
    }

    if (pid1 == 0) {
        close(fds_p2c[1]);
        close(fds_c2p[0]);
        if (unshare(CLONE_NEWNET) < 0) {
            printf("[-] unshare failed: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] Child unshared network namespace\n");
        
        // Tell parent we unshared
        write(fds_c2p[1], "x", 1);

        // Wait for parent to move wiphy
        char c;
        if (read(fds_p2c[0], &c, 1) < 0) {
            printf("[-] read failed: %s\n", strerror(errno));
            exit(1);
        }
        printf("[+] Child woke up\n");

        int genl_fd_ns = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
        int rtnl_fd_ns = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
        
        // 4. Create mywlan in ns1
        for (int i = 0; i < 10; i++) {
            if (new_interface(genl_fd_ns, nl80211_id, i, "mywlan") == 0) {
                printf("[+] new_interface mywlan on wiphy %d successful\n", i);
            }
        }

        // 5. Add altname to mywlan in ns1
        int mywlan_ifindex = if_nametoindex("mywlan");
        if (mywlan_ifindex > 0) {
            if (add_altname(rtnl_fd_ns, mywlan_ifindex, "myaltname") == 0) {
                printf("[+] add_altname successful\n");
            }
        } else {
            printf("[-] Failed to find mywlan in child\n");
        }

        // Tell parent we are done
        write(fds_c2p[1], "x", 1);

        sleep(100);
        exit(0);
    }

    close(fds_p2c[0]);
    close(fds_c2p[1]);

    // Wait for child to unshare
    char c;
    read(fds_c2p[0], &c, 1);

    // Move wiphy to ns1
    for (int i = 0; i < 10; i++) {
        if (set_wiphy_netns(genl_fd, nl80211_id, i, pid1) == 0) {
            printf("[+] set_wiphy_netns %d successful\n", i);
        }
    }

    // Tell child to proceed
    if (write(fds_p2c[1], "x", 1) < 0) {
        printf("[-] write failed: %s\n", strerror(errno));
    }

    // Wait for child to setup mywlan and altname
    read(fds_c2p[0], &c, 1);

    // 6. Destroy ns1 (triggers cfg80211_pernet_exit and default_device_exit_batch)
    printf("[+] Killing ns1\n");
    kill(pid1, SIGKILL);
    waitpid(pid1, NULL, 0);
    
    // Wait for cleanup_net to finish destroying ns1
    printf("[+] Waiting for cleanup_net\n");
    sleep(3);

    // 7. Delete radio in init_net (triggers mac80211 interface removal)
    printf("[+] Deleting radio\n");
    for (int i = 0; i < 10; i++) {
        if (delete_radio(genl_fd, hwsim_id, i) == 0) {
            printf("[+] delete_radio %d successful\n", i);
        }
    }

    sleep(2);
    printf("[+] Done\n");
    return 0;
}
