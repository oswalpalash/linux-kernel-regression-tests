// https://syzkaller.appspot.com/bug?id=fa9be2f72b2aee60b3acbbcffecc698d8b160b10
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
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
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/usb/ch9.h>

static unsigned long long procid;

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

#define MAX_FDS 30

#define USB_MAX_IFACE_NUM 4
#define USB_MAX_EP_NUM 32
#define USB_MAX_FDS 6

struct usb_endpoint_index {
  struct usb_endpoint_descriptor desc;
  int handle;
};

struct usb_iface_index {
  struct usb_interface_descriptor* iface;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bInterfaceClass;
  struct usb_endpoint_index eps[USB_MAX_EP_NUM];
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

struct usb_info {
  int fd;
  struct usb_device_index index;
};

static struct usb_info usb_devices[USB_MAX_FDS];
static int usb_devices_num;

static bool parse_usb_descriptor(const char* buffer, size_t length,
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
        memcpy(&iface->eps[iface->eps_num].desc, buffer + offset,
               sizeof(iface->eps[iface->eps_num].desc));
        iface->eps_num++;
      }
    }
    offset += desc_length;
  }
  return true;
}

static struct usb_device_index* add_usb_index(int fd, const char* dev,
                                              size_t dev_len)
{
  int i = __atomic_fetch_add(&usb_devices_num, 1, __ATOMIC_RELAXED);
  if (i >= USB_MAX_FDS)
    return NULL;
  if (!parse_usb_descriptor(dev, dev_len, &usb_devices[i].index))
    return NULL;
  __atomic_store_n(&usb_devices[i].fd, fd, __ATOMIC_RELEASE);
  return &usb_devices[i].index;
}

static struct usb_device_index* lookup_usb_index(int fd)
{
  for (int i = 0; i < USB_MAX_FDS; i++) {
    if (__atomic_load_n(&usb_devices[i].fd, __ATOMIC_ACQUIRE) == fd)
      return &usb_devices[i].index;
  }
  return NULL;
}

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

static bool
lookup_connect_response_in(int fd, const struct vusb_connect_descriptors* descs,
                           const struct usb_ctrlrequest* ctrl,
                           struct usb_qualifier_descriptor* qual,
                           char** response_data, uint32_t* response_length)
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
          qual->bLength = sizeof(*qual);
          qual->bDescriptorType = USB_DT_DEVICE_QUALIFIER;
          qual->bcdUSB = index->dev->bcdUSB;
          qual->bDeviceClass = index->dev->bDeviceClass;
          qual->bDeviceSubClass = index->dev->bDeviceSubClass;
          qual->bDeviceProtocol = index->dev->bDeviceProtocol;
          qual->bMaxPacketSize0 = index->dev->bMaxPacketSize0;
          qual->bNumConfigurations = index->dev->bNumConfigurations;
          qual->bRESERVED = 0;
          *response_data = (char*)qual;
          *response_length = sizeof(*qual);
          return true;
        }
        *response_data = descs->qual;
        *response_length = descs->qual_len;
        return true;
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  return false;
}

typedef bool (*lookup_connect_out_response_t)(
    int fd, const struct vusb_connect_descriptors* descs,
    const struct usb_ctrlrequest* ctrl, bool* done);

static bool lookup_connect_response_out_generic(
    int fd, const struct vusb_connect_descriptors* descs,
    const struct usb_ctrlrequest* ctrl, bool* done)
{
  switch (ctrl->bRequestType & USB_TYPE_MASK) {
  case USB_TYPE_STANDARD:
    switch (ctrl->bRequest) {
    case USB_REQ_SET_CONFIGURATION:
      *done = true;
      return true;
    default:
      break;
    }
    break;
  }
  return false;
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

#define USB_RAW_EPS_NUM_MAX 30
#define USB_RAW_EP_NAME_MAX 16
#define USB_RAW_EP_ADDR_ANY 0xff

struct usb_raw_ep_caps {
  __u32 type_control : 1;
  __u32 type_iso : 1;
  __u32 type_bulk : 1;
  __u32 type_int : 1;
  __u32 dir_in : 1;
  __u32 dir_out : 1;
};

struct usb_raw_ep_limits {
  __u16 maxpacket_limit;
  __u16 max_streams;
  __u32 reserved;
};

struct usb_raw_ep_info {
  __u8 name[USB_RAW_EP_NAME_MAX];
  __u32 addr;
  struct usb_raw_ep_caps caps;
  struct usb_raw_ep_limits limits;
};

struct usb_raw_eps_info {
  struct usb_raw_ep_info eps[USB_RAW_EPS_NUM_MAX];
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
#define USB_RAW_IOCTL_EPS_INFO _IOR('U', 11, struct usb_raw_eps_info)
#define USB_RAW_IOCTL_EP0_STALL _IO('U', 12)
#define USB_RAW_IOCTL_EP_SET_HALT _IOW('U', 13, __u32)
#define USB_RAW_IOCTL_EP_CLEAR_HALT _IOW('U', 14, __u32)
#define USB_RAW_IOCTL_EP_SET_WEDGE _IOW('U', 15, __u32)

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

static int usb_raw_ep0_stall(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_STALL, 0);
}

static void set_interface(int fd, int n)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  if (!index)
    return;
  if (index->iface_cur >= 0 && index->iface_cur < index->ifaces_num) {
    for (int ep = 0; ep < index->ifaces[index->iface_cur].eps_num; ep++) {
      int rv = usb_raw_ep_disable(
          fd, index->ifaces[index->iface_cur].eps[ep].handle);
      if (rv < 0) {
      } else {
      }
    }
  }
  if (n >= 0 && n < index->ifaces_num) {
    for (int ep = 0; ep < index->ifaces[n].eps_num; ep++) {
      int rv = usb_raw_ep_enable(fd, &index->ifaces[n].eps[ep].desc);
      if (rv < 0) {
      } else {
        index->ifaces[n].eps[ep].handle = rv;
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

#define USB_MAX_PACKET_SIZE 4096

struct usb_raw_control_event {
  struct usb_raw_event inner;
  struct usb_ctrlrequest ctrl;
  char data[USB_MAX_PACKET_SIZE];
};

struct usb_raw_ep_io_data {
  struct usb_raw_ep_io inner;
  char data[USB_MAX_PACKET_SIZE];
};

static volatile long
syz_usb_connect_impl(uint64_t speed, uint64_t dev_len, const char* dev,
                     const struct vusb_connect_descriptors* descs,
                     lookup_connect_out_response_t lookup_connect_response_out)
{
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
    char* response_data = NULL;
    uint32_t response_length = 0;
    struct usb_qualifier_descriptor qual;
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      if (!lookup_connect_response_in(fd, descs, &event.ctrl, &qual,
                                      &response_data, &response_length)) {
        usb_raw_ep0_stall(fd);
        continue;
      }
    } else {
      if (!lookup_connect_response_out(fd, descs, &event.ctrl, &done)) {
        usb_raw_ep0_stall(fd);
        continue;
      }
      response_data = NULL;
      response_length = event.ctrl.wLength;
    }
    if ((event.ctrl.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD &&
        event.ctrl.bRequest == USB_REQ_SET_CONFIGURATION) {
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

static volatile long syz_usb_connect(volatile long a0, volatile long a1,
                                     volatile long a2, volatile long a3)
{
  uint64_t speed = a0;
  uint64_t dev_len = a1;
  const char* dev = (const char*)a2;
  const struct vusb_connect_descriptors* descs =
      (const struct vusb_connect_descriptors*)a3;
  return syz_usb_connect_impl(speed, dev_len, dev, descs,
                              &lookup_connect_response_out_generic);
}

static void kill_and_wait(int pid, int* status)
{
  kill(-pid, SIGKILL);
  kill(pid, SIGKILL);
  for (int i = 0; i < 100; i++) {
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
  int iter = 0;
  for (;; iter++) {
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
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
  }
}

void execute_one(void)
{
  *(uint8_t*)0x20000e40 = 0x12;
  *(uint8_t*)0x20000e41 = 1;
  *(uint16_t*)0x20000e42 = 0x200;
  *(uint8_t*)0x20000e44 = 0xc7;
  *(uint8_t*)0x20000e45 = 0xbe;
  *(uint8_t*)0x20000e46 = 0x73;
  *(uint8_t*)0x20000e47 = 0x40;
  *(uint16_t*)0x20000e48 = 0x182d;
  *(uint16_t*)0x20000e4a = 0x21c;
  *(uint16_t*)0x20000e4c = 0xbd3f;
  *(uint8_t*)0x20000e4e = 1;
  *(uint8_t*)0x20000e4f = 2;
  *(uint8_t*)0x20000e50 = 3;
  *(uint8_t*)0x20000e51 = 1;
  *(uint8_t*)0x20000e52 = 9;
  *(uint8_t*)0x20000e53 = 2;
  *(uint16_t*)0x20000e54 = 0xb9;
  *(uint8_t*)0x20000e56 = 1;
  *(uint8_t*)0x20000e57 = 0;
  *(uint8_t*)0x20000e58 = 1;
  *(uint8_t*)0x20000e59 = 0x30;
  *(uint8_t*)0x20000e5a = 7;
  *(uint8_t*)0x20000e5b = 9;
  *(uint8_t*)0x20000e5c = 4;
  *(uint8_t*)0x20000e5d = 0x78;
  *(uint8_t*)0x20000e5e = 6;
  *(uint8_t*)0x20000e5f = 8;
  *(uint8_t*)0x20000e60 = 0xc6;
  *(uint8_t*)0x20000e61 = 0x5d;
  *(uint8_t*)0x20000e62 = 0xb6;
  *(uint8_t*)0x20000e63 = 0x2a;
  *(uint8_t*)0x20000e64 = 7;
  *(uint8_t*)0x20000e65 = 0x24;
  *(uint8_t*)0x20000e66 = 1;
  *(uint8_t*)0x20000e67 = 0xf3;
  *(uint8_t*)0x20000e68 = 1;
  *(uint16_t*)0x20000e69 = 1;
  *(uint8_t*)0x20000e6b = 8;
  *(uint8_t*)0x20000e6c = 0x24;
  *(uint8_t*)0x20000e6d = 2;
  *(uint8_t*)0x20000e6e = 1;
  *(uint8_t*)0x20000e6f = 6;
  *(uint8_t*)0x20000e70 = 4;
  *(uint8_t*)0x20000e71 = 0x56;
  *(uint8_t*)0x20000e72 = 2;
  *(uint8_t*)0x20000e73 = 7;
  *(uint8_t*)0x20000e74 = 0x24;
  *(uint8_t*)0x20000e75 = 1;
  *(uint8_t*)0x20000e76 = -1;
  *(uint8_t*)0x20000e77 = 0xd1;
  *(uint16_t*)0x20000e78 = 3;
  *(uint8_t*)0x20000e7a = 9;
  *(uint8_t*)0x20000e7b = 0x24;
  *(uint8_t*)0x20000e7c = 2;
  *(uint8_t*)0x20000e7d = 2;
  *(uint16_t*)0x20000e7e = 0x3ff;
  *(uint16_t*)0x20000e80 = 0x8001;
  *(uint8_t*)0x20000e82 = 0;
  *(uint8_t*)0x20000e83 = 9;
  *(uint8_t*)0x20000e84 = 5;
  *(uint8_t*)0x20000e85 = 0xc;
  *(uint8_t*)0x20000e86 = 0x10;
  *(uint16_t*)0x20000e87 = 0x40;
  *(uint8_t*)0x20000e89 = 0x1b;
  *(uint8_t*)0x20000e8a = 3;
  *(uint8_t*)0x20000e8b = 7;
  *(uint8_t*)0x20000e8c = 2;
  *(uint8_t*)0x20000e8d = 0x14;
  *(uint8_t*)0x20000e8e = 7;
  *(uint8_t*)0x20000e8f = 0x25;
  *(uint8_t*)0x20000e90 = 1;
  *(uint8_t*)0x20000e91 = 1;
  *(uint8_t*)0x20000e92 = 1;
  *(uint16_t*)0x20000e93 = 0;
  *(uint8_t*)0x20000e95 = 9;
  *(uint8_t*)0x20000e96 = 5;
  *(uint8_t*)0x20000e97 = 5;
  *(uint8_t*)0x20000e98 = 0x10;
  *(uint16_t*)0x20000e99 = 8;
  *(uint8_t*)0x20000e9b = 9;
  *(uint8_t*)0x20000e9c = 0x40;
  *(uint8_t*)0x20000e9d = 4;
  *(uint8_t*)0x20000e9e = 7;
  *(uint8_t*)0x20000e9f = 0x25;
  *(uint8_t*)0x20000ea0 = 1;
  *(uint8_t*)0x20000ea1 = 0;
  *(uint8_t*)0x20000ea2 = 0x81;
  *(uint16_t*)0x20000ea3 = 0xfff9;
  *(uint8_t*)0x20000ea5 = 7;
  *(uint8_t*)0x20000ea6 = 0x25;
  *(uint8_t*)0x20000ea7 = 1;
  *(uint8_t*)0x20000ea8 = 0x80;
  *(uint8_t*)0x20000ea9 = 2;
  *(uint16_t*)0x20000eaa = 6;
  *(uint8_t*)0x20000eac = 9;
  *(uint8_t*)0x20000ead = 5;
  *(uint8_t*)0x20000eae = 6;
  *(uint8_t*)0x20000eaf = 0;
  *(uint16_t*)0x20000eb0 = 0x10;
  *(uint8_t*)0x20000eb2 = 0x7f;
  *(uint8_t*)0x20000eb3 = 9;
  *(uint8_t*)0x20000eb4 = 0x4c;
  *(uint8_t*)0x20000eb5 = 7;
  *(uint8_t*)0x20000eb6 = 0x25;
  *(uint8_t*)0x20000eb7 = 1;
  *(uint8_t*)0x20000eb8 = 0x81;
  *(uint8_t*)0x20000eb9 = 5;
  *(uint16_t*)0x20000eba = 5;
  *(uint8_t*)0x20000ebc = 2;
  *(uint8_t*)0x20000ebd = 0x21;
  *(uint8_t*)0x20000ebe = 9;
  *(uint8_t*)0x20000ebf = 5;
  *(uint8_t*)0x20000ec0 = 0x80;
  *(uint8_t*)0x20000ec1 = 1;
  *(uint16_t*)0x20000ec2 = 0x200;
  *(uint8_t*)0x20000ec4 = 7;
  *(uint8_t*)0x20000ec5 = 0xf9;
  *(uint8_t*)0x20000ec6 = 0x5e;
  *(uint8_t*)0x20000ec7 = 2;
  *(uint8_t*)0x20000ec8 = 1;
  *(uint8_t*)0x20000ec9 = 7;
  *(uint8_t*)0x20000eca = 0x25;
  *(uint8_t*)0x20000ecb = 1;
  *(uint8_t*)0x20000ecc = 0x81;
  *(uint8_t*)0x20000ecd = 7;
  *(uint16_t*)0x20000ece = 0xf2;
  *(uint8_t*)0x20000ed0 = 9;
  *(uint8_t*)0x20000ed1 = 5;
  *(uint8_t*)0x20000ed2 = 0xf;
  *(uint8_t*)0x20000ed3 = 3;
  *(uint16_t*)0x20000ed4 = 0x40;
  *(uint8_t*)0x20000ed6 = 1;
  *(uint8_t*)0x20000ed7 = 2;
  *(uint8_t*)0x20000ed8 = 2;
  *(uint8_t*)0x20000ed9 = 7;
  *(uint8_t*)0x20000eda = 0x25;
  *(uint8_t*)0x20000edb = 1;
  *(uint8_t*)0x20000edc = 0xc0;
  *(uint8_t*)0x20000edd = 0;
  *(uint16_t*)0x20000ede = 2;
  *(uint8_t*)0x20000ee0 = 9;
  *(uint8_t*)0x20000ee1 = 5;
  *(uint8_t*)0x20000ee2 = 0xd;
  *(uint8_t*)0x20000ee3 = 0xf;
  *(uint16_t*)0x20000ee4 = 0x20;
  *(uint8_t*)0x20000ee6 = 5;
  *(uint8_t*)0x20000ee7 = 8;
  *(uint8_t*)0x20000ee8 = 6;
  *(uint8_t*)0x20000ee9 = 7;
  *(uint8_t*)0x20000eea = 0x25;
  *(uint8_t*)0x20000eeb = 1;
  *(uint8_t*)0x20000eec = 0x80;
  *(uint8_t*)0x20000eed = 6;
  *(uint16_t*)0x20000eee = 5;
  *(uint8_t*)0x20000ef0 = 9;
  *(uint8_t*)0x20000ef1 = 5;
  *(uint8_t*)0x20000ef2 = 0xf;
  *(uint8_t*)0x20000ef3 = 0x10;
  *(uint16_t*)0x20000ef4 = 0x200;
  *(uint8_t*)0x20000ef6 = 0x81;
  *(uint8_t*)0x20000ef7 = 8;
  *(uint8_t*)0x20000ef8 = 0;
  *(uint8_t*)0x20000ef9 = 2;
  *(uint8_t*)0x20000efa = 0x22;
  *(uint8_t*)0x20000efb = 9;
  *(uint8_t*)0x20000efc = 5;
  *(uint8_t*)0x20000efd = 0xd;
  *(uint8_t*)0x20000efe = 0x10;
  *(uint16_t*)0x20000eff = 0x10;
  *(uint8_t*)0x20000f01 = 3;
  *(uint8_t*)0x20000f02 = -1;
  *(uint8_t*)0x20000f03 = 0xab;
  *(uint8_t*)0x20000f04 = 7;
  *(uint8_t*)0x20000f05 = 0x25;
  *(uint8_t*)0x20000f06 = 1;
  *(uint8_t*)0x20000f07 = 0x80;
  *(uint8_t*)0x20000f08 = 0x47;
  *(uint16_t*)0x20000f09 = 9;
  syz_usb_connect(5, 0xcb, 0x20000e40, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
