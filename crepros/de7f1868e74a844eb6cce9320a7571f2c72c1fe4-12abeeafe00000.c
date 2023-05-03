// https://syzkaller.appspot.com/bug?id=de7f1868e74a844eb6cce9320a7571f2c72c1fe4
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
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
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/usb/ch9.h>

static unsigned long long procid;

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

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
}

#define MAX_FDS 30

#define USB_MAX_IFACE_NUM 4
#define USB_MAX_EP_NUM 32
#define USB_MAX_FDS 6

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
        memcpy(&iface->eps[iface->eps_num], buffer + offset,
               sizeof(iface->eps[iface->eps_num]));
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
  for (i = 0; i < USB_MAX_FDS; i++) {
    if (__atomic_load_n(&usb_devices[i].fd, __ATOMIC_ACQUIRE) == fd) {
      return &usb_devices[i].index;
    }
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
  exit(1);
  return false;
}

typedef bool (*lookup_connect_out_response_t)(
    int fd, const struct vusb_connect_descriptors* descs,
    const struct usb_ctrlrequest* ctrl, bool* done);

#define ATH9K_FIRMWARE_DOWNLOAD 0x30
#define ATH9K_FIRMWARE_DOWNLOAD_COMP 0x31

static bool lookup_connect_response_out_ath9k(
    int fd, const struct vusb_connect_descriptors* descs,
    const struct usb_ctrlrequest* ctrl, bool* done)
{
  switch (ctrl->bRequestType & USB_TYPE_MASK) {
  case USB_TYPE_STANDARD:
    switch (ctrl->bRequest) {
    case USB_REQ_SET_CONFIGURATION:
      return true;
    default:
      break;
    }
    break;
  case USB_TYPE_VENDOR:
    switch (ctrl->bRequest) {
    case ATH9K_FIRMWARE_DOWNLOAD:
      return true;
    case ATH9K_FIRMWARE_DOWNLOAD_COMP:
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

static int usb_raw_ep_write(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_WRITE, io);
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
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      bool response_found = false;
      NONFAILING(response_found = lookup_connect_response_in(
                     fd, descs, &event.ctrl, &response_data, &response_length));
      if (!response_found) {
        return -1;
      }
    } else {
      if (!lookup_connect_response_out(fd, descs, &event.ctrl, &done)) {
        return -1;
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

static volatile long syz_usb_connect_ath9k(volatile long a0, volatile long a1,
                                           volatile long a2, volatile long a3)
{
  uint64_t speed = a0;
  uint64_t dev_len = a1;
  const char* dev = (const char*)a2;
  const struct vusb_connect_descriptors* descs =
      (const struct vusb_connect_descriptors*)a3;
  return syz_usb_connect_impl(speed, dev_len, dev, descs,
                              &lookup_connect_response_out_ath9k);
}

static volatile long syz_usb_ep_write(volatile long a0, volatile long a1,
                                      volatile long a2, volatile long a3)
{
  int fd = a0;
  uint16_t ep = a1;
  uint32_t len = a2;
  char* data = (char*)a3;
  struct usb_raw_ep_io_data io_data;
  io_data.inner.ep = ep;
  io_data.inner.flags = 0;
  if (len > sizeof(io_data.data))
    len = sizeof(io_data.data);
  io_data.inner.length = len;
  NONFAILING(memcpy(&io_data.data[0], data, len));
  int rv = usb_raw_ep_write(fd, (struct usb_raw_ep_io*)&io_data);
  if (rv < 0) {
    return rv;
  }
  sleep_ms(200);
  return 0;
}

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0ul);
  install_segv_handler();
  use_temporary_dir();
  intptr_t res = 0;
  NONFAILING(*(uint8_t*)0x20000000 = 0x12);
  NONFAILING(*(uint8_t*)0x20000001 = 1);
  NONFAILING(*(uint16_t*)0x20000002 = 0x200);
  NONFAILING(*(uint8_t*)0x20000004 = -1);
  NONFAILING(*(uint8_t*)0x20000005 = -1);
  NONFAILING(*(uint8_t*)0x20000006 = -1);
  NONFAILING(*(uint8_t*)0x20000007 = 0x40);
  NONFAILING(*(uint16_t*)0x20000008 = 0xcf3);
  NONFAILING(*(uint16_t*)0x2000000a = 0x9271);
  NONFAILING(*(uint16_t*)0x2000000c = 0x108);
  NONFAILING(*(uint8_t*)0x2000000e = 1);
  NONFAILING(*(uint8_t*)0x2000000f = 2);
  NONFAILING(*(uint8_t*)0x20000010 = 3);
  NONFAILING(*(uint8_t*)0x20000011 = 1);
  NONFAILING(*(uint8_t*)0x20000012 = 9);
  NONFAILING(*(uint8_t*)0x20000013 = 2);
  NONFAILING(*(uint16_t*)0x20000014 = 0x48);
  NONFAILING(*(uint8_t*)0x20000016 = 1);
  NONFAILING(*(uint8_t*)0x20000017 = 1);
  NONFAILING(*(uint8_t*)0x20000018 = 0);
  NONFAILING(*(uint8_t*)0x20000019 = 0x80);
  NONFAILING(*(uint8_t*)0x2000001a = 0xfa);
  NONFAILING(*(uint8_t*)0x2000001b = 9);
  NONFAILING(*(uint8_t*)0x2000001c = 4);
  NONFAILING(*(uint8_t*)0x2000001d = 0);
  NONFAILING(*(uint8_t*)0x2000001e = 0);
  NONFAILING(*(uint8_t*)0x2000001f = 6);
  NONFAILING(*(uint8_t*)0x20000020 = -1);
  NONFAILING(*(uint8_t*)0x20000021 = 0);
  NONFAILING(*(uint8_t*)0x20000022 = 0);
  NONFAILING(*(uint8_t*)0x20000023 = 0);
  NONFAILING(*(uint8_t*)0x20000024 = 9);
  NONFAILING(*(uint8_t*)0x20000025 = 5);
  NONFAILING(*(uint8_t*)0x20000026 = 1);
  NONFAILING(*(uint8_t*)0x20000027 = 2);
  NONFAILING(*(uint16_t*)0x20000028 = 0x200);
  NONFAILING(*(uint8_t*)0x2000002a = 0);
  NONFAILING(*(uint8_t*)0x2000002b = 0);
  NONFAILING(*(uint8_t*)0x2000002c = 0);
  NONFAILING(*(uint8_t*)0x2000002d = 9);
  NONFAILING(*(uint8_t*)0x2000002e = 5);
  NONFAILING(*(uint8_t*)0x2000002f = 0x82);
  NONFAILING(*(uint8_t*)0x20000030 = 2);
  NONFAILING(*(uint16_t*)0x20000031 = 0x200);
  NONFAILING(*(uint8_t*)0x20000033 = 0);
  NONFAILING(*(uint8_t*)0x20000034 = 0);
  NONFAILING(*(uint8_t*)0x20000035 = 0);
  NONFAILING(*(uint8_t*)0x20000036 = 9);
  NONFAILING(*(uint8_t*)0x20000037 = 5);
  NONFAILING(*(uint8_t*)0x20000038 = 0x83);
  NONFAILING(*(uint8_t*)0x20000039 = 3);
  NONFAILING(*(uint16_t*)0x2000003a = 0x40);
  NONFAILING(*(uint8_t*)0x2000003c = 1);
  NONFAILING(*(uint8_t*)0x2000003d = 0);
  NONFAILING(*(uint8_t*)0x2000003e = 0);
  NONFAILING(*(uint8_t*)0x2000003f = 9);
  NONFAILING(*(uint8_t*)0x20000040 = 5);
  NONFAILING(*(uint8_t*)0x20000041 = 4);
  NONFAILING(*(uint8_t*)0x20000042 = 3);
  NONFAILING(*(uint16_t*)0x20000043 = 0x40);
  NONFAILING(*(uint8_t*)0x20000045 = 1);
  NONFAILING(*(uint8_t*)0x20000046 = 0);
  NONFAILING(*(uint8_t*)0x20000047 = 0);
  NONFAILING(*(uint8_t*)0x20000048 = 9);
  NONFAILING(*(uint8_t*)0x20000049 = 5);
  NONFAILING(*(uint8_t*)0x2000004a = 5);
  NONFAILING(*(uint8_t*)0x2000004b = 2);
  NONFAILING(*(uint16_t*)0x2000004c = 0x200);
  NONFAILING(*(uint8_t*)0x2000004e = 0);
  NONFAILING(*(uint8_t*)0x2000004f = 0);
  NONFAILING(*(uint8_t*)0x20000050 = 0);
  NONFAILING(*(uint8_t*)0x20000051 = 9);
  NONFAILING(*(uint8_t*)0x20000052 = 5);
  NONFAILING(*(uint8_t*)0x20000053 = 6);
  NONFAILING(*(uint8_t*)0x20000054 = 2);
  NONFAILING(*(uint16_t*)0x20000055 = 0x200);
  NONFAILING(*(uint8_t*)0x20000057 = 0);
  NONFAILING(*(uint8_t*)0x20000058 = 0);
  NONFAILING(*(uint8_t*)0x20000059 = 0);
  res = syz_usb_connect_ath9k(3, 0x5a, 0x20000000, 0);
  if (res != -1)
    r[0] = res;
  NONFAILING(*(uint8_t*)0x20000080 = 0);
  NONFAILING(*(uint8_t*)0x20000081 = 0);
  NONFAILING(*(uint16_t*)0x20000082 = htobe16(0xa));
  NONFAILING(memcpy((void*)0x20000084, "\x18\x75\x4d\xba", 4));
  NONFAILING(*(uint16_t*)0x20000088 = htobe16(3));
  NONFAILING(*(uint16_t*)0x2000008a = htobe16(0));
  NONFAILING(*(uint8_t*)0x2000008c = 0);
  NONFAILING(*(uint8_t*)0x2000008d = 0xc1);
  NONFAILING(*(uint16_t*)0x2000008e = htobe16(3));
  NONFAILING(*(uint8_t*)0x20000090 = 9);
  NONFAILING(*(uint8_t*)0x20000091 = 0x40);
  syz_usb_ep_write(r[0], 2, 0x12, 0x20000080);
  return 0;
}
