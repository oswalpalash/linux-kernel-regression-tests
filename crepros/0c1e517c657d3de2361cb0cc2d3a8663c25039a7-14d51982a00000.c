// https://syzkaller.appspot.com/bug?id=0c1e517c657d3de2361cb0cc2d3a8663c25039a7
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
    if (offset == length)
      break;
    if (offset + 1 < length)
      break;
    uint8_t length = buffer[offset];
    uint8_t type = buffer[offset + 1];
    if (type == USB_DT_ENDPOINT) {
      index->eps[index->eps_num] =
          (struct usb_endpoint_descriptor*)(buffer + offset);
      index->eps_num++;
    }
    if (index->eps_num == USB_MAX_EP_NUM)
      break;
    offset += length;
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
#define USB_FUZZER_IOCTL_EP0_READ _IOWR('U', 2, struct usb_fuzzer_event)
#define USB_FUZZER_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_EP_ENABLE _IOW('U', 4, struct usb_endpoint_descriptor)
#define USB_FUZZER_IOCTL_EP_WRITE _IOW('U', 6, struct usb_fuzzer_ep_io)
#define USB_FUZZER_IOCTL_CONFIGURE _IO('U', 8)
#define USB_FUZZER_IOCTL_VBUS_DRAW _IOW('U', 9, uint32_t)

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

int usb_fuzzer_ep0_read(int fd, struct usb_fuzzer_event* event)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_READ, event);
}

int usb_fuzzer_ep0_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP0_WRITE, io);
}

int usb_fuzzer_ep_write(int fd, struct usb_fuzzer_ep_io* io)
{
  return ioctl(fd, USB_FUZZER_IOCTL_EP_WRITE, io);
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
                                    uint32_t* response_length, bool* done)
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
        if (str_idx >= descs->strs_len)
          return false;
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
    case USB_REQ_SET_CONFIGURATION:
      *response_length = 0;
      *response_data = NULL;
      *done = true;
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
  return false;
}

static volatile long syz_usb_connect(volatile long a0, volatile long a1,
                                     volatile long a2, volatile long a3)
{
  int64_t speed = a0;
  int64_t dev_len = a1;
  char* dev = (char*)a2;
  struct vusb_connect_descriptors* descs = (struct vusb_connect_descriptors*)a3;
  if (!dev)
    return -1;
  struct usb_device_index index;
  memset(&index, 0, sizeof(index));
  int rv = false;
  NONFAILING(rv = parse_usb_descriptor(dev, dev_len, &index));
  if (!rv)
    return -1;
  int fd = usb_fuzzer_open();
  if (fd < 0)
    return -1;
  char device[32];
  sprintf(&device[0], "dummy_udc.%llu", procid);
  rv = usb_fuzzer_init(fd, speed, "dummy_udc", &device[0]);
  if (rv < 0)
    return -1;
  rv = usb_fuzzer_run(fd);
  if (rv < 0)
    return -1;
  bool done = false;
  while (!done) {
    struct usb_fuzzer_control_event event;
    event.inner.type = 0;
    event.inner.length = sizeof(event.ctrl);
    rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_event*)&event);
    if (rv < 0)
      return -1;
    if (event.inner.type != USB_FUZZER_EVENT_CONTROL)
      continue;
    bool response_found = false;
    char* response_data = NULL;
    uint32_t response_length = 0;
    NONFAILING(response_found = lookup_connect_response(
                   descs, &index, &event.ctrl, &response_data, &response_length,
                   &done));
    if (!response_found)
      return -1;
    if (done) {
      int rv = usb_fuzzer_vbus_draw(fd, index.config->bMaxPower);
      if (rv < 0)
        return -1;
      rv = usb_fuzzer_configure(fd);
      if (rv < 0)
        return -1;
      unsigned ep;
      for (ep = 0; ep < index.eps_num; ep++) {
        rv = usb_fuzzer_ep_enable(fd, index.eps[ep]);
        if (rv < 0)
          exit(1);
      }
    }
    struct usb_fuzzer_ep_io_data response;
    response.inner.ep = 0;
    response.inner.flags = 0;
    if (response_length > sizeof(response.data))
      response_length = 0;
    response.inner.length = response_length;
    if (response_data)
      memcpy(&response.data[0], response_data, response_length);
    if (event.ctrl.wLength < response.inner.length)
      response.inner.length = event.ctrl.wLength;
    usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
  }
  sleep_ms(200);
  return fd;
}

struct vusb_descriptor {
  uint8_t req_type;
  uint8_t desc_type;
  uint32_t len;
  char data[0];
} __attribute__((packed));

struct vusb_descriptors {
  uint32_t len;
  struct vusb_descriptor* generic;
  struct vusb_descriptor* descs[0];
} __attribute__((packed));

struct vusb_response {
  uint8_t type;
  uint8_t req;
  uint32_t len;
  char data[0];
} __attribute__((packed));

struct vusb_responses {
  uint32_t len;
  struct vusb_response* generic;
  struct vusb_response* resps[0];
} __attribute__((packed));

static bool lookup_control_io_response(struct vusb_descriptors* descs,
                                       struct vusb_responses* resps,
                                       struct usb_ctrlrequest* ctrl,
                                       char** response_data,
                                       uint32_t* response_length)
{
  int descs_num = (descs->len - offsetof(struct vusb_descriptors, descs)) /
                  sizeof(descs->descs[0]);
  int resps_num = (resps->len - offsetof(struct vusb_responses, resps)) /
                  sizeof(resps->resps[0]);
  uint8_t req = ctrl->bRequest;
  uint8_t req_type = ctrl->bRequestType & USB_TYPE_MASK;
  uint8_t desc_type = ctrl->wValue >> 8;
  if (req == USB_REQ_GET_DESCRIPTOR) {
    int i;
    for (i = 0; i < descs_num; i++) {
      struct vusb_descriptor* desc = descs->descs[i];
      if (!desc)
        continue;
      if (desc->req_type == req_type && desc->desc_type == desc_type) {
        *response_length = desc->len;
        if (*response_length != 0)
          *response_data = &desc->data[0];
        else
          *response_data = NULL;
        return true;
      }
    }
    if (descs->generic) {
      *response_data = &descs->generic->data[0];
      *response_length = descs->generic->len;
      return true;
    }
  } else {
    int i;
    for (i = 0; i < resps_num; i++) {
      struct vusb_response* resp = resps->resps[i];
      if (!resp)
        continue;
      if (resp->type == req_type && resp->req == req) {
        *response_length = resp->len;
        if (*response_length != 0)
          *response_data = &resp->data[0];
        else
          *response_data = NULL;
        return true;
      }
    }
    if (resps->generic) {
      *response_data = &resps->generic->data[0];
      *response_length = resps->generic->len;
      return true;
    }
  }
  return false;
}

static volatile long syz_usb_control_io(volatile long a0, volatile long a1,
                                        volatile long a2)
{
  int fd = a0;
  struct vusb_descriptors* descs = (struct vusb_descriptors*)a1;
  struct vusb_responses* resps = (struct vusb_responses*)a2;
  struct usb_fuzzer_control_event event;
  event.inner.type = 0;
  event.inner.length = sizeof(event.ctrl);
  int rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_event*)&event);
  if (rv < 0)
    return -1;
  if (event.inner.type != USB_FUZZER_EVENT_CONTROL)
    return -1;
  bool response_found = false;
  char* response_data = NULL;
  uint32_t response_length = 0;
  NONFAILING(response_found = lookup_control_io_response(
                 descs, resps, &event.ctrl, &response_data, &response_length));
  if (!response_found)
    return -1;
  struct usb_fuzzer_ep_io_data response;
  response.inner.ep = 0;
  response.inner.flags = 0;
  if (response_length > sizeof(response.data))
    response_length = 0;
  response.inner.length = response_length;
  if (response_data)
    memcpy(&response.data[0], response_data, response_length);
  if (event.ctrl.wLength < response.inner.length)
    response.inner.length = event.ctrl.wLength;
  usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
  return 0;
}

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  intptr_t res = 0;
  NONFAILING(*(uint8_t*)0x20000000 = 0x12);
  NONFAILING(*(uint8_t*)0x20000001 = 1);
  NONFAILING(*(uint16_t*)0x20000002 = 0);
  NONFAILING(*(uint8_t*)0x20000004 = 0xb0);
  NONFAILING(*(uint8_t*)0x20000005 = 0xf7);
  NONFAILING(*(uint8_t*)0x20000006 = 0x87);
  NONFAILING(*(uint8_t*)0x20000007 = 8);
  NONFAILING(*(uint16_t*)0x20000008 = 0xe41);
  NONFAILING(*(uint16_t*)0x2000000a = 0x4151);
  NONFAILING(*(uint16_t*)0x2000000c = 0x7a8f);
  NONFAILING(*(uint8_t*)0x2000000e = 0);
  NONFAILING(*(uint8_t*)0x2000000f = 0);
  NONFAILING(*(uint8_t*)0x20000010 = 0);
  NONFAILING(*(uint8_t*)0x20000011 = 1);
  NONFAILING(*(uint8_t*)0x20000012 = 9);
  NONFAILING(*(uint8_t*)0x20000013 = 2);
  NONFAILING(*(uint16_t*)0x20000014 = 0x12);
  NONFAILING(*(uint8_t*)0x20000016 = 1);
  NONFAILING(*(uint8_t*)0x20000017 = 0);
  NONFAILING(*(uint8_t*)0x20000018 = 0);
  NONFAILING(*(uint8_t*)0x20000019 = 0);
  NONFAILING(*(uint8_t*)0x2000001a = 0);
  NONFAILING(*(uint8_t*)0x2000001b = 9);
  NONFAILING(*(uint8_t*)0x2000001c = 4);
  NONFAILING(*(uint8_t*)0x2000001d = 0);
  NONFAILING(*(uint8_t*)0x2000001e = 2);
  NONFAILING(*(uint8_t*)0x2000001f = 0);
  NONFAILING(*(uint8_t*)0x20000020 = 0xd6);
  NONFAILING(*(uint8_t*)0x20000021 = 0x36);
  NONFAILING(*(uint8_t*)0x20000022 = 0x16);
  NONFAILING(*(uint8_t*)0x20000023 = 0);
  res = syz_usb_connect(0, 0x24, 0x20000000, 0);
  if (res != -1)
    r[0] = res;
  NONFAILING(*(uint32_t*)0x20003900 = 0x34);
  NONFAILING(*(uint64_t*)0x20003904 = 0);
  NONFAILING(*(uint64_t*)0x2000390c = 0);
  NONFAILING(*(uint64_t*)0x20003914 = 0);
  NONFAILING(*(uint64_t*)0x2000391c = 0);
  NONFAILING(*(uint64_t*)0x20003924 = 0);
  NONFAILING(*(uint64_t*)0x2000392c = 0);
  NONFAILING(*(uint32_t*)0x20003c40 = 0x54);
  NONFAILING(*(uint64_t*)0x20003c44 = 0);
  NONFAILING(*(uint64_t*)0x20003c4c = 0x20003a00);
  NONFAILING(*(uint8_t*)0x20003a00 = 0);
  NONFAILING(*(uint8_t*)0x20003a01 = 0xb);
  NONFAILING(*(uint32_t*)0x20003a02 = 0);
  NONFAILING(*(uint64_t*)0x20003c54 = 0);
  NONFAILING(*(uint64_t*)0x20003c5c = 0);
  NONFAILING(*(uint64_t*)0x20003c64 = 0);
  NONFAILING(*(uint64_t*)0x20003c6c = 0);
  NONFAILING(*(uint64_t*)0x20003c74 = 0);
  NONFAILING(*(uint64_t*)0x20003c7c = 0);
  NONFAILING(*(uint64_t*)0x20003c84 = 0);
  NONFAILING(*(uint64_t*)0x20003c8c = 0);
  syz_usb_control_io(r[0], 0x20003900, 0x20003c40);
  syz_usb_control_io(r[0], 0, 0);
  syz_usb_control_io(r[0], 0, 0);
  syz_usb_control_io(r[0], 0, 0);
  syz_usb_control_io(r[0], 0, 0);
  syz_usb_control_io(r[0], 0, 0);
  syz_usb_control_io(r[0], 0, 0);
  syz_usb_control_io(r[0], 0, 0);
  syz_usb_control_io(r[0], 0, 0);
  return 0;
}
