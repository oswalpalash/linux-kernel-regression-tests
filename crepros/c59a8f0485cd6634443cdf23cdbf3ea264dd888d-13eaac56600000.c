// https://syzkaller.appspot.com/bug?id=c59a8f0485cd6634443cdf23cdbf3ea264dd888d
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

void execute_one(void)
{
  memcpy(
      (void*)0x20000840,
      "\x12\x01\x00\x00\x12\x09\x1f\x08\xc0\x07\x01\x15\xa0\x74\x00\x00\x00\x01"
      "\x09\x02\x1b\x00\x03\x00\x00\x00\x00\x09\x04\xec\x00\x01\x50\x6d\x30\x00"
      "\x07\x05\x81\x8a\x7c\x29\x84\x63\x89\x56\xd3\x2d\x01\xd3\xb0\xf0\x96\x58"
      "\x85\xfc\xec\x09\xba\x0e\x36\x54\xbb\xa6\x49\x2e\x7c\xa6\xfa\xc3\x13\x6c"
      "\x3b\x2e\x10\x75\xed\xf8\xd5\x47\xec\x1b\x71\x59\xb2\x29\x1f\xdb\x69\x9f"
      "\x0c\xbf\x95\x71\x03\x33\xa1\x72\xe5\xe1\x6f\xff\xfb\xe2\x16\x71\x93\x87"
      "\xfa\x36\x13\x78\x5b\x28\x24\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00\x00\x00\x00\x00\x00\x00\x7f\xd9\x02\x94\x2a\xa4\x2a\x2d\x0a\xca"
      "\xf3\xb8\x43\xfd\x3c\xeb\xb0\x28\x4c\x68\x04\xe1\x00\x3b\x25\xbb\x66\x62"
      "\x04\xf7\x28\xd2\x2c\x4c\xcf\x6b\xbc\x17\x07\x3a\x1f\x00\x00\x00\xae\xb1"
      "\x5e\xf1\xbe\xe1\x60\x19\xd7\xdc\xa8\xfb\x7d\xdc\x07\x25\xc6\x68\x24\x2c"
      "\x2a\x18\xa9\x00\x00\x00\x00\x10\xef\xb8\x52\x4a\x3b\x51\x43\xe5\x66\x73"
      "\xc4\x27\xe1\x4c\x0a\xb0\xbe\x41\xf1\x6c\x9d\x54\x7c\xfe\xdb\x3e\x3f\xcb"
      "\x1e\xa4\xd0\xc3\x1c\x3c\xd9\x93\xfc\x4f\x13\x34\x8c\x42\xe1\xb2\x08\x00"
      "\x00\x00\x3b\x52\xe2\x12\xf4\x59\xa4\xca\xb3\xa1\x19\x0b\x8a\x94\x4a\x07"
      "\xc4\xba\x5c\x3d\x2d\xef\x05\x5f\x97\xce\xfb\xe1\x6f\xed\xbf\xbe\xa5\xc1"
      "\x92\x0e\x96\xef\xb4\xcf\x8b\x93\x2a\xd9\x71\xfd\x3f\x42\x9c\xdd\x75\x95"
      "\xa4\x76\xc7\x72\x54\x4b\xa8\xa5\xf2\x0d\x14\xf6\x6a\x07\x9d\x94\xed\x38"
      "\x44\xc7\x36\x04\xbd\x09\xb0\x2f\xe1\x94\x75\xb8\xaa\x89\x9d\xfb\xe1\xa2"
      "\x00\x00\x00\x00\x00\x00\x00\x19\x67\x11\xea\xe7\x47\x86\x65\xf0\x86\xb2"
      "\x65\xf3\x0e\x57\x1e\xfa\xdf\xf0\xd1\x60\xba\x67\xdf\x4c\xc3\xb2\x21\x21"
      "\x25\x35\xee\xcb\x6a\xc6\x18\xf2\xd2\x04\xb7\xac\xec\x54\xcd\x67\xa0\x06"
      "\x2a\x14\x46\xf8\xff\xff\xff\x57\xe1\xf6\x15\xa3\xaf\x4e\x38\x89\x23\x78"
      "\x3f\x70\x7e\x63\x9a\xa3\xee\xc3\x12\x96\x34\x91\x0b\xf3\x09\xb6\x9b\xb5"
      "\xdf\x6c\x2e\xf4\x20\x17\xbe\x56\xda\x68\x44\x90\x85\x49\x74\x6c\xe4\xbe"
      "\x41\xec\xce\x5d\x57\x64\x57\xa1\xb1\x8a\xdd\x7f\xc1\x7e\x5a\xab\x45\xea"
      "\x89\xb3\xcf\xdf\xfc\xb6\x25\xe1\x12\xda\xe9\x51\x15\xcc\x3c\x10\xc3\x3d"
      "\xae\xac\xac\xde\x3d\x2b\xab\xcf\x10\xb1\xf6\x7a\x1c\x8d\xb1\x71\x53\x18"
      "\xd4\xb6\x50\xca\xff\x54\x06\x64\xe1\x67\x9c\x53\x55\x49\xbb\xc1\xbf\x82"
      "\xa3\xb6\x01\xb5\x54\xba\x06\x12\xc1\x90\x97\x71\xfd\xf4\x55\x7e\xe4\xa5"
      "\x38\xb1\xa2\xe5\x0e\x00\x02\x00\x00\xc9\x82\xd7\x79\xb9\x6a\x6a\xb1\x5d"
      "\x5d\x35\xb5\xd7\x88\x9c\xfa\x4f\xf5\xe4\x71\x96\x28\x14\x32\x56\x3b\xa3"
      "\x88\x1d\xfd\x4b\xb6\x51\x7d\x91\x7d\xf8\x34\x35\xf0\x08\x6e\xb4\x27\x5a"
      "\xe4\x6a\xb3\xc3\x61\xb6\x0f\x29\x06\x25\x2c\x7d\x28\x0c\x95\xdb\x70\x4e"
      "\xb6\x08\x1f\xdb\xea\xd5\xea\x50\x33\x71\x1a\xdc\xc7\x5b\x18\xd3\x21\x64"
      "\x8f\x34\xbb\x6a\x26\xa9\x61\xe5\x3a\x15\x1f\x14\x9e\xa0\x7a\x4b\x06\x6e"
      "\xca\x42\x0b\x33\x88\x35\xdf\x8a\xae\x0c\x1c\x3e\x86\x9f\x37\x75\xc6\x9b"
      "\xd1\x03\x0c\x9b\x0f\x85\xfb\x97\xe0\x2f\x81\xa1\x7d\x15\x7a\x6a\x40\x34"
      "\x81\xd5\x1a\x01\x54\x2b\x94\xa9\x13\x6d\x7c\x93\x50\xad\x81\x04\x87\xeb"
      "\x57\x19\xac\xb9\x43\x39\x2e\x4f\x24\x79\xdc\xac\xc2\x2a\xfa\xea\x57\x8c"
      "\xc8\xd1\x23\x04\x9f\x4b\xde\x8a\x79\x24\xbf\xa5\xcd\x95\x7f\x21\xc9\xbf"
      "\xd5\xae\x2d\x6c\xcb\x39\x25\x59\x0a\xd3\x13\x5d\x65\xe8\x34\x38\x7e\x7b"
      "\x90\xcf\x92\x66\x5b\xf4\x41\xca\xea\xf4\x3f\x2a\xbc\x78\x8f\xef\x84\xa7"
      "\x86\x80\xc4\xf5\x6f\x47\x0f\xe9\xb9\x4d\x2f\x08\x23\xcc\xe6\x04\xd8\xc0"
      "\x8b\xba\x1d\x6a\xd0\xa7\x61\x59\x68\x84\xaa\xbb\xcb\x60\x48\x23\xa4\x57"
      "\xd2\x33\xe4\x6c\xf4\x11\xcd\x54\x24\xa5\xa5\x30\xee\x7e\x74\x5c\xe9\xe4"
      "\x3e\xe8\x38\xd5\x96\x54\xcc\x7f\x98\xb6\xce\x47\x8d\xd3\x7f\x4d\xac\xbd"
      "\x8b\x33\xdb\x8d\x1d\x72\xc6\xf9\xc6\x24\xd4\x6d\xde\x83\x81\x47\x1f\x70"
      "\x2d\xf6\xe1\x58\xac\xd6\x4a\x15\x0a\x94\xe4\xd5\x2e\xfa\xaf\xb7\x5a\x77"
      "\x9f\xa4\x2b\xc1\x71\xd1\xb6\x77\xb6\x90\x0a\xca\xd0\x53\xb1\xb8\x34\x66"
      "\xc8\x98\x65\xfa\x7f\x91\x05\x4a\x56\x03\xe6\xb5\x7d\xa9\x01\xb7\xfd\xbd"
      "\x88\xfd\x8a\x14\x55\x7f\x7a\x2a\x6f\xcb\xec\xcf\xd0\x8a\x99\x82\xcc\x12"
      "\xb5\x61\xf6\xed\x07\x0d\x52\xc5\x5b\xfb\xb0\x56\xde\xc6\x8f\x78\xd5\x43"
      "\x45\xf4\x1f\x8f\xb1\xba\x72\x1d\xde\x4e\xaa\x72\x6b\x1b\x64\x23\x57\xd4"
      "\x0e\x78\x8d\x41\x41\xd9\xcc\x97\x8d\xcc\x62\x39\x13\xac\x47\x0b\x74\xfa"
      "\x5c\x34\x52\x9e\x34\x7b\xd1\xfa\xde\x27",
      1000);
  syz_usb_connect(1, 0x2d, 0x20000840, 0);
  syz_open_dev(0xc, 0xb4, 0x10001);
  syz_open_dev(0xc, 0xb4, 0);
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
