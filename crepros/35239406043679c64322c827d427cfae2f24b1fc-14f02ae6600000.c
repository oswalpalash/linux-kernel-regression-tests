// https://syzkaller.appspot.com/bug?id=35239406043679c64322c827d427cfae2f24b1fc
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

#define USB_DEBUG 0

#define USB_MAX_IFACE_NUM 4
#define USB_MAX_EP_NUM 32

struct usb_iface_index {
  struct usb_interface_descriptor* iface;
  struct usb_endpoint_descriptor* eps[USB_MAX_EP_NUM];
  unsigned eps_num;
};

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
  unsigned config_length;
  struct usb_iface_index ifaces[USB_MAX_IFACE_NUM];
  unsigned ifaces_num;
};

static bool parse_usb_descriptor(char* buffer, size_t length,
                                 struct usb_device_index* index)
{
  if (length < sizeof(*index->dev) + sizeof(*index->config))
    return false;
  memset(index, 0, sizeof(*index));
  index->dev = (struct usb_device_descriptor*)buffer;
  index->config = (struct usb_config_descriptor*)(buffer + sizeof(*index->dev));
  index->config_length = length - sizeof(*index->dev);
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
      index->ifaces[index->ifaces_num++].iface = iface;
    }
    if (desc_type == USB_DT_ENDPOINT && index->ifaces_num > 0) {
      struct usb_iface_index* iface = &index->ifaces[index->ifaces_num - 1];
      if (iface->eps_num < USB_MAX_EP_NUM)
        iface->eps[iface->eps_num++] =
            (struct usb_endpoint_descriptor*)(buffer + offset);
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
#define USB_FUZZER_IOCTL_EP_WRITE _IOW('U', 7, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP_READ _IOWR('U', 8, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_CONFIGURE _IO('U', 9)
#define USB_FUZZER_IOCTL_VBUS_DRAW _IOW('U', 10, uint32_t)

int usb_fuzzer_open()
{
  return open("/sys/kernel/debug/usb-fuzzer", O_RDWR);
}

int usb_fuzzer_init(int fd, uint32_t speed, const char* driver,
                    const char* device)
{
  struct usb_fuzzer_init arg;
  arg.speed = speed;
  arg.driver_name = driver;
  arg.device_name = device;
  return ioctl(fd, USB_FUZZER_IOCTL_INIT, &arg);
}

int usb_fuzzer_run(int fd)
{
  return ioctl(fd, USB_FUZZER_IOCTL_RUN, 0);
}

int usb_fuzzer_event_fetch(int fd, struct usb_fuzzer_event* event)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EVENT_FETCH, event);
}

int usb_fuzzer_ep0_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_WRITE, io);
}

int usb_fuzzer_ep0_read(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_READ, io);
}

int usb_fuzzer_ep_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_WRITE, io);
}

int usb_fuzzer_ep_read(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_READ, io);
}

int usb_fuzzer_ep_enable(int fd, struct usb_endpoint_descriptor* desc)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_ENABLE, desc);
}

int usb_fuzzer_configure(int fd)
{
  return ioctl(fd, USB_FUZZER_IOCTL_CONFIGURE, 0);
}

int usb_fuzzer_vbus_draw(int fd, uint32_t power)
{
  return ioctl(fd, USB_FUZZER_IOCTL_VBUS_DRAW, power);
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

static bool lookup_connect_response(struct vusb_connect_descriptors* descs,
                                    struct usb_device_index* index,
                                    struct usb_ctrlrequest* ctrl,
                                    char** response_data,
                                    uint32_t* response_length)
{
  uint8_t str_idx;
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
  struct usb_device_index index;
  memset(&index, 0, sizeof(index));
  int rv = 0;
  rv = parse_usb_descriptor(dev, dev_len, &index);
  if (!rv) {
    return rv;
  }
  int fd = usb_fuzzer_open();
  if (fd < 0) {
    return fd;
  }
  char device[32];
  sprintf(&device[0], "dummy_udc.%llu", procid);
  rv = usb_fuzzer_init(fd, speed, "dummy_udc", &device[0]);
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
          descs, &index, &event.ctrl, &response_data, &response_length);
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
      rv = usb_fuzzer_vbus_draw(fd, index.config->bMaxPower);
      if (rv < 0) {
        return rv;
      }
      rv = usb_fuzzer_configure(fd);
      if (rv < 0) {
        return rv;
      }
      unsigned ep;
      for (ep = 0; ep < index.ifaces[0].eps_num; ep++) {
        rv = usb_fuzzer_ep_enable(fd, index.ifaces[0].eps[ep]);
        if (rv < 0) {
        } else {
        }
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

  *(uint8_t*)0x200008c0 = 0x12;
  *(uint8_t*)0x200008c1 = 1;
  *(uint16_t*)0x200008c2 = 0;
  *(uint8_t*)0x200008c4 = 0xcd;
  *(uint8_t*)0x200008c5 = 0x78;
  *(uint8_t*)0x200008c6 = 0x39;
  *(uint8_t*)0x200008c7 = 8;
  *(uint16_t*)0x200008c8 = 0x54c;
  *(uint16_t*)0x200008ca = 0x6c1;
  *(uint16_t*)0x200008cc = 0xe5ee;
  *(uint8_t*)0x200008ce = 0;
  *(uint8_t*)0x200008cf = 0;
  *(uint8_t*)0x200008d0 = 0;
  *(uint8_t*)0x200008d1 = 1;
  *(uint8_t*)0x200008d2 = 9;
  *(uint8_t*)0x200008d3 = 2;
  *(uint16_t*)0x200008d4 = 0xfd91;
  *(uint8_t*)0x200008d6 = 1;
  *(uint8_t*)0x200008d7 = 0;
  *(uint8_t*)0x200008d8 = 0;
  *(uint8_t*)0x200008d9 = 0;
  *(uint8_t*)0x200008da = 2;
  *(uint8_t*)0x200008db = 9;
  *(uint8_t*)0x200008dc = 4;
  *(uint8_t*)0x200008dd = 0x2f;
  *(uint8_t*)0x200008de = 0;
  *(uint8_t*)0x200008df = 2;
  *(uint8_t*)0x200008e0 = 0x5b;
  *(uint8_t*)0x200008e1 = 0x7c;
  *(uint8_t*)0x200008e2 = 0x1e;
  *(uint8_t*)0x200008e3 = 0;
  *(uint8_t*)0x200008e4 = 9;
  *(uint8_t*)0x200008e5 = 5;
  *(uint8_t*)0x200008e6 = 0xb;
  *(uint8_t*)0x200008e7 = 2;
  *(uint16_t*)0x200008e8 = 0x3d2;
  *(uint8_t*)0x200008ea = 0;
  *(uint8_t*)0x200008eb = 0;
  *(uint8_t*)0x200008ec = 0;
  *(uint8_t*)0x200008ed = 9;
  *(uint8_t*)0x200008ee = 5;
  *(uint8_t*)0x200008ef = 0x8a;
  *(uint8_t*)0x200008f0 = 2;
  *(uint16_t*)0x200008f1 = 0;
  *(uint8_t*)0x200008f3 = 0;
  *(uint8_t*)0x200008f4 = 0;
  *(uint8_t*)0x200008f5 = 0;
  syz_usb_connect(0, 0x6e, 0x200008c0, 0);
  return 0;
}
