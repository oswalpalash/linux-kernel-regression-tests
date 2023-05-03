// https://syzkaller.appspot.com/bug?id=426fbebc1eac728afa08e52b1bcf8171c9413e29
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

#define USB_MAX_EP_NUM 32

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
  unsigned config_length;
  struct usb_interface_descriptor* iface;
  struct usb_endpoint_descriptor* eps[USB_MAX_EP_NUM];
  unsigned eps_num;
};

static bool parse_usb_descriptor(char* buffer, size_t length,
                                 struct usb_device_index* index)
{
  if (length <
      sizeof(*index->dev) + sizeof(*index->config) + sizeof(*index->iface))
    return false;
  index->dev = (struct usb_device_descriptor*)buffer;
  index->config = (struct usb_config_descriptor*)(buffer + sizeof(*index->dev));
  index->config_length = length - sizeof(*index->dev);
  index->iface =
      (struct usb_interface_descriptor*)(buffer + sizeof(*index->dev) +
                                         sizeof(*index->config));
  index->eps_num = 0;
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
    if (desc_type == USB_DT_ENDPOINT) {
      index->eps[index->eps_num] =
          (struct usb_endpoint_descriptor*)(buffer + offset);
      index->eps_num++;
    }
    if (index->eps_num == USB_MAX_EP_NUM)
      break;
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
        if (str_idx >= descs->strs_len && descs->strs_len > 0) {
          str_idx = descs->strs_len - 1;
        }
        *response_data = descs->strs[str_idx].str;
        *response_length = descs->strs[str_idx].len;
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
      for (ep = 0; ep < index.eps_num; ep++) {
        rv = usb_fuzzer_ep_enable(fd, index.eps[ep]);
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
    if (event.ctrl.bRequestType & USB_DIR_IN)
      rv = usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
    else
      rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_ep_io*)&response);
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

  *(uint8_t*)0x20000000 = 0x12;
  *(uint8_t*)0x20000001 = 1;
  *(uint16_t*)0x20000002 = 0x200;
  *(uint8_t*)0x20000004 = 0xfb;
  *(uint8_t*)0x20000005 = 0x3b;
  *(uint8_t*)0x20000006 = 0x66;
  *(uint8_t*)0x20000007 = 0x10;
  *(uint16_t*)0x20000008 = 0x1618;
  *(uint16_t*)0x2000000a = 0x9113;
  *(uint16_t*)0x2000000c = 0x6a87;
  *(uint8_t*)0x2000000e = 0;
  *(uint8_t*)0x2000000f = 2;
  *(uint8_t*)0x20000010 = 0x5e;
  *(uint8_t*)0x20000011 = 1;
  *(uint8_t*)0x20000012 = 9;
  *(uint8_t*)0x20000013 = 2;
  *(uint16_t*)0x20000014 = 0x9c;
  *(uint8_t*)0x20000016 = 1;
  *(uint8_t*)0x20000017 = 9;
  *(uint8_t*)0x20000018 = 2;
  *(uint8_t*)0x20000019 = 0x10;
  *(uint8_t*)0x2000001a = 0x2b;
  *(uint8_t*)0x2000001b = 9;
  *(uint8_t*)0x2000001c = 4;
  *(uint8_t*)0x2000001d = 0xe4;
  *(uint8_t*)0x2000001e = 0x37;
  *(uint8_t*)0x2000001f = 5;
  *(uint8_t*)0x20000020 = 0xf2;
  *(uint8_t*)0x20000021 = 0xc3;
  *(uint8_t*)0x20000022 = 0x5f;
  *(uint8_t*)0x20000023 = 0xc8;
  *(uint8_t*)0x20000024 = 2;
  *(uint8_t*)0x20000025 = 0x22;
  *(uint8_t*)0x20000026 = 0xc;
  *(uint8_t*)0x20000027 = 0x24;
  *(uint8_t*)0x20000028 = 0x1b;
  *(uint16_t*)0x20000029 = 0xfa;
  *(uint16_t*)0x2000002b = 2;
  *(uint8_t*)0x2000002d = -1;
  *(uint8_t*)0x2000002e = 9;
  *(uint16_t*)0x2000002f = 1;
  *(uint8_t*)0x20000031 = -1;
  *(uint8_t*)0x20000032 = 0xe;
  *(uint8_t*)0x20000033 = 0x24;
  *(uint8_t*)0x20000034 = 7;
  *(uint8_t*)0x20000035 = -1;
  *(uint16_t*)0x20000036 = 6;
  *(uint16_t*)0x20000038 = 5;
  *(uint16_t*)0x2000003a = 0xcf;
  *(uint16_t*)0x2000003c = 0x101;
  *(uint16_t*)0x2000003e = 1;
  *(uint8_t*)0x20000040 = 7;
  *(uint8_t*)0x20000041 = 0x24;
  *(uint8_t*)0x20000042 = 0x14;
  *(uint16_t*)0x20000043 = 4;
  *(uint16_t*)0x20000045 = 9;
  *(uint8_t*)0x20000047 = 5;
  *(uint8_t*)0x20000048 = 0x24;
  *(uint8_t*)0x20000049 = 0x15;
  *(uint16_t*)0x2000004a = 0xda;
  *(uint8_t*)0x2000004c = 6;
  *(uint8_t*)0x2000004d = 0x24;
  *(uint8_t*)0x2000004e = 0x1a;
  *(uint16_t*)0x2000004f = 0;
  *(uint8_t*)0x20000051 = 9;
  *(uint8_t*)0x20000052 = 4;
  *(uint8_t*)0x20000053 = 0x24;
  *(uint8_t*)0x20000054 = 1;
  *(uint8_t*)0x20000055 = 2;
  *(uint8_t*)0x20000056 = 4;
  *(uint8_t*)0x20000057 = 0x24;
  *(uint8_t*)0x20000058 = 0x13;
  *(uint8_t*)0x20000059 = 4;
  *(uint8_t*)0x2000005a = 4;
  *(uint8_t*)0x2000005b = 0x24;
  *(uint8_t*)0x2000005c = 2;
  *(uint8_t*)0x2000005d = 8;
  *(uint8_t*)0x2000005e = 0xe;
  *(uint8_t*)0x2000005f = 0x24;
  *(uint8_t*)0x20000060 = 7;
  *(uint8_t*)0x20000061 = 0xf8;
  *(uint16_t*)0x20000062 = 2;
  *(uint16_t*)0x20000064 = 7;
  *(uint16_t*)0x20000066 = 0x34a;
  *(uint16_t*)0x20000068 = 1;
  *(uint16_t*)0x2000006a = 6;
  *(uint8_t*)0x2000006c = 8;
  *(uint8_t*)0x2000006d = 0x24;
  *(uint8_t*)0x2000006e = 0x1c;
  *(uint16_t*)0x2000006f = 2;
  *(uint8_t*)0x20000071 = 9;
  *(uint16_t*)0x20000072 = 1;
  *(uint8_t*)0x20000074 = 3;
  *(uint8_t*)0x20000075 = 0x24;
  *(uint8_t*)0x20000076 = 0x1f;
  *(uint8_t*)0x20000077 = 9;
  *(uint8_t*)0x20000078 = 5;
  *(uint8_t*)0x20000079 = 4;
  *(uint8_t*)0x2000007a = 2;
  *(uint16_t*)0x2000007b = 0x81;
  *(uint8_t*)0x2000007d = 6;
  *(uint8_t*)0x2000007e = 0;
  *(uint8_t*)0x2000007f = 0;
  *(uint8_t*)0x20000080 = 2;
  *(uint8_t*)0x20000081 = 0x22;
  *(uint8_t*)0x20000082 = 9;
  *(uint8_t*)0x20000083 = 5;
  *(uint8_t*)0x20000084 = 0xf;
  *(uint8_t*)0x20000085 = 2;
  *(uint16_t*)0x20000086 = 0x94;
  *(uint8_t*)0x20000088 = -1;
  *(uint8_t*)0x20000089 = 0;
  *(uint8_t*)0x2000008a = 0xfd;
  *(uint8_t*)0x2000008b = 2;
  *(uint8_t*)0x2000008c = 0x2f;
  *(uint8_t*)0x2000008d = 2;
  *(uint8_t*)0x2000008e = 0x2f;
  *(uint8_t*)0x2000008f = 9;
  *(uint8_t*)0x20000090 = 5;
  *(uint8_t*)0x20000091 = 0xf;
  *(uint8_t*)0x20000092 = 0x17;
  *(uint16_t*)0x20000093 = 0xfe;
  *(uint8_t*)0x20000095 = 8;
  *(uint8_t*)0x20000096 = 0xf7;
  *(uint8_t*)0x20000097 = 8;
  *(uint8_t*)0x20000098 = 2;
  *(uint8_t*)0x20000099 = 0x2d;
  *(uint8_t*)0x2000009a = 9;
  *(uint8_t*)0x2000009b = 5;
  *(uint8_t*)0x2000009c = 0xc;
  *(uint8_t*)0x2000009d = 2;
  *(uint16_t*)0x2000009e = 0;
  *(uint8_t*)0x200000a0 = 5;
  *(uint8_t*)0x200000a1 = 0;
  *(uint8_t*)0x200000a2 = 6;
  *(uint8_t*)0x200000a3 = 2;
  *(uint8_t*)0x200000a4 = 2;
  *(uint8_t*)0x200000a5 = 9;
  *(uint8_t*)0x200000a6 = 5;
  *(uint8_t*)0x200000a7 = 3;
  *(uint8_t*)0x200000a8 = 0x10;
  *(uint16_t*)0x200000a9 = 0x40;
  *(uint8_t*)0x200000ab = 0;
  *(uint8_t*)0x200000ac = 4;
  *(uint8_t*)0x200000ad = 1;
  *(uint32_t*)0x20000580 = 0;
  *(uint64_t*)0x20000584 = 0;
  *(uint32_t*)0x2000058c = 0;
  *(uint64_t*)0x20000590 = 0;
  *(uint32_t*)0x20000598 = 1;
  *(uint32_t*)0x2000059c = 0;
  *(uint64_t*)0x200005a0 = 0;
  syz_usb_connect(5, 0xae, 0x20000000, 0x20000580);
  return 0;
}
