// https://syzkaller.appspot.com/bug?id=0141bd6b37153edec9c4ffa0f0e990c7228897f9
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

#define USB_DEBUG 0

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

static const char* default_string = "syzkaller";

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
        if (str_idx >= descs->strs_len) {
          *response_data = (char*)default_string;
          *response_length = strlen(default_string);
        } else {
          *response_data = descs->strs[str_idx].str;
          *response_length = descs->strs[str_idx].len;
        }
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

static bool lookup_control_response(struct vusb_descriptors* descs,
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
  int rv = usb_fuzzer_event_fetch(fd, (struct usb_fuzzer_event*)&event);
  if (rv < 0) {
    return rv;
  }
  if (event.inner.type != USB_FUZZER_EVENT_CONTROL) {
    return -1;
  }
  bool response_found = false;
  char* response_data = NULL;
  uint32_t response_length = 0;
  if (event.ctrl.bRequestType & USB_DIR_IN) {
    response_found = lookup_control_response(descs, resps, &event.ctrl,
                                             &response_data, &response_length);
    if (!response_found) {
      return -1;
    }
  } else {
    response_length = event.ctrl.wLength;
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
  sleep_ms(200);
  return 0;
}

static volatile long syz_usb_disconnect(volatile long a0)
{
  int fd = a0;
  int rv = close(fd);
  sleep_ms(200);
  return rv;
}

static long syz_open_dev(volatile long a0, volatile long a1, volatile long a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block", (uint8_t)a1,
            (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy(
      (void*)0x20000400,
      "\x12\x01\x00\x00\x00\x00\x00\x20\x6d\x04\x1c\xc7\x40\x00\x00\x00\x00\x01"
      "\x09\x02\x24\x00\x01\x00\x00\xa0\x00\x09\x04\x00\x00\x09\x03\x01\x01\x00"
      "\x09\x21\x00\x00\x00\x01\x22\x15\x00\x09\x05\x81\x03\x00\x00\x00\x00\x00"
      "\xee\xb4\x88\x8f\xff\xfc\x1f\xe0\x2d\xa3\x65\xa9\xc4\xce\xfe\x8b\x90\x61"
      "\x70\x1c\x3c\xcd\x35\x2d\x15\x83\x2b\xe2\x59\x85\x90\x20\x08\x5b\x4a\x55"
      "\x25\xda\xaa\xfd\xc4\x9f\x59\x66\xf9\x00\x63\x4c\x11\xc5\x62\x10\x23\x5d"
      "\xca\x74\xb0\x5a\xbe\xdf\x3b\x46\xe5\xd9\x29\x2b\x05\x00\xe1\x8c\x4a\x00"
      "\x00\x00\x00\x9c\xc7\x04\xcc\x79\x15\x55\x97\x94\x80\x5a\x7a\xb1\x3d\x44"
      "\x84\x90\xdb\xac\xf2\x68\xac\x24\x72\x3a\xcf\x3c\xa4\x84\x0a\xf0\x39\xb7"
      "\xf9\xb8\x36\xbd\xe4\x66\xbe\x4d\x32\x35\x99\x80\x1b\x61\xa4\xa4\xfd\x48"
      "\x44\xf9\xeb\xc1\xd2\xc0\x99\xd1\x74\x7d\x8a\x91\xb7\x62\xf8\x21\x02\x49"
      "\x1b\x72\x36\x20\xa3\xb8\x27\x0a\x84\x5f\xf9\xf1\xc3\xb2\x59\x2b\x1c\x7b"
      "\xf2\x32\x64\xe0\x73\x61\x9b\x76\x05\xc4\x6f\xe4\x0f\xa7\xc3\xf5\x04\xd8"
      "\xc8\xea\x34\xf3\xb2\x2e\x92\x00\x55\xf2\x4e\x2e\xb6\x78\xb7\x27\xfb\x1a"
      "\xc5\xfd\x57\x81\x44\xe9\x37\xfd\x37\xa0\xd3\xc6\x04\x6a\xbd\xe8\x52\x6b"
      "\x5f\xca\xf1\x40\xe7\xf4\x81\x2a\x08\x24\xfa\x5f\x4d\x83\x94\x8b\xa6\x68"
      "\xf5\x6a\xe3\xe8\x62\x0d\x99\xb9\xc1\xf7\x85\x82\x65\x28\x78\x50\x84\x94"
      "\xe0\xc4\x14\xf9\x8f\x00\x11\xe4\x80\x59\xb8\x75\x82\x9a\x88\x8f\xe1\x7b"
      "\x21\xdf\xa2\x20\x06\xeb\xc7\x72\x17\x4d\x01\x54\x90\xa6\x44\x73\xe4\xc9"
      "\x35\x70\x45\x01\x30\x1c\xf5\x2e\xd2\x74\x04\xac\x30\xd1\x8c\xbb\xee\xe2"
      "\x4d\x2b\x8f\x6e\x05\x9c\x28\x8d\x50\xcc\x67\xf3\x86\x7c\xe5\x40\x31\xe1"
      "\x90\x5d\xc7\xc5\xea\xa2\x6b\xe3\xf4\x42\xf0\x3b\x58\xa2\x86\xa0\xfd\xf0"
      "\xf9\xf5\x56\xb7\x2c\xe8\x9f\x13\xbc\xd8\xe8\x93\xda\x40\x0a\xd1\x31\x8d"
      "\x76\xa8\x62\xa7\x6f\xb7\xa1\x49\xaf\x39\x37\x9d\x9f\x4e\xdf\x33\xe0\x83"
      "\xb8\x26\x9d\xa2\x4c\xc9\xf0\x90\xff\xe7\xeb\x07\x8a\x60\x50\x1f\x47\x43"
      "\x95\x46\x5e\x68\x7f\x67\xcf\xef\x7b\xc9\x04\x42\xc4\x2d\x43\xd1\x08\x41"
      "\x3a\x24\xdc\x94\x25\xf0\xb3\x26\xc2\xf3\x75\x9c\xb7\x96\xfa\x6a\x86\x75"
      "\x57\x55\x08\x49\xb6\xc9\xc5\xa3\x4b\x7a\x64\x98\xeb\xd5\x0e\x4e\x0c\xe1"
      "\xda\x1d\x05\x8d\xeb\x3d\x42\x44\x73\x32\xcb\xbd\xe6\xec\xc1\x23\xcc\x2e"
      "\x4d\x4d\x3e\x50\x2b\xdd\xa0\xdb\xc9\x54\x78\xa0\x2c\xef\xc6\xdf\x24\x2c"
      "\xca\x03\x8f\xc4\x38\x36\x94\xde\x69\x8b\xec\xde\xeb\xdc\xe5\x9a\x82\xff"
      "\x59\x44\x83\x31\x9b\x51\xea\xed\xd1\x51\xc3\xf8\x9f\x28\x21\x22\x45\xf0"
      "\xc8\xa4\x1d\x0d\x95\x7b\x4c\x11\xb9\x04\xab\x8a\x11\x46\x43\x88\x31\x63"
      "\xb1\x68\x27\x5d\xc3\x8b\x0d\x9d\x64\xfc\xc9\xd1\x62\x3f\xcb\xf0\x7e\x5a"
      "\x33\x6a\x0f\x00\x00\x00\x00\x00\x00\x0a\x37\x25\x33\x11\x27\x6f\xf7\x04"
      "\x39\xc7\x6d\xef\xb6\x22\x9b\xb9\x59\xb0\x07\xf3\x41\xce\x76\x6c\x42\x91"
      "\x20\x98\x57\x70\x75\x9f\x34\xda\x7c\x1c\x5f\x0b\xbc\x9c\xac\xbf\x7f\xe7"
      "\x09\x00\x00\x00\xc5\x5a\x93\xdc\xf8\x42\x86\x4e\x00\x00\x00",
      681);
  res = syz_usb_connect(0, 0x36, 0x20000400, 0);
  if (res != -1)
    r[0] = res;
  syz_usb_control_io(r[0], 0, 0);
  *(uint32_t*)0x20000340 = 0x34;
  *(uint64_t*)0x20000344 = 0;
  *(uint64_t*)0x2000034c = 0;
  *(uint64_t*)0x20000354 = 0;
  *(uint64_t*)0x2000035c = 0x20000380;
  memcpy((void*)0x20000380,
         "\x00\x22\x15\x00\x00\x00\x8e\x03\xdd\x03\x4f\x4e\x37\x85\x15\xe0\x81",
         17);
  *(uint32_t*)0x20001bc0 = 0xcc;
  *(uint64_t*)0x20001bc4 = 0;
  *(uint64_t*)0x20001bcc = 0;
  *(uint64_t*)0x20001bd4 = 0;
  *(uint64_t*)0x20001bdc = 0;
  *(uint64_t*)0x20001be4 = 0;
  syz_usb_control_io(r[0], 0x20000340, 0x20001bc0);
  memcpy((void*)0x200002c0, "/dev/hidraw#\000", 13);
  res = syz_open_dev(0x200002c0, 0, 1);
  if (res != -1)
    r[1] = res;
  syz_usb_disconnect(r[0]);
  syscall(__NR_ioctl, r[1], 0x80404805, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
