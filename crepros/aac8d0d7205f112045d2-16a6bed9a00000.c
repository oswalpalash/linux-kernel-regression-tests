// https://syzkaller.appspot.com/bug?id=69bf3422c0eb7a37dec8c1a6c2d56ea40bf6bacf
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
    rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_event*)&event);
    if (rv < 0) {
      return rv;
    }
    if (event.inner.type != USB_FUZZER_EVENT_CONTROL)
      continue;
    bool response_found = false;
    char* response_data = NULL;
    uint32_t response_length = 0;
    response_found = lookup_connect_response(
        descs, &index, &event.ctrl, &response_data, &response_length, &done);
    if (!response_found) {
      return -1;
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
    rv = usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
    if (rv < 0) {
      return rv;
    }
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
  event.inner.length = USB_MAX_PACKET_SIZE;
  int rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_event*)&event);
  if (rv < 0) {
    return rv;
  }
  if (event.inner.type != USB_FUZZER_EVENT_CONTROL) {
    return -1;
  }
  if (!(event.ctrl.bRequestType & USB_DIR_IN) && event.ctrl.wLength != 0) {
  }
  bool response_found = false;
  char* response_data = NULL;
  uint32_t response_length = 0;
  response_found = lookup_control_io_response(descs, resps, &event.ctrl,
                                              &response_data, &response_length);
  if (!response_found) {
    return -1;
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
  rv = usb_fuzzer_ep0_write(fd, (struct usb_fuzzer_ep_io*)&response);
  if (rv < 0) {
    return rv;
  }
  sleep_ms(200);
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
  *(uint8_t*)0x20000040 = 0x12;
  *(uint8_t*)0x20000041 = 1;
  *(uint16_t*)0x20000042 = 0;
  *(uint8_t*)0x20000044 = 0x58;
  *(uint8_t*)0x20000045 = 0xad;
  *(uint8_t*)0x20000046 = 0xba;
  *(uint8_t*)0x20000047 = 8;
  *(uint16_t*)0x20000048 = 0x2040;
  *(uint16_t*)0x2000004a = 0x4982;
  *(uint16_t*)0x2000004c = 0xf9cf;
  *(uint8_t*)0x2000004e = 0;
  *(uint8_t*)0x2000004f = 0;
  *(uint8_t*)0x20000050 = 0;
  *(uint8_t*)0x20000051 = 1;
  *(uint8_t*)0x20000052 = 9;
  *(uint8_t*)0x20000053 = 2;
  *(uint16_t*)0x20000054 = 0x1b;
  *(uint8_t*)0x20000056 = 1;
  *(uint8_t*)0x20000057 = 0;
  *(uint8_t*)0x20000058 = 0;
  *(uint8_t*)0x20000059 = 0;
  *(uint8_t*)0x2000005a = 0;
  *(uint8_t*)0x2000005b = 9;
  *(uint8_t*)0x2000005c = 4;
  *(uint8_t*)0x2000005d = 0xc8;
  *(uint8_t*)0x2000005e = 0;
  *(uint8_t*)0x2000005f = 1;
  *(uint8_t*)0x20000060 = 0;
  *(uint8_t*)0x20000061 = 0xcb;
  *(uint8_t*)0x20000062 = 0x3f;
  *(uint8_t*)0x20000063 = 0;
  *(uint8_t*)0x20000064 = 7;
  *(uint8_t*)0x20000065 = 5;
  *(uint8_t*)0x20000066 = 0x84;
  *(uint8_t*)0x20000067 = 2;
  *(uint16_t*)0x20000068 = 0;
  *(uint8_t*)0x2000006a = 0;
  *(uint8_t*)0x2000006b = 0;
  *(uint8_t*)0x2000006c = 0;
  res = syz_usb_connect(0, 0x2d, 0x20000040, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000ec0 = 0x24;
  *(uint64_t*)0x20000ec4 = 0;
  *(uint64_t*)0x20000ecc = 0;
  *(uint64_t*)0x20000ed4 = 0;
  *(uint64_t*)0x20000edc = 0;
  *(uint32_t*)0x200010c0 = 0x24;
  *(uint64_t*)0x200010c4 = 0x20000f00;
  *(uint8_t*)0x20000f00 = 0x60;
  *(uint8_t*)0x20000f01 = 0x31;
  *(uint32_t*)0x20000f02 = 0xdd;
  memcpy((void*)0x20000f06,
         "\xe2\xc8\x00\x37\x02\x8a\xd4\xcc\x4f\xa1\x5b\x33\x89\xc8\x5b\x7b\x0a"
         "\x06\x09\xc4\x6c\x1f\x28\x42\x8c\x8c\x07\xdb\xe3\x8e\x69\x2a\x25\x5a"
         "\x2d\x59\x01\xa7\xc5\x50\xc9\x97\xe1\x15\xd1\x43\xf5\x2e\xfb\xd0\xfb"
         "\x22\x47\xfe\x7e\x94\x97\x77\x1d\x93\xce\xb5\xef\xff\xec\xa8\xad\x39"
         "\xd8\xc9\x3a\xc4\x7e\x24\xf7\x1a\x10\xd6\x5a\xbe\x98\xb6\xb7\x10\x65"
         "\x30\xaa\x48\x90\x27\x98\x49\x1a\x5b\x5a\x37\x0c\x5c\x18\x50\x25\x4a"
         "\x80\x9e\xdf\x24\x9a\xa6\xdb\xe7\xdb\x44\xac\x86\xa0\x94\xb7\xee\x69"
         "\xcd\x9c\x46\xb1\xbd\x41\x44\xdb\x66\x81\x17\x49\x21\x6c\xfe\x88\xf8"
         "\x07\xe5\x97\xb2\xf0\xea\xa6\x24\xe0\xc0\xc7\xa5\xd1\x8a\x91\xc8\x2f"
         "\x6c\x1b\xd0\x27\xd2\xa8\xad\x0f\x9a\xd9\x89\x4a\x96\x6b\x1c\x17\x18"
         "\xdd\x08\xc3\xe2\x31\xd5\xe6\xfa\x9e\x1e\xbf\x8c\x4c\x9e\x9a\xde\x27"
         "\xcc\xd3\x83\xc1\x47\x10\x5d\x18\x72\x42\xe6\x29\x11\xa5\x43\xee\x58"
         "\x97\xa2\xd4\xfb\x8d\xc7\x97\xda\x56\x9c\x98\xbd\x49\xdf\xea\x5f\xb1",
         221);
  *(uint64_t*)0x200010cc = 0;
  *(uint64_t*)0x200010d4 = 0;
  *(uint64_t*)0x200010dc = 0;
  syz_usb_control_io(r[0], 0x20000ec0, 0x200010c0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
