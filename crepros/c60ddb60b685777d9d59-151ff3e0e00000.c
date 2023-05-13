// https://syzkaller.appspot.com/bug?id=c2f000b7826e712b064b66b32ed73e21ee09d7a5
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
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

unsigned long long procid;

static void sleep_ms(uint64_t ms)
{
  usleep(ms * 1000);
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

enum usb_fuzzer_event_type {
  USB_FUZZER_EVENT_INVALID,
  USB_FUZZER_EVENT_CONNECT,
  USB_FUZZER_EVENT_DISCONNECT,
  USB_FUZZER_EVENT_SUSPEND,
  USB_FUZZER_EVENT_RESUME,
  USB_FUZZER_EVENT_CONTROL,
};

struct usb_fuzzer_event {
  uint32_t type;
  uint32_t length;
  char data[0];
};

struct usb_fuzzer_init {
  uint64_t speed;
  const char* driver_name;
  const char* device_name;
};

struct usb_fuzzer_ep_io {
  uint16_t ep;
  uint16_t flags;
  uint32_t length;
  char data[0];
};

#define USB_FUZZER_IOCTL_INIT _IOW('U', 0, struct usb_fuzzer_init)
#define USB_FUZZER_IOCTL_RUN _IO('U', 1)
#define USB_FUZZER_IOCTL_EVENT_FETCH _IOR('U', 2, struct usb_fuzzer_event)
#define USB_FUZZER_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP0_READ _IOWR('U', 4, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP_ENABLE _IOW('U', 5, struct usb_endpoint_descriptor)
#define USB_FUZZER_IOCTL_EP_DISABLE _IOW('U', 6, int)
#define USB_FUZZER_IOCTL_EP_WRITE _IOW('U', 7, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP_READ _IOWR('U', 8, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_CONFIGURE _IO('U', 9)
#define USB_FUZZER_IOCTL_VBUS_DRAW _IOW('U', 10, uint32_t)

static int usb_fuzzer_open()
{
  return open("/sys/kernel/debug/usb-fuzzer", O_RDWR);
}

static int usb_fuzzer_init(int fd, uint32_t speed, const char* driver,
                           const char* device)
{
  struct usb_fuzzer_init arg;
  arg.speed = speed;
  arg.driver_name = driver;
  arg.device_name = device;
  return ioctl(fd, USB_FUZZER_IOCTL_INIT, &arg);
}

static int usb_fuzzer_run(int fd)
{
  return ioctl(fd, USB_FUZZER_IOCTL_RUN, 0);
}

static int usb_fuzzer_event_fetch(int fd, struct usb_fuzzer_event* event)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EVENT_FETCH, event);
}

static int usb_fuzzer_ep0_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_WRITE, io);
}

static int usb_fuzzer_ep0_read(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_READ, io);
}

static int usb_fuzzer_ep_enable(int fd, struct usb_endpoint_descriptor* desc)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_ENABLE, desc);
}

static int usb_fuzzer_ep_disable(int fd, int ep)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_DISABLE, ep);
}

static int usb_fuzzer_configure(int fd)
{
  return ioctl(fd, USB_FUZZER_IOCTL_CONFIGURE, 0);
}

static int usb_fuzzer_vbus_draw(int fd, uint32_t power)
{
  return ioctl(fd, USB_FUZZER_IOCTL_VBUS_DRAW, power);
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
      int rv = usb_fuzzer_ep_disable(fd, ep);
      if (rv < 0) {
      } else {
      }
    }
  }
  if (n >= 0 && n < index->ifaces_num) {
    for (ep = 0; ep < index->ifaces[n].eps_num; ep++) {
      int rv = usb_fuzzer_ep_enable(fd, &index->ifaces[n].eps[ep]);
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
  int rv = usb_fuzzer_vbus_draw(fd, index->bMaxPower);
  if (rv < 0) {
    return rv;
  }
  rv = usb_fuzzer_configure(fd);
  if (rv < 0) {
    return rv;
  }
  set_interface(fd, 0);
  return 0;
}

#define USB_MAX_PACKET_SIZE 1024

struct usb_fuzzer_control_event {
  struct usb_fuzzer_event inner;
  struct usb_ctrlrequest ctrl;
  char data[USB_MAX_PACKET_SIZE];
};

struct usb_fuzzer_ep_io_data {
  struct usb_fuzzer_ep_io inner;
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
  int fd = usb_fuzzer_open();
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
  int rv = usb_fuzzer_init(fd, speed, "dummy_udc", &device[0]);
  if (rv < 0) {
    return rv;
  }
  rv = usb_fuzzer_run(fd);
  if (rv < 0) {
    return rv;
  }
  bool done = false;
  while (!done) {
    struct usb_fuzzer_control_event event;
    event.inner.type = 0;
    event.inner.length = sizeof(event.ctrl);
    rv = usb_fuzzer_event_fetch(fd, (struct usb_fuzzer_event*)&event);
    if (rv < 0) {
      return rv;
    }
    if (event.inner.type != USB_FUZZER_EVENT_CONTROL)
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
    struct usb_fuzzer_ep_io_data response;
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
      rv = usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
    } else {
      rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_ep_io*)&response);
    }
    if (rv < 0) {
      return rv;
    }
  }
  sleep_ms(200);
  return fd;
}

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);

  *(uint8_t*)0x20000080 = 0x12;
  *(uint8_t*)0x20000081 = 1;
  *(uint16_t*)0x20000082 = 0x201;
  *(uint8_t*)0x20000084 = 0x32;
  *(uint8_t*)0x20000085 = 0xd4;
  *(uint8_t*)0x20000086 = 0x7f;
  *(uint8_t*)0x20000087 = 0x10;
  *(uint16_t*)0x20000088 = 0x1df7;
  *(uint16_t*)0x2000008a = 0x2500;
  *(uint16_t*)0x2000008c = 0x36db;
  *(uint8_t*)0x2000008e = 1;
  *(uint8_t*)0x2000008f = 2;
  *(uint8_t*)0x20000090 = 3;
  *(uint8_t*)0x20000091 = 1;
  *(uint8_t*)0x20000092 = 9;
  *(uint8_t*)0x20000093 = 2;
  *(uint16_t*)0x20000094 = 0x124;
  *(uint8_t*)0x20000096 = 3;
  *(uint8_t*)0x20000097 = 0x74;
  *(uint8_t*)0x20000098 = 5;
  *(uint8_t*)0x20000099 = 0xe0;
  *(uint8_t*)0x2000009a = 0;
  *(uint8_t*)0x2000009b = 9;
  *(uint8_t*)0x2000009c = 4;
  *(uint8_t*)0x2000009d = 0x74;
  *(uint8_t*)0x2000009e = 7;
  *(uint8_t*)0x2000009f = 0;
  *(uint8_t*)0x200000a0 = 0x93;
  *(uint8_t*)0x200000a1 = 0xb6;
  *(uint8_t*)0x200000a2 = 0x1c;
  *(uint8_t*)0x200000a3 = 5;
  *(uint8_t*)0x200000a4 = 5;
  *(uint8_t*)0x200000a5 = 0x24;
  *(uint8_t*)0x200000a6 = 6;
  *(uint8_t*)0x200000a7 = 0;
  *(uint8_t*)0x200000a8 = 1;
  *(uint8_t*)0x200000a9 = 5;
  *(uint8_t*)0x200000aa = 0x24;
  *(uint8_t*)0x200000ab = 0;
  *(uint16_t*)0x200000ac = 7;
  *(uint8_t*)0x200000ae = 0xd;
  *(uint8_t*)0x200000af = 0x24;
  *(uint8_t*)0x200000b0 = 0xf;
  *(uint8_t*)0x200000b1 = 1;
  *(uint32_t*)0x200000b2 = 4;
  *(uint16_t*)0x200000b6 = 6;
  *(uint16_t*)0x200000b8 = 0x1ff;
  *(uint8_t*)0x200000ba = 0x1c;
  *(uint8_t*)0x200000bb = 6;
  *(uint8_t*)0x200000bc = 0x24;
  *(uint8_t*)0x200000bd = 0x1a;
  *(uint16_t*)0x200000be = 3;
  *(uint8_t*)0x200000c0 = 5;
  *(uint8_t*)0x200000c1 = 9;
  *(uint8_t*)0x200000c2 = 4;
  *(uint8_t*)0x200000c3 = 0x8c;
  *(uint8_t*)0x200000c4 = 0x81;
  *(uint8_t*)0x200000c5 = 0xe;
  *(uint8_t*)0x200000c6 = 0xb0;
  *(uint8_t*)0x200000c7 = 0x72;
  *(uint8_t*)0x200000c8 = 0xfa;
  *(uint8_t*)0x200000c9 = 0x81;
  *(uint8_t*)0x200000ca = 9;
  *(uint8_t*)0x200000cb = 5;
  *(uint8_t*)0x200000cc = 0xb;
  *(uint8_t*)0x200000cd = 0x11;
  *(uint16_t*)0x200000ce = 0x14e;
  *(uint8_t*)0x200000d0 = 5;
  *(uint8_t*)0x200000d1 = 4;
  *(uint8_t*)0x200000d2 = 0x70;
  *(uint8_t*)0x200000d3 = 7;
  *(uint8_t*)0x200000d4 = 0x25;
  *(uint8_t*)0x200000d5 = 1;
  *(uint8_t*)0x200000d6 = 0x80;
  *(uint8_t*)0x200000d7 = 1;
  *(uint16_t*)0x200000d8 = 0x1000;
  *(uint8_t*)0x200000da = 2;
  *(uint8_t*)0x200000db = 0x30;
  *(uint8_t*)0x200000dc = 9;
  *(uint8_t*)0x200000dd = 5;
  *(uint8_t*)0x200000de = 0xa;
  *(uint8_t*)0x200000df = 0xc;
  *(uint16_t*)0x200000e0 = 0x290;
  *(uint8_t*)0x200000e2 = 5;
  *(uint8_t*)0x200000e3 = 7;
  *(uint8_t*)0x200000e4 = 7;
  *(uint8_t*)0x200000e5 = 9;
  *(uint8_t*)0x200000e6 = 5;
  *(uint8_t*)0x200000e7 = 0x80;
  *(uint8_t*)0x200000e8 = 1;
  *(uint16_t*)0x200000e9 = 0x142;
  *(uint8_t*)0x200000eb = 7;
  *(uint8_t*)0x200000ec = 2;
  *(uint8_t*)0x200000ed = 0x1e;
  *(uint8_t*)0x200000ee = 7;
  *(uint8_t*)0x200000ef = 0x25;
  *(uint8_t*)0x200000f0 = 1;
  *(uint8_t*)0x200000f1 = 2;
  *(uint8_t*)0x200000f2 = 5;
  *(uint16_t*)0x200000f3 = 3;
  *(uint8_t*)0x200000f5 = 9;
  *(uint8_t*)0x200000f6 = 5;
  *(uint8_t*)0x200000f7 = 0x96;
  *(uint8_t*)0x200000f8 = 8;
  *(uint16_t*)0x200000f9 = 6;
  *(uint8_t*)0x200000fb = 7;
  *(uint8_t*)0x200000fc = -1;
  *(uint8_t*)0x200000fd = 3;
  *(uint8_t*)0x200000fe = 9;
  *(uint8_t*)0x200000ff = 5;
  *(uint8_t*)0x20000100 = 5;
  *(uint8_t*)0x20000101 = 0;
  *(uint16_t*)0x20000102 = 0x365;
  *(uint8_t*)0x20000104 = 1;
  *(uint8_t*)0x20000105 = 0x10;
  *(uint8_t*)0x20000106 = 3;
  *(uint8_t*)0x20000107 = 7;
  *(uint8_t*)0x20000108 = 0x25;
  *(uint8_t*)0x20000109 = 1;
  *(uint8_t*)0x2000010a = 0;
  *(uint8_t*)0x2000010b = 0x80;
  *(uint16_t*)0x2000010c = 0x40;
  *(uint8_t*)0x2000010e = 7;
  *(uint8_t*)0x2000010f = 0x25;
  *(uint8_t*)0x20000110 = 1;
  *(uint8_t*)0x20000111 = 0xc2;
  *(uint8_t*)0x20000112 = 0x45;
  *(uint16_t*)0x20000113 = 1;
  *(uint8_t*)0x20000115 = 9;
  *(uint8_t*)0x20000116 = 5;
  *(uint8_t*)0x20000117 = 0xb;
  *(uint8_t*)0x20000118 = 0;
  *(uint16_t*)0x20000119 = 0x166;
  *(uint8_t*)0x2000011b = 0x58;
  *(uint8_t*)0x2000011c = 0;
  *(uint8_t*)0x2000011d = 0x40;
  *(uint8_t*)0x2000011e = 9;
  *(uint8_t*)0x2000011f = 5;
  *(uint8_t*)0x20000120 = 4;
  *(uint8_t*)0x20000121 = 4;
  *(uint16_t*)0x20000122 = 0x49;
  *(uint8_t*)0x20000124 = -1;
  *(uint8_t*)0x20000125 = 9;
  *(uint8_t*)0x20000126 = 3;
  *(uint8_t*)0x20000127 = 9;
  *(uint8_t*)0x20000128 = 5;
  *(uint8_t*)0x20000129 = 8;
  *(uint8_t*)0x2000012a = 3;
  *(uint16_t*)0x2000012b = 0x3d9;
  *(uint8_t*)0x2000012d = 3;
  *(uint8_t*)0x2000012e = 3;
  *(uint8_t*)0x2000012f = 8;
  *(uint8_t*)0x20000130 = 7;
  *(uint8_t*)0x20000131 = 0x25;
  *(uint8_t*)0x20000132 = 1;
  *(uint8_t*)0x20000133 = 1;
  *(uint8_t*)0x20000134 = 0xdd;
  *(uint16_t*)0x20000135 = 0xfeff;
  *(uint8_t*)0x20000137 = 7;
  *(uint8_t*)0x20000138 = 0x25;
  *(uint8_t*)0x20000139 = 1;
  *(uint8_t*)0x2000013a = 2;
  *(uint8_t*)0x2000013b = 0x7f;
  *(uint16_t*)0x2000013c = 2;
  *(uint8_t*)0x2000013e = 9;
  *(uint8_t*)0x2000013f = 5;
  *(uint8_t*)0x20000140 = 3;
  *(uint8_t*)0x20000141 = 0;
  *(uint16_t*)0x20000142 = 0x280;
  *(uint8_t*)0x20000144 = -1;
  *(uint8_t*)0x20000145 = 0xc;
  *(uint8_t*)0x20000146 = 2;
  *(uint8_t*)0x20000147 = 9;
  *(uint8_t*)0x20000148 = 5;
  *(uint8_t*)0x20000149 = 0xc;
  *(uint8_t*)0x2000014a = 0xc;
  *(uint16_t*)0x2000014b = 0x13b;
  *(uint8_t*)0x2000014d = 5;
  *(uint8_t*)0x2000014e = 9;
  *(uint8_t*)0x2000014f = 5;
  *(uint8_t*)0x20000150 = 9;
  *(uint8_t*)0x20000151 = 5;
  *(uint8_t*)0x20000152 = 0;
  *(uint8_t*)0x20000153 = 0x10;
  *(uint16_t*)0x20000154 = 0x1ab;
  *(uint8_t*)0x20000156 = 3;
  *(uint8_t*)0x20000157 = 8;
  *(uint8_t*)0x20000158 = 1;
  *(uint8_t*)0x20000159 = 2;
  *(uint8_t*)0x2000015a = 7;
  *(uint8_t*)0x2000015b = 9;
  *(uint8_t*)0x2000015c = 5;
  *(uint8_t*)0x2000015d = 0x80;
  *(uint8_t*)0x2000015e = 0x10;
  *(uint16_t*)0x2000015f = 0x297;
  *(uint8_t*)0x20000161 = 0x80;
  *(uint8_t*)0x20000162 = 0;
  *(uint8_t*)0x20000163 = 0xfd;
  *(uint8_t*)0x20000164 = 7;
  *(uint8_t*)0x20000165 = 0x25;
  *(uint8_t*)0x20000166 = 1;
  *(uint8_t*)0x20000167 = 0x82;
  *(uint8_t*)0x20000168 = 1;
  *(uint16_t*)0x20000169 = 9;
  *(uint8_t*)0x2000016b = 9;
  *(uint8_t*)0x2000016c = 5;
  *(uint8_t*)0x2000016d = 0xc;
  *(uint8_t*)0x2000016e = 0;
  *(uint16_t*)0x2000016f = 0x22b;
  *(uint8_t*)0x20000171 = 0xe7;
  *(uint8_t*)0x20000172 = 0;
  *(uint8_t*)0x20000173 = 0;
  *(uint8_t*)0x20000174 = 7;
  *(uint8_t*)0x20000175 = 0x25;
  *(uint8_t*)0x20000176 = 1;
  *(uint8_t*)0x20000177 = 0;
  *(uint8_t*)0x20000178 = 0;
  *(uint16_t*)0x20000179 = 0x3ff;
  *(uint8_t*)0x2000017b = 9;
  *(uint8_t*)0x2000017c = 5;
  *(uint8_t*)0x2000017d = 5;
  *(uint8_t*)0x2000017e = 1;
  *(uint16_t*)0x2000017f = 0x362;
  *(uint8_t*)0x20000181 = 0x19;
  *(uint8_t*)0x20000182 = 0xf7;
  *(uint8_t*)0x20000183 = 8;
  *(uint8_t*)0x20000184 = 2;
  *(uint8_t*)0x20000185 = 0xe;
  *(uint8_t*)0x20000186 = 7;
  *(uint8_t*)0x20000187 = 0x25;
  *(uint8_t*)0x20000188 = 1;
  *(uint8_t*)0x20000189 = 0x80;
  *(uint8_t*)0x2000018a = 3;
  *(uint16_t*)0x2000018b = 0x2a9;
  *(uint8_t*)0x2000018d = 9;
  *(uint8_t*)0x2000018e = 4;
  *(uint8_t*)0x2000018f = 0x98;
  *(uint8_t*)0x20000190 = 7;
  *(uint8_t*)0x20000191 = 1;
  *(uint8_t*)0x20000192 = 0;
  *(uint8_t*)0x20000193 = 0xc4;
  *(uint8_t*)0x20000194 = 0xf5;
  *(uint8_t*)0x20000195 = 0xa6;
  *(uint8_t*)0x20000196 = 5;
  *(uint8_t*)0x20000197 = 0x24;
  *(uint8_t*)0x20000198 = 6;
  *(uint8_t*)0x20000199 = 0;
  *(uint8_t*)0x2000019a = 0;
  *(uint8_t*)0x2000019b = 5;
  *(uint8_t*)0x2000019c = 0x24;
  *(uint8_t*)0x2000019d = 0;
  *(uint16_t*)0x2000019e = 0;
  *(uint8_t*)0x200001a0 = 0xd;
  *(uint8_t*)0x200001a1 = 0x24;
  *(uint8_t*)0x200001a2 = 0xf;
  *(uint8_t*)0x200001a3 = 1;
  *(uint32_t*)0x200001a4 = 7;
  *(uint16_t*)0x200001a8 = 0;
  *(uint16_t*)0x200001aa = 0xdd;
  *(uint8_t*)0x200001ac = 9;
  *(uint8_t*)0x200001ad = 9;
  *(uint8_t*)0x200001ae = 5;
  *(uint8_t*)0x200001af = 0x1c;
  *(uint8_t*)0x200001b0 = 0;
  *(uint16_t*)0x200001b1 = 0x3ed;
  *(uint8_t*)0x200001b3 = 4;
  *(uint8_t*)0x200001b4 = 3;
  *(uint8_t*)0x200001b5 = 4;
  *(uint32_t*)0x20000500 = 0;
  *(uint64_t*)0x20000504 = 0;
  *(uint32_t*)0x2000050c = 0;
  *(uint64_t*)0x20000510 = 0;
  *(uint32_t*)0x20000518 = 2;
  *(uint32_t*)0x2000051c = 0;
  *(uint64_t*)0x20000520 = 0;
  *(uint32_t*)0x20000528 = 0;
  *(uint64_t*)0x2000052c = 0;
  syz_usb_connect(4, 0x136, 0x20000080, 0x20000500);
  return 0;
}
