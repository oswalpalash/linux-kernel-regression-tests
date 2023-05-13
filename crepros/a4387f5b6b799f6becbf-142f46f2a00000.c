// https://syzkaller.appspot.com/bug?id=775f90f43cfd6f8ac6c15251ce68e604453da226
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
  rv = parse_usb_descriptor(dev, dev_len, &index);
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
    response_found = lookup_connect_response(
        descs, &index, &event.ctrl, &response_data, &response_length, &done);
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
  response_found = lookup_control_io_response(descs, resps, &event.ctrl,
                                              &response_data, &response_length);
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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  *(uint8_t*)0x20000000 = 0x12;
  *(uint8_t*)0x20000001 = 1;
  *(uint16_t*)0x20000002 = 0;
  *(uint8_t*)0x20000004 = 0xac;
  *(uint8_t*)0x20000005 = 0xf1;
  *(uint8_t*)0x20000006 = 0xfb;
  *(uint8_t*)0x20000007 = 8;
  *(uint16_t*)0x20000008 = 0x10c4;
  *(uint16_t*)0x2000000a = 0x818a;
  *(uint16_t*)0x2000000c = 0x3710;
  *(uint8_t*)0x2000000e = 0;
  *(uint8_t*)0x2000000f = 0;
  *(uint8_t*)0x20000010 = 0;
  *(uint8_t*)0x20000011 = 1;
  *(uint8_t*)0x20000012 = 9;
  *(uint8_t*)0x20000013 = 2;
  *(uint16_t*)0x20000014 = 0x12;
  *(uint8_t*)0x20000016 = 1;
  *(uint8_t*)0x20000017 = 0;
  *(uint8_t*)0x20000018 = 0;
  *(uint8_t*)0x20000019 = 0;
  *(uint8_t*)0x2000001a = 0;
  *(uint8_t*)0x2000001b = 9;
  *(uint8_t*)0x2000001c = 4;
  *(uint8_t*)0x2000001d = 0xd2;
  *(uint8_t*)0x2000001e = 0;
  *(uint8_t*)0x2000001f = 0;
  *(uint8_t*)0x20000020 = 3;
  *(uint8_t*)0x20000021 = 0;
  *(uint8_t*)0x20000022 = 0;
  *(uint8_t*)0x20000023 = 0;
  res = syz_usb_connect(0, 0x24, 0x20000000, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000040 = 0x34;
  *(uint64_t*)0x20000044 = 0;
  *(uint64_t*)0x2000004c = 0;
  *(uint64_t*)0x20000054 = 0;
  *(uint64_t*)0x2000005c = 0;
  *(uint64_t*)0x20000064 = 0;
  *(uint64_t*)0x2000006c = 0;
  *(uint32_t*)0x20000500 = 0x54;
  *(uint64_t*)0x20000504 = 0x20003000;
  *(uint8_t*)0x20003000 = 0;
  *(uint8_t*)0x20003001 = 0;
  *(uint32_t*)0x20003002 = 3;
  memcpy((void*)0x20003006, "\xea\x8b\x5f", 3);
  *(uint64_t*)0x2000050c = 0;
  *(uint64_t*)0x20000514 = 0;
  *(uint64_t*)0x2000051c = 0;
  *(uint64_t*)0x20000524 = 0;
  *(uint64_t*)0x2000052c = 0;
  *(uint64_t*)0x20000534 = 0;
  *(uint64_t*)0x2000053c = 0;
  *(uint64_t*)0x20000544 = 0;
  *(uint64_t*)0x2000054c = 0;
  syz_usb_control_io(r[0], 0x20000040, 0x20000500);
  *(uint8_t*)0x20001a40 = 0x12;
  *(uint8_t*)0x20001a41 = 1;
  *(uint16_t*)0x20001a42 = 9;
  *(uint8_t*)0x20001a44 = 0x3b;
  *(uint8_t*)0x20001a45 = 0xbf;
  *(uint8_t*)0x20001a46 = 0x18;
  *(uint8_t*)0x20001a47 = -1;
  *(uint16_t*)0x20001a48 = 0xa21;
  *(uint16_t*)0x20001a4a = 0x8001;
  *(uint16_t*)0x20001a4c = 0x50bb;
  *(uint8_t*)0x20001a4e = 0;
  *(uint8_t*)0x20001a4f = 0;
  *(uint8_t*)0x20001a50 = 1;
  *(uint8_t*)0x20001a51 = 1;
  *(uint8_t*)0x20001a52 = 9;
  *(uint8_t*)0x20001a53 = 2;
  *(uint16_t*)0x20001a54 = 0x470;
  *(uint8_t*)0x20001a56 = 1;
  *(uint8_t*)0x20001a57 = 0;
  *(uint8_t*)0x20001a58 = 6;
  *(uint8_t*)0x20001a59 = 0x10;
  *(uint8_t*)0x20001a5a = 0x80;
  *(uint8_t*)0x20001a5b = 9;
  *(uint8_t*)0x20001a5c = 4;
  *(uint8_t*)0x20001a5d = 0xac;
  *(uint8_t*)0x20001a5e = 2;
  *(uint8_t*)0x20001a5f = 8;
  *(uint8_t*)0x20001a60 = 0x86;
  *(uint8_t*)0x20001a61 = 0xcf;
  *(uint8_t*)0x20001a62 = 0xb2;
  *(uint8_t*)0x20001a63 = 4;
  *(uint8_t*)0x20001a64 = 5;
  *(uint8_t*)0x20001a65 = 0x24;
  *(uint8_t*)0x20001a66 = 0x15;
  *(uint16_t*)0x20001a67 = 0xfffc;
  *(uint8_t*)0x20001a69 = 4;
  *(uint8_t*)0x20001a6a = 0x24;
  *(uint8_t*)0x20001a6b = 0x13;
  *(uint8_t*)0x20001a6c = 6;
  *(uint8_t*)0x20001a6d = 4;
  *(uint8_t*)0x20001a6e = 0x24;
  *(uint8_t*)0x20001a6f = 1;
  *(uint8_t*)0x20001a70 = 2;
  *(uint8_t*)0x20001a71 = 5;
  *(uint8_t*)0x20001a72 = 0x24;
  *(uint8_t*)0x20001a73 = 0;
  *(uint16_t*)0x20001a74 = 0x7ff;
  *(uint8_t*)0x20001a76 = 5;
  *(uint8_t*)0x20001a77 = 0x24;
  *(uint8_t*)0x20001a78 = 0;
  *(uint16_t*)0x20001a79 = 5;
  *(uint8_t*)0x20001a7b = 0xd;
  *(uint8_t*)0x20001a7c = 0x24;
  *(uint8_t*)0x20001a7d = 0xf;
  *(uint8_t*)0x20001a7e = 7;
  *(uint32_t*)0x20001a7f = 0;
  *(uint16_t*)0x20001a83 = 3;
  *(uint16_t*)0x20001a85 = 0x7fff;
  *(uint8_t*)0x20001a87 = 0;
  *(uint8_t*)0x20001a88 = 3;
  *(uint8_t*)0x20001a89 = 0x24;
  *(uint8_t*)0x20001a8a = 6;
  *(uint8_t*)0x20001a8b = 6;
  *(uint8_t*)0x20001a8c = 0x24;
  *(uint8_t*)0x20001a8d = 7;
  *(uint8_t*)0x20001a8e = -1;
  *(uint16_t*)0x20001a8f = 0;
  *(uint8_t*)0x20001a91 = 0xa;
  *(uint8_t*)0x20001a92 = 0x24;
  *(uint8_t*)0x20001a93 = 7;
  *(uint8_t*)0x20001a94 = 0;
  *(uint16_t*)0x20001a95 = 0x20;
  *(uint16_t*)0x20001a97 = 0;
  *(uint16_t*)0x20001a99 = 2;
  *(uint8_t*)0x20001a9b = 7;
  *(uint8_t*)0x20001a9c = 5;
  *(uint8_t*)0x20001a9d = 1;
  *(uint8_t*)0x20001a9e = 0;
  *(uint16_t*)0x20001a9f = 1;
  *(uint8_t*)0x20001aa1 = 0;
  *(uint8_t*)0x20001aa2 = 0;
  *(uint8_t*)0x20001aa3 = 8;
  *(uint8_t*)0x20001aa4 = 7;
  *(uint8_t*)0x20001aa5 = 5;
  *(uint8_t*)0x20001aa6 = -1;
  *(uint8_t*)0x20001aa7 = 0;
  *(uint16_t*)0x20001aa8 = 1;
  *(uint8_t*)0x20001aaa = 9;
  *(uint8_t*)0x20001aab = 0x67;
  *(uint8_t*)0x20001aac = 0;
  *(uint8_t*)0x20001aad = 0x52;
  *(uint8_t*)0x20001aae = 0x11;
  memcpy((void*)0x20001aaf,
         "\xb7\x73\xf3\xe0\x0b\x8a\x0a\x42\xea\xba\x87\x25\x3e\x6f\x80\xa3\xb4"
         "\xa4\xad\x87\x78\x39\x43\x4f\xab\xa3\x1a\xe9\xa7\x9a\x5b\x6f\x5b\x71"
         "\x43\xf8\xfd\x9b\xf8\x17\xef\xfd\x62\xd9\xae\x89\xca\x07\xe6\x8f\xdc"
         "\x47\x8d\x4b\xae\xf9\xe5\x12\x0f\xfe\x2e\x37\x37\xad\x3a\xf7\xf7\xb0"
         "\x0a\x9f\x26\x42\x70\x61\x90\xcc\x26\x6c\xaf\x46",
         80);
  *(uint8_t*)0x20001aff = 0xd3;
  *(uint8_t*)0x20001b00 = 0xb;
  memcpy((void*)0x20001b01,
         "\xab\x66\xd5\x75\xcd\xad\x4f\x20\xdc\x0d\x15\x2a\xfc\x57\xef\x7d\xdb"
         "\xc8\x9e\x5d\xb2\x0e\xe0\x4c\xb3\xc0\x42\x2a\x1e\x7b\xae\x6d\xcb\x05"
         "\xed\x3a\x08\x3c\x3c\x4e\x19\xbc\x86\x91\x42\x24\xc0\xf2\x48\xb5\x02"
         "\x41\xd3\xb3\x67\x01\xf0\xb6\xcc\x11\x5a\x68\x89\xfe\x5b\x7e\x02\x04"
         "\x3f\x31\x3a\x31\xf3\xf3\x6a\x85\x65\x11\x9d\x40\x0c\x7f\xf4\x82\x8c"
         "\x68\x3f\x50\xeb\xf8\x69\xc1\x3e\xe3\x88\x0b\xcb\x8a\x00\xa9\x5d\xae"
         "\xb1\xaa\xbd\x65\xc8\xfb\x39\x5a\x49\x92\xce\x3e\x17\x98\xcb\x2f\xa9"
         "\x4c\x20\x6d\x7e\x47\xf1\x8b\x84\xa3\x2a\xd9\x6a\xa3\xfd\x3a\x83\xc9"
         "\xf8\x3f\xd7\x6d\xbd\xe3\xa2\x34\x68\x1b\x87\x62\x33\x81\xb7\x13\x55"
         "\xb7\x7f\x82\xb6\x8a\x88\x86\x5b\x33\x09\x04\x12\xbe\xdf\x07\xf7\xf2"
         "\xb4\xdf\x6c\xaa\x44\x65\x71\x62\x84\x6e\xa5\xd6\x47\xfe\xc1\x4d\x01"
         "\x73\x4d\x95\xb9\xeb\xbb\xc9\x02\x2f\xe7\x00\x01\x69\x10\xcf\x2b\x06"
         "\x35\x3c\xb4\xf9\xf3",
         209);
  *(uint8_t*)0x20001bd2 = 7;
  *(uint8_t*)0x20001bd3 = 5;
  *(uint8_t*)0x20001bd4 = 8;
  *(uint8_t*)0x20001bd5 = 0;
  *(uint16_t*)0x20001bd6 = 5;
  *(uint8_t*)0x20001bd8 = 0;
  *(uint8_t*)0x20001bd9 = 9;
  *(uint8_t*)0x20001bda = 1;
  *(uint8_t*)0x20001bdb = 0x86;
  *(uint8_t*)0x20001bdc = 0x3f;
  memcpy((void*)0x20001bdd,
         "\xb2\xf6\x65\xee\x31\x02\x97\x90\x07\xc0\x01\x7d\x82\x8b\x9b\x0d\xde"
         "\x38\x31\x58\x29\x57\x87\x20\x70\x51\x78\x8e\xc5\x44\x6e\xaa\x6a\x0d"
         "\x76\xa9\x69\x8d\xa9\xd1\xde\xee\xce\xd4\xb7\xed\x87\x1b\x41\x46\x5a"
         "\x68\xd3\x0e\x85\x1f\x80\x2d\x8e\x79\xf7\xf3\x00\x42\xaf\x28\x76\x85"
         "\x9a\xb6\xe8\xfa\x79\xb9\xd9\x19\xca\xe9\x26\xea\xc2\x97\x92\x24\xbe"
         "\xf0\x37\xb5\x01\x2f\x6f\x52\xf9\x70\xcd\x64\xbe\x64\xe2\xec\x59\xb7"
         "\xed\x42\xf4\xfc\x7f\x46\x1f\xa5\x27\xab\xe7\xb4\xcf\x64\xc6\xfe\x73"
         "\x4a\xc7\xd8\x58\xee\x45\xbf\xea\x9f\xc7\x5d\xe6\xe2",
         132);
  *(uint8_t*)0x20001c61 = 0xf1;
  *(uint8_t*)0x20001c62 = 0xa;
  memcpy((void*)0x20001c63,
         "\x6e\xe7\xd3\x53\x8b\xcb\x46\x5f\x67\x01\x65\xb5\x94\xe8\xa3\x7d\x57"
         "\xb5\xc3\xd0\x71\xfa\x53\x6a\xd5\x3d\xdc\x11\xce\x1f\x9f\xe8\x63\x06"
         "\x3b\x4a\x9e\x30\x8c\xa1\x51\x2f\xe8\xbc\x5d\x0f\x2f\x47\x3a\x1a\x4b"
         "\x64\xfa\xbb\x06\xd4\x4e\x60\x7d\xdc\xe8\x38\xbc\xab\x48\xf1\x49\x1f"
         "\xeb\x49\x01\x0f\xc0\x41\x8b\xe3\xf0\xd6\xff\xf1\x49\x3e\x7f\xd8\xe5"
         "\x48\xbd\xa1\xb5\x8d\xf2\xa5\x55\x23\x63\x96\xe9\xe5\x79\x9c\x2c\x11"
         "\x6e\xe2\x13\x09\xb3\xa7\xf3\xe3\x02\x82\xa2\x44\x1b\xff\x57\xc2\x38"
         "\x3c\xf4\xde\xe7\x36\x15\x14\xe4\x81\x44\x4c\x72\xe7\xb8\xa3\x3c\x26"
         "\xea\x46\xfa\xbb\x0c\xda\x7a\xc1\x48\x26\xbc\xf1\x0b\x3f\xc2\x82\xe5"
         "\xb9\xd1\x63\x9a\xd6\xcc\x14\x6b\x61\x36\x5f\x9f\xde\x05\xdb\x50\x6f"
         "\xa6\x05\x86\xc1\xc0\xdc\x50\xa7\x82\x40\xb7\x8f\x02\xd0\x23\xa1\xf6"
         "\x1e\xbc\x7e\x05\x92\x3d\x8c\xa7\x2e\x2b\xae\x7d\xa7\x20\x26\x8f\x3a"
         "\xdd\x2a\x6a\xdb\x4a\xa5\x3f\x36\x98\x6c\x18\x26\xa5\x55\x4c\x8f\x9c"
         "\x9d\x8a\x76\xfd\x1d\xa1\x1a\x4c\xdb\x7a\xe9\x60\x39\xdb\xd6\x93\xb6"
         "\xe0",
         239);
  *(uint8_t*)0x20001d52 = 7;
  *(uint8_t*)0x20001d53 = 5;
  *(uint8_t*)0x20001d54 = 0;
  *(uint8_t*)0x20001d55 = 0x10;
  *(uint16_t*)0x20001d56 = 0x3f;
  *(uint8_t*)0x20001d58 = 0xbf;
  *(uint8_t*)0x20001d59 = 0xbb;
  *(uint8_t*)0x20001d5a = 6;
  *(uint8_t*)0x20001d5b = 0x9a;
  *(uint8_t*)0x20001d5c = 0x31;
  memcpy((void*)0x20001d5d,
         "\x57\xba\xcf\x8b\x63\x11\x13\xeb\xa7\xbd\x89\xf0\xf5\xc4\x64\x65\xd9"
         "\xcf\xe6\xda\x1d\x1d\x4e\x13\x21\x74\xcb\x46\x8c\x35\x97\x99\x4d\xe0"
         "\x34\xb7\xf4\x88\xe8\x0f\xd8\xa4\xe8\xcb\xf5\x22\x60\x84\x02\x0e\xa9"
         "\xa9\x0f\xab\x26\x50\x55\x77\x78\x9b\x96\x3f\x59\xc8\x48\x00\x4c\xc6"
         "\x2e\x0d\xc9\x40\xa3\xef\x84\xc2\x49\xd0\x24\x20\x2b\x5f\x98\x30\x27"
         "\x0e\x71\xab\xc6\xc9\x6a\x9c\x8d\xf2\xe3\x90\xf0\x99\x9e\x92\x3a\x03"
         "\x26\xed\x07\x94\x60\x23\x23\xf7\x4a\x76\x17\xa9\x9e\x22\x4d\xa8\x78"
         "\x98\x53\x27\x1b\x93\x71\xa8\x05\xa5\xaf\x73\x7f\xff\xe5\x77\xd6\xdc"
         "\xaa\xa8\x49\x28\x26\x01\xdb\x6a\xc8\xb8\x21\x6d\xca\x56\x1f\x87",
         152);
  *(uint8_t*)0x20001df5 = 7;
  *(uint8_t*)0x20001df6 = 5;
  *(uint8_t*)0x20001df7 = 1;
  *(uint8_t*)0x20001df8 = 0x10;
  *(uint16_t*)0x20001df9 = 5;
  *(uint8_t*)0x20001dfb = 0;
  *(uint8_t*)0x20001dfc = 0;
  *(uint8_t*)0x20001dfd = -1;
  *(uint8_t*)0x20001dfe = 7;
  *(uint8_t*)0x20001dff = 5;
  *(uint8_t*)0x20001e00 = -1;
  *(uint8_t*)0x20001e01 = 0xd;
  *(uint16_t*)0x20001e02 = 0xff;
  *(uint8_t*)0x20001e04 = 7;
  *(uint8_t*)0x20001e05 = 0xe;
  *(uint8_t*)0x20001e06 = 4;
  *(uint8_t*)0x20001e07 = 7;
  *(uint8_t*)0x20001e08 = 5;
  *(uint8_t*)0x20001e09 = 7;
  *(uint8_t*)0x20001e0a = 0x13;
  *(uint16_t*)0x20001e0b = 0;
  *(uint8_t*)0x20001e0d = 0;
  *(uint8_t*)0x20001e0e = 5;
  *(uint8_t*)0x20001e0f = 1;
  *(uint8_t*)0x20001e10 = 2;
  *(uint8_t*)0x20001e11 = 0x22;
  *(uint8_t*)0x20001e12 = 2;
  *(uint8_t*)0x20001e13 = 3;
  *(uint8_t*)0x20001e14 = 7;
  *(uint8_t*)0x20001e15 = 5;
  *(uint8_t*)0x20001e16 = 3;
  *(uint8_t*)0x20001e17 = 0x1c;
  *(uint16_t*)0x20001e18 = 6;
  *(uint8_t*)0x20001e1a = 0xc1;
  *(uint8_t*)0x20001e1b = -1;
  *(uint8_t*)0x20001e1c = -1;
  *(uint8_t*)0x20001e1d = 0xa5;
  *(uint8_t*)0x20001e1e = 9;
  memcpy((void*)0x20001e1f,
         "\x77\x51\xa2\xfc\xf2\x7d\x82\x5c\xbb\x42\x5e\x75\x88\x85\xc7\x1e\x50"
         "\x5e\xd8\xe1\x8c\x1f\x10\x68\xfa\xdd\x73\xfe\xd5\xc9\x13\x6e\x02\x59"
         "\x03\x21\x48\xa7\x19\x98\xa9\xfd\x5e\xdc\x44\x59\xc8\x4d\x5b\xcf\xce"
         "\xda\x14\x92\xe9\xd3\x56\xcc\x65\x17\xd7\x76\xe3\xd8\x58\x0b\x12\x10"
         "\xee\xd4\xe7\x7e\x62\x00\x2c\x6e\x20\x35\x26\x31\xd6\xba\x9c\x59\xe0"
         "\x07\x7a\xd9\x82\xf2\x14\xb2\xb7\x2d\x71\xb9\xfc\x1a\xb7\xaa\xab\x71"
         "\xf2\x32\x8f\xeb\xdc\xb9\x87\xd2\xd6\x34\x99\x81\x7f\xdb\x12\x91\x20"
         "\xe0\x59\x13\xcc\x12\xa2\x16\x3d\x6e\x85\xc2\x3c\x9a\xdb\x55\x69\xce"
         "\xca\x3e\x96\xf3\xe8\x1f\x26\xa3\x53\x9d\x45\xaf\xe2\xf4\x19\x58\x44"
         "\x9f\x77\x10\x28\x76\x84\x0d\xdd\xef\x4f",
         163);
  syz_usb_connect(6, 0x482, 0x20001a40, 0);
  syz_usb_control_io(r[0], 0, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
