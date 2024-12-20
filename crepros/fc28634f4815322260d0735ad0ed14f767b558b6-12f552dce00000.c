// https://syzkaller.appspot.com/bug?id=fc28634f4815322260d0735ad0ed14f767b558b6
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
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
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

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

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
}

static uint64_t current_time_ms(void)
{
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts))
    exit(1);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
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

static struct {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[1024];
} nlmsg;

static void netlink_init(int typ, int flags, const void* data, int size)
{
  memset(&nlmsg, 0, sizeof(nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg.buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg.pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(int typ, const void* data, int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg.pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  memcpy(attr + 1, data, size);
  nlmsg.pos += NLMSG_ALIGN(attr->nla_len);
}

static int netlink_send_ext(int sock, uint16_t reply_type, int* reply_len)
{
  if (nlmsg.pos > nlmsg.buf + sizeof(nlmsg.buf) || nlmsg.nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg.buf;
  hdr->nlmsg_len = nlmsg.pos - nlmsg.buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  unsigned n = sendto(sock, nlmsg.buf, hdr->nlmsg_len, 0,
                      (struct sockaddr*)&addr, sizeof(addr));
  if (n != hdr->nlmsg_len)
    exit(1);
  n = recv(sock, nlmsg.buf, sizeof(nlmsg.buf), 0);
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

static int netlink_send(int sock)
{
  return netlink_send_ext(sock, 0, NULL);
}

const int kInitNetNsFd = 239;

#define DEVLINK_FAMILY_NAME "devlink"

#define DEVLINK_CMD_RELOAD 37
#define DEVLINK_ATTR_BUS_NAME 1
#define DEVLINK_ATTR_DEV_NAME 2
#define DEVLINK_ATTR_NETNS_FD 137

static void netlink_devlink_netns_move(const char* bus_name,
                                       const char* dev_name, int netns_fd)
{
  struct genlmsghdr genlhdr;
  struct nlattr* attr;
  int sock, err, n;
  uint16_t id = 0;
  sock = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (sock == -1)
    exit(1);
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(CTRL_ATTR_FAMILY_NAME, DEVLINK_FAMILY_NAME,
               strlen(DEVLINK_FAMILY_NAME) + 1);
  err = netlink_send_ext(sock, GENL_ID_CTRL, &n);
  if (err) {
    goto error;
  }
  attr =
      (struct nlattr*)(nlmsg.buf + NLMSG_HDRLEN + NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg.buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    goto error;
  }
  recv(sock, nlmsg.buf, sizeof(nlmsg.buf), 0); /* recv ack */
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = DEVLINK_CMD_RELOAD;
  netlink_init(id, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(DEVLINK_ATTR_BUS_NAME, bus_name, strlen(bus_name) + 1);
  netlink_attr(DEVLINK_ATTR_DEV_NAME, dev_name, strlen(dev_name) + 1);
  netlink_attr(DEVLINK_ATTR_NETNS_FD, &netns_fd, sizeof(netns_fd));
  netlink_send(sock);
error:
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
}

#define MAX_FDS 30

#define USB_DEBUG 0

#define USB_MAX_IFACE_NUM 4
#define USB_MAX_EP_NUM 32

struct usb_iface_index {
  struct usb_interface_descriptor* iface;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  struct usb_endpoint_descriptor eps[USB_MAX_EP_NUM];
  int eps_num;
};

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
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

enum usb_raw_event_type {
  USB_RAW_EVENT_INVALID,
  USB_RAW_EVENT_CONNECT,
  USB_RAW_EVENT_CONTROL,
};

struct usb_raw_event {
  uint32_t type;
  uint32_t length;
  char data[0];
};

struct usb_raw_init {
  uint64_t speed;
  const char* driver_name;
  const char* device_name;
};

struct usb_raw_ep_io {
  uint16_t ep;
  uint16_t flags;
  uint32_t length;
  char data[0];
};

#define USB_RAW_IOCTL_INIT _IOW('U', 0, struct usb_raw_init)
#define USB_RAW_IOCTL_RUN _IO('U', 1)
#define USB_RAW_IOCTL_EVENT_FETCH _IOR('U', 2, struct usb_raw_event)
#define USB_RAW_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP0_READ _IOWR('U', 4, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_ENABLE _IOW('U', 5, struct usb_endpoint_descriptor)
#define USB_RAW_IOCTL_EP_DISABLE _IOW('U', 6, int)
#define USB_RAW_IOCTL_EP_WRITE _IOW('U', 7, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_READ _IOWR('U', 8, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_CONFIGURE _IO('U', 9)
#define USB_RAW_IOCTL_VBUS_DRAW _IOW('U', 10, uint32_t)

static int usb_raw_open()
{
  return open("/sys/kernel/debug/usb/raw-gadget", O_RDWR);
}

static int usb_raw_init(int fd, uint32_t speed, const char* driver,
                        const char* device)
{
  struct usb_raw_init arg;
  arg.speed = speed;
  arg.driver_name = driver;
  arg.device_name = device;
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
  rv = parse_usb_descriptor(dev, dev_len, &usb_devices[i].index);
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
      response_found = lookup_connect_response(
          fd, descs, &event.ctrl, &response_data, &response_length);
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

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  int i;
  for (i = 0; i < 100; i++) {
    if (waitpid(-1, status, WNOHANG | __WALL) == pid)
      return;
    usleep(1000);
  }
  DIR* dir = opendir("/sys/fs/fuse/connections");
  if (dir) {
    for (;;) {
      struct dirent* ent = readdir(dir);
      if (!ent)
        break;
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
        continue;
      char abort[300];
      snprintf(abort, sizeof(abort), "/sys/fs/fuse/connections/%s/abort",
               ent->d_name);
      int fd = open(abort, O_WRONLY);
      if (fd == -1) {
        continue;
      }
      if (write(fd, abort, 1) < 0) {
      }
      close(fd);
    }
    closedir(dir);
  } else {
  }
  while (waitpid(-1, status, __WALL) != pid) {
  }
}

static void setup_test()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  write_file("/proc/self/oom_score_adj", "1000");
}

static void execute_one(void);

#define WAIT_FLAGS __WALL

static void loop(void)
{
  int iter;
  for (iter = 0;; iter++) {
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      setup_test();
      execute_one();
      exit(0);
    }
    int status = 0;
    uint64_t start = current_time_ms();
    for (;;) {
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      sleep_ms(1);
      if (current_time_ms() - start < 5 * 1000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

void execute_one(void)
{
  memcpy(
      (void*)0x200000c0,
      "\x12\x01\x00\x00\xe3\x81\xec\x08\xaa\x07\x51\x00\xb9\x3e\x83\x00\x00\x01"
      "\x09\x02\x82\x03\x01\x00\x00\x00\x00\x09\x04\x59\x00\x09\x3e\xdc\xaa\x00"
      "\x09\x05\x0c\x10\x87\x02\xfb\x01\x04\x09\x05\x00\x00\x84\x03\x06\x00\x05"
      "\x07\x25\x01\x02\x01\xe4\x6c\xea\x21\xa3\xa6\x78\xd5\x91\xe1\xc7\x9c\x51"
      "\x2f\xdf\x92\x9b\x71\x24\x73\xf0\x0a\x82\x08\x5c\x04\xde\x84\x51\x90\x25"
      "\x3d\xcb\xd8\xc2\x5a\x3f\x41\xf0\x14\xe4\x32\x12\x1f\x3e\x9f\x7f\x47\x8b"
      "\xbf\xc0\xd6\xaf\x63\xaf\xfd\x5b\x4d\x67\x5b\x1f\x64\xe3\xac\x43\x15\x9e"
      "\x62\x1b\x32\xaf\x9f\xbd\xfa\x79\x65\x08\x3b\x27\x4e\xdc\x22\xeb\x94\x0d"
      "\x34\x13\x1b\xe8\xa4\xc8\x95\x69\x8b\x7c\x68\xf4\xdd\xa5\xdf\x77\x86\x01"
      "\x4f\xd2\xe6\xbe\x77\xaf\xfb\x9d\x19\x7c\x05\xbf\x83\x87\x27\xc3\x05\x2c"
      "\x59\x02\x72\xe9\xf2\x33\x39\x6e\xd6\x8f\x07\x63\x8f\x65\xad\x06\xdf\xae"
      "\xc9\x23\xaf\x4f\xf6\xc3\x6c\x25\xf2\x48\x7a\xd0\xc3\x78\x52\xe2\xa3\x34"
      "\x85\x4e\x65\x39\x44\x7c\x94\xd3\x0e\x4b\x4b\x56\x33\x63\x17\x8a\x03\x86"
      "\xfc\x86\x29\xf7\xe4\x3d\xd0\x6a\x37\xb8\x2d\xca\x55\xd9\x2d\x4d\x2d\xdc"
      "\x4e\x97\x31\x91\x3d\x27\x22\xd7\x09\xf4\x9d\xbc\xe2\x3b\x8e\x7a\x9b\x44"
      "\x2c\xc2\x83\x22\xd4\x89\x7e\x4c\xbd\x47\x85\x5d\x75\xe5\x26\x5c\xbe\xfb"
      "\xe4\xb5\x02\x56\xf4\x3c\xf2\x09\x05\x80\x10\x36\x01\x81\x05\xf9\x45\x00"
      "\x6d\x03\xe3\x55\x4c\x8b\x11\x3a\x04\x75\x96\xa8\x89\x2c\x78\xfb\x01\x51"
      "\xf9\x8d\x65\x9b\x63\xd3\x3f\x23\x1b\x12\x98\xc9\x0f\x73\x0b\x4e\x28\x6a"
      "\x5f\xc6\x72\x24\xd8\x23\xe9\x7c\x2b\xbe\x31\x96\x94\xfc\x41\xe3\x24\x79"
      "\xbb\x85\x88\x66\x6b\x6f\xed\x1a\x18\x41\x2c\x8b\x2c\xef\x07\x5e\x6d\xc6"
      "\xbe\xd9\x40\xe9\x52\xf6\x16\x9a\x2a\x3d\x88\x7d\xfd\x44\x66\x35\x55\xec"
      "\xac\x95\x87\xa4\x5d\xaa\x65\xb4\xb8\x85\x64\xb7\xca\x32\x7a\xa6\x8f\x30"
      "\x7d\xb1\xf3\x23\x1e\x3b\x4e\x09\x94\x12\x5f\x6f\x94\x4c\x43\x16\x6d\x2f"
      "\xa6\x90\xe2\xdc\x03\xe5\xf0\x1e\x56\x2f\x06\x3f\x3a\x19\x6b\x56\xb5\x1b"
      "\x35\xe8\x25\xc0\xfc\x24\xb8\xf5\xea\x2a\xc1\x19\xd0\xc6\xd1\x92\x53\xd6"
      "\x98\x16\x79\x99\x54\xa4\x9e\x08\x52\xe3\xc8\xed\x58\xa6\x9b\x42\x97\x2a"
      "\xca\x6c\x03\x1e\x56\x3f\x3e\xab\x03\x9f\xca\x5a\xc1\x22\x6e\xb4\x54\x0a"
      "\xb3\x9b\x21\x9d\x57\x32\x9e\x1c\x28\x23\x95\xcb\x48\x9a\x6e\x35\x0d\xb6"
      "\x53\x37\xcb\x90\xc3\xb8\xa4\x6d\x68\x9a\x6b\x32\x41\x1e\x67\xaa\x5c\x68"
      "\x6a\xeb\x74\x94\xca\xaa\xf6\x6c\x47\x73\xe5\x51\x35\xfc\x84\x6a\x74\x6b"
      "\x1a\xa6\x70\x40\xfb\x4c\xc0\x83\x15\xa3\x9e\x5d\x41\xde\x00\x18\x76\x57"
      "\x52\x85\xcd\x9e\x5c\x9c\x78\x66\x2c\x41\x86\x7f\x3f\x50\xe8\xfb\x7c\x42"
      "\xb0\x54\x25\xd1\x4d\xfb\xde\x0a\x5f\x6d\x21\x38\x37\xa7\x03\x0b\x09\x05"
      "\x04\x08\xf9\x01\x0a\x00\x40\x09\x05\xe1\x08\xba\x03\x05\x42\x04\xec\x0b"
      "\xa5\x31\x9f\xdf\x46\x4e\xdd\x56\x69\xee\x2c\x46\xa4\x0c\x28\x2a\xcf\x26"
      "\x9f\x69\xd0\x04\xd9\xb1\xba\x29\x87\xa0\x20\xea\xc7\xd0\xa1\x90\x56\xc8"
      "\x20\x65\x3a\x95\x0f\xc5\x75\x37\x6e\x21\xa4\xc7\xd5\x6b\xf0\xb0\x5d\xd3"
      "\xf5\x44\x0a\x13\xbc\xa0\x3d\x8d\x2d\xcf\x45\x0e\x93\x04\x77\x61\xa1\xd0"
      "\x37\xd8\x04\x5a\xa9\x5e\x8d\xc3\x49\x7b\x19\xed\xec\x9b\xe8\xae\xeb\xcf"
      "\xf3\x84\x72\xef\x97\xb4\x60\x64\xf1\xf3\x0e\x0e\xd8\xd1\x7f\x25\x6f\xb9"
      "\x3d\xa4\x91\xa2\x40\xa5\x3b\x55\xab\x4c\xf7\x74\x4b\x45\x75\xc1\x12\x2d"
      "\x83\x93\xb5\xfb\xb1\x92\x3c\x7e\x7a\x34\x99\x08\xa3\xbf\xa4\x1e\x39\xa2"
      "\x6e\xa0\x84\x3a\x4b\xe1\xd0\xd2\x8b\xe6\xe1\xf9\xee\x4c\x52\x9e\x44\x7a"
      "\x24\x06\xeb\x51\x70\x7f\xa2\x58\x7a\xab\xf1\xd1\x95\x7c\xd1\x53\x30\x49"
      "\xd4\xda\x8e\x11\xab\xc9\x8c\x64\x5e\x5a\x8f\xac\x0f\x61\x07\x84\x29\x41"
      "\x82\x60\xfa\xfe\x0a\x4c\xac\x82\xad\x9f\xe4\x09\x00\x7e\x30\xfc\xb9\x91"
      "\x69\x26\x5c\xa5\x6a\x78\x5c\x31\x4e\xac\x67\xc2\x15\x83\xca\x6f\x3f\x27"
      "\x09\x05\x09\x02\x36\x02\x40\xd6\x6f\x07\x25\x01\x01\x00\x07\x00\x09\x05"
      "\x00\x10\x7f\x02\x00\x04\xef\x07\x25\x01\x80\x1a\x07\x00\x09\x05\x80\x00"
      "\xc1\x00\x31\x85\x1f\x09\x05\x0b\x00\xd6\x02\x09\x09\x06\x00\x00",
      916);
  syz_usb_connect(7, 0x394, 0x200000c0, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
