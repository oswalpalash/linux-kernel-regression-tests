// https://syzkaller.appspot.com/bug?id=531a744660b6e3c4949e68851d679e035082b85c
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <linux/capability.h>
#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/usb/ch9.h>
#include <linux/veth.h>

unsigned long long procid;

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* ctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  exit(sig);
}

static void install_segv_handler(void)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                        \
  {                                                                            \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
    if (_setjmp(segv_env) == 0) {                                              \
      __VA_ARGS__;                                                             \
    }                                                                          \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);                       \
  }

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static bool write_file(const char* file, const char* what, ...)
{
  char buf[1024];
  va_list args;
  va_start(args, what);
  vsnprintf(buf, sizeof(buf), what, args);
  va_end(args);
  buf[sizeof(buf) - 1] = 0;
  int len = strlen(buf);
  int fd = open(file, O_WRONLY | O_CLOEXEC);
  if (fd == -1)
    return false;
  if (write(fd, buf, len) != len) {
    int err = errno;
    close(fd);
    errno = err;
    return false;
  }
  close(fd);
  return true;
}

struct nlmsg {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[1024];
};

static struct nlmsg nlmsg;

static void netlink_init(struct nlmsg* nlmsg, int typ, int flags,
                         const void* data, int size)
{
  memset(nlmsg, 0, sizeof(*nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg->pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(struct nlmsg* nlmsg, int typ, const void* data,
                         int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg->pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  memcpy(attr + 1, data, size);
  nlmsg->pos += NLMSG_ALIGN(attr->nla_len);
}

static int netlink_send_ext(struct nlmsg* nlmsg, int sock, uint16_t reply_type,
                            int* reply_len)
{
  if (nlmsg->pos > nlmsg->buf + sizeof(nlmsg->buf) || nlmsg->nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_len = nlmsg->pos - nlmsg->buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  unsigned n = sendto(sock, nlmsg->buf, hdr->nlmsg_len, 0,
                      (struct sockaddr*)&addr, sizeof(addr));
  if (n != hdr->nlmsg_len)
    exit(1);
  n = recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  if (hdr->nlmsg_type == NLMSG_DONE) {
    *reply_len = 0;
    return 0;
  }
  if (n < sizeof(struct nlmsghdr))
    exit(1);
  if (reply_len && hdr->nlmsg_type == reply_type) {
    *reply_len = n;
    return 0;
  }
  if (n < sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))
    exit(1);
  if (hdr->nlmsg_type != NLMSG_ERROR)
    exit(1);
  return -((struct nlmsgerr*)(hdr + 1))->error;
}

static int netlink_send(struct nlmsg* nlmsg, int sock)
{
  return netlink_send_ext(nlmsg, sock, 0, NULL);
}

static int netlink_next_msg(struct nlmsg* nlmsg, unsigned int offset,
                            unsigned int total_len)
{
  struct nlmsghdr* hdr = (struct nlmsghdr*)(nlmsg->buf + offset);
  if (offset == total_len || offset + hdr->nlmsg_len > total_len)
    return -1;
  return hdr->nlmsg_len;
}

static void netlink_device_change(struct nlmsg* nlmsg, int sock,
                                  const char* name, bool up, const char* master,
                                  const void* mac, int macsize,
                                  const char* new_name)
{
  struct ifinfomsg hdr;
  memset(&hdr, 0, sizeof(hdr));
  if (up)
    hdr.ifi_flags = hdr.ifi_change = IFF_UP;
  hdr.ifi_index = if_nametoindex(name);
  netlink_init(nlmsg, RTM_NEWLINK, 0, &hdr, sizeof(hdr));
  if (new_name)
    netlink_attr(nlmsg, IFLA_IFNAME, new_name, strlen(new_name));
  if (master) {
    int ifindex = if_nametoindex(master);
    netlink_attr(nlmsg, IFLA_MASTER, &ifindex, sizeof(ifindex));
  }
  if (macsize)
    netlink_attr(nlmsg, IFLA_ADDRESS, mac, macsize);
  int err = netlink_send(nlmsg, sock);
  (void)err;
}

const int kInitNetNsFd = 239;

#define DEVLINK_FAMILY_NAME "devlink"

#define DEVLINK_CMD_PORT_GET 5
#define DEVLINK_CMD_RELOAD 37
#define DEVLINK_ATTR_BUS_NAME 1
#define DEVLINK_ATTR_DEV_NAME 2
#define DEVLINK_ATTR_NETDEV_NAME 7
#define DEVLINK_ATTR_NETNS_FD 138

static int netlink_devlink_id_get(struct nlmsg* nlmsg, int sock)
{
  struct genlmsghdr genlhdr;
  struct nlattr* attr;
  int err, n;
  uint16_t id = 0;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, DEVLINK_FAMILY_NAME,
               strlen(DEVLINK_FAMILY_NAME) + 1);
  err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n);
  if (err) {
    return -1;
  }
  attr = (struct nlattr*)(nlmsg->buf + NLMSG_HDRLEN +
                          NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg->buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    return -1;
  }
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0); /* recv ack */
  return id;
}

static void netlink_devlink_netns_move(const char* bus_name,
                                       const char* dev_name, int netns_fd)
{
  struct genlmsghdr genlhdr;
  int sock;
  int id, err;
  sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  id = netlink_devlink_id_get(&nlmsg, sock);
  if (id == -1)
    goto error;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_RELOAD;
  netlink_init(&nlmsg, id, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_NETNS_FD, &netns_fd, sizeof(netns_fd));
  err = netlink_send(&nlmsg, sock);
  if (err) {
  }
error:
  close(sock);
}

static struct nlmsg nlmsg2;

static void initialize_devlink_ports(const char* bus_name, const char* dev_name,
                                     const char* netdev_prefix)
{
  struct genlmsghdr genlhdr;
  int len, total_len, id, err, offset;
  uint16_t netdev_index;
  int sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  int rtsock = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (rtsock == -1)
    exit(1);
  id = netlink_devlink_id_get(&nlmsg, sock);
  if (id == -1)
    goto error;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_PORT_GET;
  netlink_init(&nlmsg, id, NLM_F_DUMP, &genlhdr, sizeof(genlhdr));
  netlink_attr(&nlmsg, DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(&nlmsg, DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  err = netlink_send_ext(&nlmsg, sock, id, &total_len);
  if (err) {
    goto error;
  }
  offset = 0;
  netdev_index = 0;
  while ((len = netlink_next_msg(&nlmsg, offset, total_len)) != -1) {
    struct nlattr* attr = (struct nlattr*)(nlmsg.buf + offset + NLMSG_HDRLEN +
                                           NLMSG_ALIGN(sizeof(genlhdr)));
    for (; (char*)attr < nlmsg.buf + offset + len;
         attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
      if (attr->nla_type == DEVLINK_ATTR_NETDEV_NAME) {
        char* port_name;
        char netdev_name[IFNAMSIZ];
        port_name = (char*)(attr + 1);
        snprintf(netdev_name, sizeof(netdev_name), "%s%d", netdev_prefix,
                 netdev_index);
        netlink_device_change(&nlmsg2, rtsock, port_name, true, 0, 0, 0,
                              netdev_name);
        break;
      }
    }
    offset += len;
    netdev_index++;
  }
error:
  close(rtsock);
  close(sock);
}

static void initialize_devlink_pci(void)
{
  int netns = open("/proc/self/ns/net", O_RDONLY);
  if (netns == -1)
    exit(1);
  int ret = setns(kInitNetNsFd, 0);
  if (ret == -1)
    exit(1);
  netlink_devlink_netns_move("pci", "0000:00:10.0", netns);
  ret = setns(netns, 0);
  if (ret == -1)
    exit(1);
  close(netns);
  initialize_devlink_ports("pci", "0000:00:10.0", "netpci");
}

#define MAX_FDS 30

#define USB_MAX_IFACE_NUM 4
#define USB_MAX_EP_NUM 32

struct usb_iface_index {
  struct usb_interface_descriptor* iface;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bInterfaceClass;
  struct usb_endpoint_descriptor eps[USB_MAX_EP_NUM];
  int eps_num;
};

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
  uint8_t bDeviceClass;
  uint8_t bMaxPower;
  int config_length;
  struct usb_iface_index ifaces[USB_MAX_IFACE_NUM];
  int ifaces_num;
  int iface_cur;
};

static bool parse_usb_descriptor(char* buffer, size_t length,
                                 struct usb_device_index* index)
{
  if (length < sizeof(*index->dev) + sizeof(*index->config))
    return false;
  memset(index, 0, sizeof(*index));
  index->dev = (struct usb_device_descriptor*)buffer;
  index->config = (struct usb_config_descriptor*)(buffer + sizeof(*index->dev));
  index->bDeviceClass = index->dev->bDeviceClass;
  index->bMaxPower = index->config->bMaxPower;
  index->config_length = length - sizeof(*index->dev);
  index->iface_cur = -1;
  size_t offset = 0;
  while (true) {
    if (offset + 1 >= length)
      break;
    uint8_t desc_length = buffer[offset];
    uint8_t desc_type = buffer[offset + 1];
    if (desc_length <= 2)
      break;
    if (offset + desc_length > length)
      break;
    if (desc_type == USB_DT_INTERFACE &&
        index->ifaces_num < USB_MAX_IFACE_NUM) {
      struct usb_interface_descriptor* iface =
          (struct usb_interface_descriptor*)(buffer + offset);
      index->ifaces[index->ifaces_num].iface = iface;
      index->ifaces[index->ifaces_num].bInterfaceNumber =
          iface->bInterfaceNumber;
      index->ifaces[index->ifaces_num].bAlternateSetting =
          iface->bAlternateSetting;
      index->ifaces[index->ifaces_num].bInterfaceClass = iface->bInterfaceClass;
      index->ifaces_num++;
    }
    if (desc_type == USB_DT_ENDPOINT && index->ifaces_num > 0) {
      struct usb_iface_index* iface = &index->ifaces[index->ifaces_num - 1];
      if (iface->eps_num < USB_MAX_EP_NUM) {
        memcpy(&iface->eps[iface->eps_num], buffer + offset,
               sizeof(iface->eps[iface->eps_num]));
        iface->eps_num++;
      }
    }
    offset += desc_length;
  }
  return true;
}

#define UDC_NAME_LENGTH_MAX 128

struct usb_raw_init {
  __u8 driver_name[UDC_NAME_LENGTH_MAX];
  __u8 device_name[UDC_NAME_LENGTH_MAX];
  __u8 speed;
};

enum usb_raw_event_type {
  USB_RAW_EVENT_INVALID = 0,
  USB_RAW_EVENT_CONNECT = 1,
  USB_RAW_EVENT_CONTROL = 2,
};

struct usb_raw_event {
  __u32 type;
  __u32 length;
  __u8 data[0];
};

struct usb_raw_ep_io {
  __u16 ep;
  __u16 flags;
  __u32 length;
  __u8 data[0];
};

#define USB_RAW_IOCTL_INIT _IOW('U', 0, struct usb_raw_init)
#define USB_RAW_IOCTL_RUN _IO('U', 1)
#define USB_RAW_IOCTL_EVENT_FETCH _IOR('U', 2, struct usb_raw_event)
#define USB_RAW_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP0_READ _IOWR('U', 4, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_ENABLE _IOW('U', 5, struct usb_endpoint_descriptor)
#define USB_RAW_IOCTL_EP_DISABLE _IOW('U', 6, __u32)
#define USB_RAW_IOCTL_EP_WRITE _IOW('U', 7, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_READ _IOWR('U', 8, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_CONFIGURE _IO('U', 9)
#define USB_RAW_IOCTL_VBUS_DRAW _IOW('U', 10, __u32)

static int usb_raw_open()
{
  return open("/dev/raw-gadget", O_RDWR);
}

static int usb_raw_init(int fd, uint32_t speed, const char* driver,
                        const char* device)
{
  struct usb_raw_init arg;
  strncpy((char*)&arg.driver_name[0], driver, sizeof(arg.driver_name));
  strncpy((char*)&arg.device_name[0], device, sizeof(arg.device_name));
  arg.speed = speed;
  return ioctl(fd, USB_RAW_IOCTL_INIT, &arg);
}

static int usb_raw_run(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_RUN, 0);
}

static int usb_raw_event_fetch(int fd, struct usb_raw_event* event)
{
  return ioctl(fd, USB_RAW_IOCTL_EVENT_FETCH, event);
}

static int usb_raw_ep0_write(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_WRITE, io);
}

static int usb_raw_ep0_read(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_READ, io);
}

static int usb_raw_ep_enable(int fd, struct usb_endpoint_descriptor* desc)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_ENABLE, desc);
}

static int usb_raw_ep_disable(int fd, int ep)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_DISABLE, ep);
}

static int usb_raw_configure(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_CONFIGURE, 0);
}

static int usb_raw_vbus_draw(int fd, uint32_t power)
{
  return ioctl(fd, USB_RAW_IOCTL_VBUS_DRAW, power);
}

#define MAX_USB_FDS 6

struct usb_info {
  int fd;
  struct usb_device_index index;
};

static struct usb_info usb_devices[MAX_USB_FDS];
static int usb_devices_num;

static struct usb_device_index* add_usb_index(int fd, char* dev, size_t dev_len)
{
  int i = __atomic_fetch_add(&usb_devices_num, 1, __ATOMIC_RELAXED);
  if (i >= MAX_USB_FDS)
    return NULL;
  int rv = 0;
  NONFAILING(rv = parse_usb_descriptor(dev, dev_len, &usb_devices[i].index));
  if (!rv)
    return NULL;
  __atomic_store_n(&usb_devices[i].fd, fd, __ATOMIC_RELEASE);
  return &usb_devices[i].index;
}

static struct usb_device_index* lookup_usb_index(int fd)
{
  int i;
  for (i = 0; i < MAX_USB_FDS; i++) {
    if (__atomic_load_n(&usb_devices[i].fd, __ATOMIC_ACQUIRE) == fd) {
      return &usb_devices[i].index;
    }
  }
  return NULL;
}

static void set_interface(int fd, int n)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  int ep;
  if (!index)
    return;
  if (index->iface_cur >= 0 && index->iface_cur < index->ifaces_num) {
    for (ep = 0; ep < index->ifaces[index->iface_cur].eps_num; ep++) {
      int rv = usb_raw_ep_disable(fd, ep);
      if (rv < 0) {
      } else {
      }
    }
  }
  if (n >= 0 && n < index->ifaces_num) {
    for (ep = 0; ep < index->ifaces[n].eps_num; ep++) {
      int rv = usb_raw_ep_enable(fd, &index->ifaces[n].eps[ep]);
      if (rv < 0) {
      } else {
      }
    }
    index->iface_cur = n;
  }
}

static int configure_device(int fd)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  if (!index)
    return -1;
  int rv = usb_raw_vbus_draw(fd, index->bMaxPower);
  if (rv < 0) {
    return rv;
  }
  rv = usb_raw_configure(fd);
  if (rv < 0) {
    return rv;
  }
  set_interface(fd, 0);
  return 0;
}

#define USB_MAX_PACKET_SIZE 1024

struct usb_raw_control_event {
  struct usb_raw_event inner;
  struct usb_ctrlrequest ctrl;
  char data[USB_MAX_PACKET_SIZE];
};

struct usb_raw_ep_io_data {
  struct usb_raw_ep_io inner;
  char data[USB_MAX_PACKET_SIZE];
};

struct vusb_connect_string_descriptor {
  uint32_t len;
  char* str;
} __attribute__((packed));

struct vusb_connect_descriptors {
  uint32_t qual_len;
  char* qual;
  uint32_t bos_len;
  char* bos;
  uint32_t strs_len;
  struct vusb_connect_string_descriptor strs[0];
} __attribute__((packed));

static const char default_string[] = {8, USB_DT_STRING, 's', 0, 'y', 0, 'z', 0};

static const char default_lang_id[] = {4, USB_DT_STRING, 0x09, 0x04};

static bool lookup_connect_response(int fd,
                                    struct vusb_connect_descriptors* descs,
                                    struct usb_ctrlrequest* ctrl,
                                    char** response_data,
                                    uint32_t* response_length)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  uint8_t str_idx;
  if (!index)
    return false;
  switch (ctrl->bRequestType & USB_TYPE_MASK) {
  case USB_TYPE_STANDARD:
    switch (ctrl->bRequest) {
    case USB_REQ_GET_DESCRIPTOR:
      switch (ctrl->wValue >> 8) {
      case USB_DT_DEVICE:
        *response_data = (char*)index->dev;
        *response_length = sizeof(*index->dev);
        return true;
      case USB_DT_CONFIG:
        *response_data = (char*)index->config;
        *response_length = index->config_length;
        return true;
      case USB_DT_STRING:
        str_idx = (uint8_t)ctrl->wValue;
        if (descs && str_idx < descs->strs_len) {
          *response_data = descs->strs[str_idx].str;
          *response_length = descs->strs[str_idx].len;
          return true;
        }
        if (str_idx == 0) {
          *response_data = (char*)&default_lang_id[0];
          *response_length = default_lang_id[0];
          return true;
        }
        *response_data = (char*)&default_string[0];
        *response_length = default_string[0];
        return true;
      case USB_DT_BOS:
        *response_data = descs->bos;
        *response_length = descs->bos_len;
        return true;
      case USB_DT_DEVICE_QUALIFIER:
        if (!descs->qual) {
          struct usb_qualifier_descriptor* qual =
              (struct usb_qualifier_descriptor*)response_data;
          qual->bLength = sizeof(*qual);
          qual->bDescriptorType = USB_DT_DEVICE_QUALIFIER;
          qual->bcdUSB = index->dev->bcdUSB;
          qual->bDeviceClass = index->dev->bDeviceClass;
          qual->bDeviceSubClass = index->dev->bDeviceSubClass;
          qual->bDeviceProtocol = index->dev->bDeviceProtocol;
          qual->bMaxPacketSize0 = index->dev->bMaxPacketSize0;
          qual->bNumConfigurations = index->dev->bNumConfigurations;
          qual->bRESERVED = 0;
          *response_length = sizeof(*qual);
          return true;
        }
        *response_data = descs->qual;
        *response_length = descs->qual_len;
        return true;
      default:
        exit(1);
        return false;
      }
      break;
    default:
      exit(1);
      return false;
    }
    break;
  default:
    exit(1);
    return false;
  }
  return false;
}

static volatile long syz_usb_connect(volatile long a0, volatile long a1,
                                     volatile long a2, volatile long a3)
{
  uint64_t speed = a0;
  uint64_t dev_len = a1;
  char* dev = (char*)a2;
  struct vusb_connect_descriptors* descs = (struct vusb_connect_descriptors*)a3;
  if (!dev) {
    return -1;
  }
  int fd = usb_raw_open();
  if (fd < 0) {
    return fd;
  }
  if (fd >= MAX_FDS) {
    close(fd);
    return -1;
  }
  struct usb_device_index* index = add_usb_index(fd, dev, dev_len);
  if (!index) {
    return -1;
  }
  char device[32];
  sprintf(&device[0], "dummy_udc.%llu", procid);
  int rv = usb_raw_init(fd, speed, "dummy_udc", &device[0]);
  if (rv < 0) {
    return rv;
  }
  rv = usb_raw_run(fd);
  if (rv < 0) {
    return rv;
  }
  bool done = false;
  while (!done) {
    struct usb_raw_control_event event;
    event.inner.type = 0;
    event.inner.length = sizeof(event.ctrl);
    rv = usb_raw_event_fetch(fd, (struct usb_raw_event*)&event);
    if (rv < 0) {
      return rv;
    }
    if (event.inner.type != USB_RAW_EVENT_CONTROL)
      continue;
    bool response_found = false;
    char* response_data = NULL;
    uint32_t response_length = 0;
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      NONFAILING(response_found = lookup_connect_response(
                     fd, descs, &event.ctrl, &response_data, &response_length));
      if (!response_found) {
        return -1;
      }
    } else {
      if ((event.ctrl.bRequestType & USB_TYPE_MASK) != USB_TYPE_STANDARD ||
          event.ctrl.bRequest != USB_REQ_SET_CONFIGURATION) {
        exit(1);
        return -1;
      }
      done = true;
    }
    if (done) {
      rv = configure_device(fd);
      if (rv < 0) {
        return rv;
      }
    }
    struct usb_raw_ep_io_data response;
    response.inner.ep = 0;
    response.inner.flags = 0;
    if (response_length > sizeof(response.data))
      response_length = 0;
    if (event.ctrl.wLength < response_length)
      response_length = event.ctrl.wLength;
    response.inner.length = response_length;
    if (response_data)
      memcpy(&response.data[0], response_data, response_length);
    else
      memset(&response.data[0], 0, response_length);
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      rv = usb_raw_ep0_write(fd, (struct usb_raw_ep_io*)&response);
    } else {
      rv = usb_raw_ep0_read(fd, (struct usb_raw_ep_io*)&response);
    }
    if (rv < 0) {
      return rv;
    }
  }
  sleep_ms(200);
  return fd;
}

static void setup_common()
{
  if (mount(0, "/sys/fs/fuse/connections", "fusectl", 0, 0)) {
  }
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();
  int netns = open("/proc/self/ns/net", O_RDONLY);
  if (netns == -1)
    exit(1);
  if (dup2(netns, kInitNetNsFd) < 0)
    exit(1);
  close(netns);
  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = (200 << 20);
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 32 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 136 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 256;
  setrlimit(RLIMIT_NOFILE, &rlim);
  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(0x02000000)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
  typedef struct {
    const char* name;
    const char* value;
  } sysctl_t;
  static const sysctl_t sysctls[] = {
      {"/proc/sys/kernel/shmmax", "16777216"},
      {"/proc/sys/kernel/shmall", "536870912"},
      {"/proc/sys/kernel/shmmni", "1024"},
      {"/proc/sys/kernel/msgmax", "8192"},
      {"/proc/sys/kernel/msgmni", "1024"},
      {"/proc/sys/kernel/msgmnb", "1024"},
      {"/proc/sys/kernel/sem", "1024 1048576 500 1024"},
  };
  unsigned i;
  for (i = 0; i < sizeof(sysctls) / sizeof(sysctls[0]); i++)
    write_file(sysctls[i].name, sysctls[i].value);
}

int wait_for_loop(int pid)
{
  if (pid < 0)
    exit(1);
  int status = 0;
  while (waitpid(-1, &status, __WALL) != pid) {
  }
  return WEXITSTATUS(status);
}

static void drop_caps(void)
{
  struct __user_cap_header_struct cap_hdr = {};
  struct __user_cap_data_struct cap_data[2] = {};
  cap_hdr.version = _LINUX_CAPABILITY_VERSION_3;
  cap_hdr.pid = getpid();
  if (syscall(SYS_capget, &cap_hdr, &cap_data))
    exit(1);
  const int drop = (1 << CAP_SYS_PTRACE) | (1 << CAP_SYS_NICE);
  cap_data[0].effective &= ~drop;
  cap_data[0].permitted &= ~drop;
  cap_data[0].inheritable &= ~drop;
  if (syscall(SYS_capset, &cap_hdr, &cap_data))
    exit(1);
}

static int do_sandbox_none(void)
{
  int pid = fork();
  if (pid != 0)
    return wait_for_loop(pid);
  setup_common();
  sandbox_common();
  drop_caps();
  if (unshare(CLONE_NEWNET)) {
  }
  initialize_devlink_pci();
  loop();
  exit(1);
}

static void close_fds()
{
  int fd;
  for (fd = 3; fd < MAX_FDS; fd++)
    close(fd);
}

static void setup_binfmt_misc()
{
  if (mount(0, "/proc/sys/fs/binfmt_misc", "binfmt_misc", 0, 0)) {
  }
  write_file("/proc/sys/fs/binfmt_misc/register", ":syz0:M:0:\x01::./file0:");
  write_file("/proc/sys/fs/binfmt_misc/register",
             ":syz1:M:1:\x02::./file0:POC");
}

void loop(void)
{
  NONFAILING(*(uint8_t*)0x20000000 = 0x12);
  NONFAILING(*(uint8_t*)0x20000001 = 1);
  NONFAILING(*(uint16_t*)0x20000002 = 0x300);
  NONFAILING(*(uint8_t*)0x20000004 = 0xa3);
  NONFAILING(*(uint8_t*)0x20000005 = 0xfd);
  NONFAILING(*(uint8_t*)0x20000006 = 0xe9);
  NONFAILING(*(uint8_t*)0x20000007 = 8);
  NONFAILING(*(uint16_t*)0x20000008 = 0xccd);
  NONFAILING(*(uint16_t*)0x2000000a = 0x39);
  NONFAILING(*(uint16_t*)0x2000000c = 0x529);
  NONFAILING(*(uint8_t*)0x2000000e = 1);
  NONFAILING(*(uint8_t*)0x2000000f = 2);
  NONFAILING(*(uint8_t*)0x20000010 = 3);
  NONFAILING(*(uint8_t*)0x20000011 = 1);
  NONFAILING(*(uint8_t*)0x20000012 = 9);
  NONFAILING(*(uint8_t*)0x20000013 = 2);
  NONFAILING(*(uint16_t*)0x20000014 = 0xe7);
  NONFAILING(*(uint8_t*)0x20000016 = 1);
  NONFAILING(*(uint8_t*)0x20000017 = 0x22);
  NONFAILING(*(uint8_t*)0x20000018 = 0x3f);
  NONFAILING(*(uint8_t*)0x20000019 = 0x80);
  NONFAILING(*(uint8_t*)0x2000001a = 0xea);
  NONFAILING(*(uint8_t*)0x2000001b = 9);
  NONFAILING(*(uint8_t*)0x2000001c = 4);
  NONFAILING(*(uint8_t*)0x2000001d = 0xbb);
  NONFAILING(*(uint8_t*)0x2000001e = 4);
  NONFAILING(*(uint8_t*)0x2000001f = 0xa);
  NONFAILING(*(uint8_t*)0x20000020 = 0xcb);
  NONFAILING(*(uint8_t*)0x20000021 = 0x40);
  NONFAILING(*(uint8_t*)0x20000022 = 0xec);
  NONFAILING(*(uint8_t*)0x20000023 = 0x80);
  NONFAILING(*(uint8_t*)0x20000024 = 9);
  NONFAILING(*(uint8_t*)0x20000025 = 0x21);
  NONFAILING(*(uint16_t*)0x20000026 = 0x401);
  NONFAILING(*(uint8_t*)0x20000028 = 0);
  NONFAILING(*(uint8_t*)0x20000029 = 1);
  NONFAILING(*(uint8_t*)0x2000002a = 0x22);
  NONFAILING(*(uint16_t*)0x2000002b = 0x22a);
  NONFAILING(*(uint8_t*)0x2000002d = 8);
  NONFAILING(*(uint8_t*)0x2000002e = 0x24);
  NONFAILING(*(uint8_t*)0x2000002f = 2);
  NONFAILING(*(uint8_t*)0x20000030 = 1);
  NONFAILING(*(uint8_t*)0x20000031 = 0x40);
  NONFAILING(*(uint8_t*)0x20000032 = 4);
  NONFAILING(*(uint8_t*)0x20000033 = 0x3f);
  NONFAILING(*(uint8_t*)0x20000034 = 6);
  NONFAILING(*(uint8_t*)0x20000035 = 7);
  NONFAILING(*(uint8_t*)0x20000036 = 0x24);
  NONFAILING(*(uint8_t*)0x20000037 = 1);
  NONFAILING(*(uint8_t*)0x20000038 = 0xc4);
  NONFAILING(*(uint8_t*)0x20000039 = 0);
  NONFAILING(*(uint16_t*)0x2000003a = 0x1001);
  NONFAILING(*(uint8_t*)0x2000003c = 7);
  NONFAILING(*(uint8_t*)0x2000003d = 0x24);
  NONFAILING(*(uint8_t*)0x2000003e = 1);
  NONFAILING(*(uint8_t*)0x2000003f = 0x7f);
  NONFAILING(*(uint8_t*)0x20000040 = 6);
  NONFAILING(*(uint16_t*)0x20000041 = 1);
  NONFAILING(*(uint8_t*)0x20000043 = 9);
  NONFAILING(*(uint8_t*)0x20000044 = 0x24);
  NONFAILING(*(uint8_t*)0x20000045 = 2);
  NONFAILING(*(uint8_t*)0x20000046 = 2);
  NONFAILING(*(uint16_t*)0x20000047 = 5);
  NONFAILING(*(uint16_t*)0x20000049 = 0xdc7);
  NONFAILING(*(uint8_t*)0x2000004b = 3);
  NONFAILING(*(uint8_t*)0x2000004c = 8);
  NONFAILING(*(uint8_t*)0x2000004d = 0x24);
  NONFAILING(*(uint8_t*)0x2000004e = 2);
  NONFAILING(*(uint8_t*)0x2000004f = 1);
  NONFAILING(*(uint8_t*)0x20000050 = 0x3f);
  NONFAILING(*(uint8_t*)0x20000051 = 2);
  NONFAILING(*(uint8_t*)0x20000052 = 1);
  NONFAILING(*(uint8_t*)0x20000053 = 0x51);
  NONFAILING(*(uint8_t*)0x20000054 = 7);
  NONFAILING(*(uint8_t*)0x20000055 = 0x24);
  NONFAILING(*(uint8_t*)0x20000056 = 1);
  NONFAILING(*(uint8_t*)0x20000057 = 0x1f);
  NONFAILING(*(uint8_t*)0x20000058 = 0xc1);
  NONFAILING(*(uint16_t*)0x20000059 = 0);
  NONFAILING(*(uint8_t*)0x2000005b = 9);
  NONFAILING(*(uint8_t*)0x2000005c = 5);
  NONFAILING(*(uint8_t*)0x2000005d = 0xc);
  NONFAILING(*(uint8_t*)0x2000005e = 0xc);
  NONFAILING(*(uint16_t*)0x2000005f = 0x10);
  NONFAILING(*(uint8_t*)0x20000061 = 8);
  NONFAILING(*(uint8_t*)0x20000062 = 9);
  NONFAILING(*(uint8_t*)0x20000063 = 0x40);
  NONFAILING(*(uint8_t*)0x20000064 = 2);
  NONFAILING(*(uint8_t*)0x20000065 = 0x23);
  NONFAILING(*(uint8_t*)0x20000066 = 9);
  NONFAILING(*(uint8_t*)0x20000067 = 5);
  NONFAILING(*(uint8_t*)0x20000068 = 7);
  NONFAILING(*(uint8_t*)0x20000069 = 1);
  NONFAILING(*(uint16_t*)0x2000006a = 0x400);
  NONFAILING(*(uint8_t*)0x2000006c = 9);
  NONFAILING(*(uint8_t*)0x2000006d = 8);
  NONFAILING(*(uint8_t*)0x2000006e = 0);
  NONFAILING(*(uint8_t*)0x2000006f = 2);
  NONFAILING(*(uint8_t*)0x20000070 = 5);
  NONFAILING(*(uint8_t*)0x20000071 = 7);
  NONFAILING(*(uint8_t*)0x20000072 = 0x25);
  NONFAILING(*(uint8_t*)0x20000073 = 1);
  NONFAILING(*(uint8_t*)0x20000074 = 0);
  NONFAILING(*(uint8_t*)0x20000075 = 0xa0);
  NONFAILING(*(uint16_t*)0x20000076 = 0xa6e5);
  NONFAILING(*(uint8_t*)0x20000078 = 9);
  NONFAILING(*(uint8_t*)0x20000079 = 5);
  NONFAILING(*(uint8_t*)0x2000007a = 0xd);
  NONFAILING(*(uint8_t*)0x2000007b = 0x10);
  NONFAILING(*(uint16_t*)0x2000007c = 0x20);
  NONFAILING(*(uint8_t*)0x2000007e = -1);
  NONFAILING(*(uint8_t*)0x2000007f = 0);
  NONFAILING(*(uint8_t*)0x20000080 = 7);
  NONFAILING(*(uint8_t*)0x20000081 = 9);
  NONFAILING(*(uint8_t*)0x20000082 = 5);
  NONFAILING(*(uint8_t*)0x20000083 = 7);
  NONFAILING(*(uint8_t*)0x20000084 = 1);
  NONFAILING(*(uint16_t*)0x20000085 = 0x666);
  NONFAILING(*(uint8_t*)0x20000087 = 0);
  NONFAILING(*(uint8_t*)0x20000088 = 8);
  NONFAILING(*(uint8_t*)0x20000089 = 2);
  NONFAILING(*(uint8_t*)0x2000008a = 7);
  NONFAILING(*(uint8_t*)0x2000008b = 0x25);
  NONFAILING(*(uint8_t*)0x2000008c = 1);
  NONFAILING(*(uint8_t*)0x2000008d = 1);
  NONFAILING(*(uint8_t*)0x2000008e = 9);
  NONFAILING(*(uint16_t*)0x2000008f = 0xe7e);
  NONFAILING(*(uint8_t*)0x20000091 = 2);
  NONFAILING(*(uint8_t*)0x20000092 = 9);
  NONFAILING(*(uint8_t*)0x20000093 = 9);
  NONFAILING(*(uint8_t*)0x20000094 = 5);
  NONFAILING(*(uint8_t*)0x20000095 = 0xd);
  NONFAILING(*(uint8_t*)0x20000096 = 0x10);
  NONFAILING(*(uint16_t*)0x20000097 = 0x40);
  NONFAILING(*(uint8_t*)0x20000099 = 0xea);
  NONFAILING(*(uint8_t*)0x2000009a = 2);
  NONFAILING(*(uint8_t*)0x2000009b = 0x20);
  NONFAILING(*(uint8_t*)0x2000009c = 7);
  NONFAILING(*(uint8_t*)0x2000009d = 0x25);
  NONFAILING(*(uint8_t*)0x2000009e = 1);
  NONFAILING(*(uint8_t*)0x2000009f = 2);
  NONFAILING(*(uint8_t*)0x200000a0 = 0xe3);
  NONFAILING(*(uint16_t*)0x200000a1 = 0x1e2);
  NONFAILING(*(uint8_t*)0x200000a3 = 9);
  NONFAILING(*(uint8_t*)0x200000a4 = 5);
  NONFAILING(*(uint8_t*)0x200000a5 = 5);
  NONFAILING(*(uint8_t*)0x200000a6 = 0x10);
  NONFAILING(*(uint16_t*)0x200000a7 = 0x10);
  NONFAILING(*(uint8_t*)0x200000a9 = 0x25);
  NONFAILING(*(uint8_t*)0x200000aa = 5);
  NONFAILING(*(uint8_t*)0x200000ab = 0x1f);
  NONFAILING(*(uint8_t*)0x200000ac = 2);
  NONFAILING(*(uint8_t*)0x200000ad = 9);
  NONFAILING(*(uint8_t*)0x200000ae = 7);
  NONFAILING(*(uint8_t*)0x200000af = 0x25);
  NONFAILING(*(uint8_t*)0x200000b0 = 1);
  NONFAILING(*(uint8_t*)0x200000b1 = 1);
  NONFAILING(*(uint8_t*)0x200000b2 = -1);
  NONFAILING(*(uint16_t*)0x200000b3 = 0x101);
  NONFAILING(*(uint8_t*)0x200000b5 = 9);
  NONFAILING(*(uint8_t*)0x200000b6 = 5);
  NONFAILING(*(uint8_t*)0x200000b7 = 1);
  NONFAILING(*(uint8_t*)0x200000b8 = 0);
  NONFAILING(*(uint16_t*)0x200000b9 = 0x200);
  NONFAILING(*(uint8_t*)0x200000bb = 4);
  NONFAILING(*(uint8_t*)0x200000bc = 0x40);
  NONFAILING(*(uint8_t*)0x200000bd = 2);
  NONFAILING(*(uint8_t*)0x200000be = 7);
  NONFAILING(*(uint8_t*)0x200000bf = 0x25);
  NONFAILING(*(uint8_t*)0x200000c0 = 1);
  NONFAILING(*(uint8_t*)0x200000c1 = 0x81);
  NONFAILING(*(uint8_t*)0x200000c2 = 0);
  NONFAILING(*(uint16_t*)0x200000c3 = 0x1ff);
  NONFAILING(*(uint8_t*)0x200000c5 = 7);
  NONFAILING(*(uint8_t*)0x200000c6 = 0x25);
  NONFAILING(*(uint8_t*)0x200000c7 = 1);
  NONFAILING(*(uint8_t*)0x200000c8 = 3);
  NONFAILING(*(uint8_t*)0x200000c9 = 8);
  NONFAILING(*(uint16_t*)0x200000ca = 0xff4e);
  NONFAILING(*(uint8_t*)0x200000cc = 9);
  NONFAILING(*(uint8_t*)0x200000cd = 5);
  NONFAILING(*(uint8_t*)0x200000ce = 2);
  NONFAILING(*(uint8_t*)0x200000cf = 0xc);
  NONFAILING(*(uint16_t*)0x200000d0 = 0x40);
  NONFAILING(*(uint8_t*)0x200000d2 = 0);
  NONFAILING(*(uint8_t*)0x200000d3 = 1);
  NONFAILING(*(uint8_t*)0x200000d4 = 0xd2);
  NONFAILING(*(uint8_t*)0x200000d5 = 9);
  NONFAILING(*(uint8_t*)0x200000d6 = 5);
  NONFAILING(*(uint8_t*)0x200000d7 = 1);
  NONFAILING(*(uint8_t*)0x200000d8 = 0x10);
  NONFAILING(*(uint16_t*)0x200000d9 = 0x40);
  NONFAILING(*(uint8_t*)0x200000db = 0);
  NONFAILING(*(uint8_t*)0x200000dc = 3);
  NONFAILING(*(uint8_t*)0x200000dd = 0x6e);
  NONFAILING(*(uint8_t*)0x200000de = 7);
  NONFAILING(*(uint8_t*)0x200000df = 0x25);
  NONFAILING(*(uint8_t*)0x200000e0 = 1);
  NONFAILING(*(uint8_t*)0x200000e1 = 0x80);
  NONFAILING(*(uint8_t*)0x200000e2 = 3);
  NONFAILING(*(uint16_t*)0x200000e3 = 0x397);
  NONFAILING(*(uint8_t*)0x200000e5 = 7);
  NONFAILING(*(uint8_t*)0x200000e6 = 0x25);
  NONFAILING(*(uint8_t*)0x200000e7 = 1);
  NONFAILING(*(uint8_t*)0x200000e8 = 0x81);
  NONFAILING(*(uint8_t*)0x200000e9 = 8);
  NONFAILING(*(uint16_t*)0x200000ea = 2);
  NONFAILING(*(uint8_t*)0x200000ec = 9);
  NONFAILING(*(uint8_t*)0x200000ed = 5);
  NONFAILING(*(uint8_t*)0x200000ee = 0xe);
  NONFAILING(*(uint8_t*)0x200000ef = 3);
  NONFAILING(*(uint16_t*)0x200000f0 = 8);
  NONFAILING(*(uint8_t*)0x200000f2 = 0xf9);
  NONFAILING(*(uint8_t*)0x200000f3 = 0x55);
  NONFAILING(*(uint8_t*)0x200000f4 = 0xc4);
  NONFAILING(*(uint8_t*)0x200000f5 = 2);
  NONFAILING(*(uint8_t*)0x200000f6 = 7);
  NONFAILING(*(uint8_t*)0x200000f7 = 2);
  NONFAILING(*(uint8_t*)0x200000f8 = 0x31);
  NONFAILING(*(uint32_t*)0x20000400 = 0xa);
  NONFAILING(*(uint64_t*)0x20000404 = 0x20000340);
  NONFAILING(*(uint8_t*)0x20000340 = 0xa);
  NONFAILING(*(uint8_t*)0x20000341 = 6);
  NONFAILING(*(uint16_t*)0x20000342 = 0x250);
  NONFAILING(*(uint8_t*)0x20000344 = 0x20);
  NONFAILING(*(uint8_t*)0x20000345 = 2);
  NONFAILING(*(uint8_t*)0x20000346 = 0x4f);
  NONFAILING(*(uint8_t*)0x20000347 = 8);
  NONFAILING(*(uint8_t*)0x20000348 = 0x8c);
  NONFAILING(*(uint8_t*)0x20000349 = 0);
  NONFAILING(*(uint32_t*)0x2000040c = 0);
  NONFAILING(*(uint64_t*)0x20000410 = 0);
  NONFAILING(*(uint32_t*)0x20000418 = 1);
  NONFAILING(*(uint32_t*)0x2000041c = 0);
  NONFAILING(*(uint64_t*)0x20000420 = 0);
  syz_usb_connect(2, 0xf9, 0x20000000, 0x20000400);
  close_fds();
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  setup_binfmt_misc();
  install_segv_handler();
  do_sandbox_none();
  return 0;
}
