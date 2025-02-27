// https://syzkaller.appspot.com/bug?id=2f4d19de8c9e9f0b9794e53ca54d68e0ffe9f068
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
  *(uint8_t*)0x20000880 = 0x12;
  *(uint8_t*)0x20000881 = 1;
  *(uint16_t*)0x20000882 = 0;
  *(uint8_t*)0x20000884 = 0x4c;
  *(uint8_t*)0x20000885 = 0x27;
  *(uint8_t*)0x20000886 = 0xda;
  *(uint8_t*)0x20000887 = 0x10;
  *(uint16_t*)0x20000888 = 0x5032;
  *(uint16_t*)0x2000088a = 0xfa1;
  *(uint16_t*)0x2000088c = 0x71b4;
  *(uint8_t*)0x2000088e = 1;
  *(uint8_t*)0x2000088f = 2;
  *(uint8_t*)0x20000890 = 3;
  *(uint8_t*)0x20000891 = 1;
  *(uint8_t*)0x20000892 = 9;
  *(uint8_t*)0x20000893 = 2;
  *(uint16_t*)0x20000894 = 0xf2;
  *(uint8_t*)0x20000896 = 2;
  *(uint8_t*)0x20000897 = 0;
  *(uint8_t*)0x20000898 = 0;
  *(uint8_t*)0x20000899 = 0;
  *(uint8_t*)0x2000089a = 0;
  *(uint8_t*)0x2000089b = 9;
  *(uint8_t*)0x2000089c = 4;
  *(uint8_t*)0x2000089d = 0x23;
  *(uint8_t*)0x2000089e = 0;
  *(uint8_t*)0x2000089f = 0xa;
  *(uint8_t*)0x200008a0 = 0xfc;
  *(uint8_t*)0x200008a1 = 0xf0;
  *(uint8_t*)0x200008a2 = 0xac;
  *(uint8_t*)0x200008a3 = 0;
  *(uint8_t*)0x200008a4 = 5;
  *(uint8_t*)0x200008a5 = 0x24;
  *(uint8_t*)0x200008a6 = 6;
  *(uint8_t*)0x200008a7 = 0;
  *(uint8_t*)0x200008a8 = 0;
  *(uint8_t*)0x200008a9 = 5;
  *(uint8_t*)0x200008aa = 0x24;
  *(uint8_t*)0x200008ab = 0;
  *(uint16_t*)0x200008ac = 8;
  *(uint8_t*)0x200008ae = 0xd;
  *(uint8_t*)0x200008af = 0x24;
  *(uint8_t*)0x200008b0 = 0xf;
  *(uint8_t*)0x200008b1 = 1;
  *(uint32_t*)0x200008b2 = 0x80000000;
  *(uint16_t*)0x200008b6 = 5;
  *(uint16_t*)0x200008b8 = 0xfe01;
  *(uint8_t*)0x200008ba = 4;
  *(uint8_t*)0x200008bb = 7;
  *(uint8_t*)0x200008bc = 0x24;
  *(uint8_t*)0x200008bd = 0x14;
  *(uint16_t*)0x200008be = 0x1f;
  *(uint16_t*)0x200008c0 = 0;
  *(uint8_t*)0x200008c2 = 5;
  *(uint8_t*)0x200008c3 = 0x24;
  *(uint8_t*)0x200008c4 = 0x15;
  *(uint16_t*)0x200008c5 = 0x100;
  *(uint8_t*)0x200008c7 = 8;
  *(uint8_t*)0x200008c8 = 0x24;
  *(uint8_t*)0x200008c9 = 0x1c;
  *(uint16_t*)0x200008ca = 3;
  *(uint8_t*)0x200008cc = 0xea;
  *(uint16_t*)0x200008cd = 0x7a;
  *(uint8_t*)0x200008cf = 7;
  *(uint8_t*)0x200008d0 = 0x24;
  *(uint8_t*)0x200008d1 = 0xa;
  *(uint8_t*)0x200008d2 = 0xbf;
  *(uint8_t*)0x200008d3 = 0;
  *(uint8_t*)0x200008d4 = 0;
  *(uint8_t*)0x200008d5 = 0;
  *(uint8_t*)0x200008d6 = 7;
  *(uint8_t*)0x200008d7 = 0x24;
  *(uint8_t*)0x200008d8 = 0xa;
  *(uint8_t*)0x200008d9 = 9;
  *(uint8_t*)0x200008da = 1;
  *(uint8_t*)0x200008db = 2;
  *(uint8_t*)0x200008dc = 0x38;
  *(uint8_t*)0x200008dd = 9;
  *(uint8_t*)0x200008de = 5;
  *(uint8_t*)0x200008df = 0x80;
  *(uint8_t*)0x200008e0 = 0x10;
  *(uint16_t*)0x200008e1 = 0x40;
  *(uint8_t*)0x200008e3 = 0x40;
  *(uint8_t*)0x200008e4 = 3;
  *(uint8_t*)0x200008e5 = 3;
  *(uint8_t*)0x200008e6 = 7;
  *(uint8_t*)0x200008e7 = 0x25;
  *(uint8_t*)0x200008e8 = 1;
  *(uint8_t*)0x200008e9 = 0x80;
  *(uint8_t*)0x200008ea = 0x7f;
  *(uint16_t*)0x200008eb = 8;
  *(uint8_t*)0x200008ed = 9;
  *(uint8_t*)0x200008ee = 5;
  *(uint8_t*)0x200008ef = 9;
  *(uint8_t*)0x200008f0 = 3;
  *(uint16_t*)0x200008f1 = 0x40;
  *(uint8_t*)0x200008f3 = 2;
  *(uint8_t*)0x200008f4 = 8;
  *(uint8_t*)0x200008f5 = 0xe7;
  *(uint8_t*)0x200008f6 = 7;
  *(uint8_t*)0x200008f7 = 0x25;
  *(uint8_t*)0x200008f8 = 1;
  *(uint8_t*)0x200008f9 = 0;
  *(uint8_t*)0x200008fa = 8;
  *(uint16_t*)0x200008fb = 0xaeb3;
  *(uint8_t*)0x200008fd = 2;
  *(uint8_t*)0x200008fe = 0x24;
  *(uint8_t*)0x200008ff = 9;
  *(uint8_t*)0x20000900 = 5;
  *(uint8_t*)0x20000901 = 0x80;
  *(uint8_t*)0x20000902 = 0xc;
  *(uint16_t*)0x20000903 = 0x20;
  *(uint8_t*)0x20000905 = 5;
  *(uint8_t*)0x20000906 = 0x20;
  *(uint8_t*)0x20000907 = 6;
  *(uint8_t*)0x20000908 = 2;
  *(uint8_t*)0x20000909 = 0x21;
  *(uint8_t*)0x2000090a = 9;
  *(uint8_t*)0x2000090b = 5;
  *(uint8_t*)0x2000090c = 0x80;
  *(uint8_t*)0x2000090d = 0x11;
  *(uint16_t*)0x2000090e = 0x10;
  *(uint8_t*)0x20000910 = 5;
  *(uint8_t*)0x20000911 = 4;
  *(uint8_t*)0x20000912 = 1;
  *(uint8_t*)0x20000913 = 2;
  *(uint8_t*)0x20000914 = 0x22;
  *(uint8_t*)0x20000915 = 7;
  *(uint8_t*)0x20000916 = 0x25;
  *(uint8_t*)0x20000917 = 1;
  *(uint8_t*)0x20000918 = 1;
  *(uint8_t*)0x20000919 = 0x40;
  *(uint16_t*)0x2000091a = 0x80;
  *(uint8_t*)0x2000091c = 9;
  *(uint8_t*)0x2000091d = 5;
  *(uint8_t*)0x2000091e = 6;
  *(uint8_t*)0x2000091f = 0;
  *(uint16_t*)0x20000920 = 0x3f7;
  *(uint8_t*)0x20000922 = 0xc1;
  *(uint8_t*)0x20000923 = 9;
  *(uint8_t*)0x20000924 = 5;
  *(uint8_t*)0x20000925 = 9;
  *(uint8_t*)0x20000926 = 5;
  *(uint8_t*)0x20000927 = 0x80;
  *(uint8_t*)0x20000928 = 0xc;
  *(uint16_t*)0x20000929 = 0x20;
  *(uint8_t*)0x2000092b = 2;
  *(uint8_t*)0x2000092c = 0x3f;
  *(uint8_t*)0x2000092d = 1;
  *(uint8_t*)0x2000092e = 7;
  *(uint8_t*)0x2000092f = 0x25;
  *(uint8_t*)0x20000930 = 1;
  *(uint8_t*)0x20000931 = 3;
  *(uint8_t*)0x20000932 = 6;
  *(uint16_t*)0x20000933 = 1;
  *(uint8_t*)0x20000935 = 2;
  *(uint8_t*)0x20000936 = 0xc;
  *(uint8_t*)0x20000937 = 9;
  *(uint8_t*)0x20000938 = 5;
  *(uint8_t*)0x20000939 = 0xa;
  *(uint8_t*)0x2000093a = 0x10;
  *(uint16_t*)0x2000093b = 8;
  *(uint8_t*)0x2000093d = 8;
  *(uint8_t*)0x2000093e = 8;
  *(uint8_t*)0x2000093f = 0x7c;
  *(uint8_t*)0x20000940 = 9;
  *(uint8_t*)0x20000941 = 5;
  *(uint8_t*)0x20000942 = 4;
  *(uint8_t*)0x20000943 = 0xc;
  *(uint16_t*)0x20000944 = 0x200;
  *(uint8_t*)0x20000946 = 7;
  *(uint8_t*)0x20000947 = 5;
  *(uint8_t*)0x20000948 = 0x1f;
  *(uint8_t*)0x20000949 = 2;
  *(uint8_t*)0x2000094a = 0x22;
  *(uint8_t*)0x2000094b = 7;
  *(uint8_t*)0x2000094c = 0x25;
  *(uint8_t*)0x2000094d = 1;
  *(uint8_t*)0x2000094e = 1;
  *(uint8_t*)0x2000094f = 0xfc;
  *(uint16_t*)0x20000950 = 1;
  *(uint8_t*)0x20000952 = 9;
  *(uint8_t*)0x20000953 = 5;
  *(uint8_t*)0x20000954 = 0xb;
  *(uint8_t*)0x20000955 = 1;
  *(uint16_t*)0x20000956 = 0x20;
  *(uint8_t*)0x20000958 = 0;
  *(uint8_t*)0x20000959 = 3;
  *(uint8_t*)0x2000095a = 6;
  *(uint8_t*)0x2000095b = 7;
  *(uint8_t*)0x2000095c = 0x25;
  *(uint8_t*)0x2000095d = 1;
  *(uint8_t*)0x2000095e = 0x83;
  *(uint8_t*)0x2000095f = 0x25;
  *(uint16_t*)0x20000960 = 1;
  *(uint8_t*)0x20000962 = 2;
  *(uint8_t*)0x20000963 = 4;
  *(uint8_t*)0x20000964 = 9;
  *(uint8_t*)0x20000965 = 5;
  *(uint8_t*)0x20000966 = 0xe;
  *(uint8_t*)0x20000967 = 0x10;
  *(uint16_t*)0x20000968 = 0x200;
  *(uint8_t*)0x2000096a = 8;
  *(uint8_t*)0x2000096b = 0x4c;
  *(uint8_t*)0x2000096c = 2;
  *(uint8_t*)0x2000096d = 7;
  *(uint8_t*)0x2000096e = 0x25;
  *(uint8_t*)0x2000096f = 1;
  *(uint8_t*)0x20000970 = 0x81;
  *(uint8_t*)0x20000971 = 0x54;
  *(uint16_t*)0x20000972 = 0x20;
  *(uint8_t*)0x20000974 = 7;
  *(uint8_t*)0x20000975 = 0x25;
  *(uint8_t*)0x20000976 = 1;
  *(uint8_t*)0x20000977 = 0x80;
  *(uint8_t*)0x20000978 = 0x80;
  *(uint16_t*)0x20000979 = 1;
  *(uint8_t*)0x2000097b = 9;
  *(uint8_t*)0x2000097c = 4;
  *(uint8_t*)0x2000097d = 0x4a;
  *(uint8_t*)0x2000097e = 0;
  *(uint8_t*)0x2000097f = 0;
  *(uint8_t*)0x20000980 = 0xba;
  *(uint8_t*)0x20000981 = 0x1d;
  *(uint8_t*)0x20000982 = 0xee;
  *(uint8_t*)0x20000983 = 0;
  syz_usb_connect(0, 0x104, 0x20000880, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
