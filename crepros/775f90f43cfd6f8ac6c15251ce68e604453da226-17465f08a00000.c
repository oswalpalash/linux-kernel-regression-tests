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

static volatile long syz_usb_connect(volatile long a0, volatile long a1,
                                     volatile long a2, volatile long a3)
{
  int64_t speed = a0;
  int64_t dev_len = a1;
  char* dev = (char*)a2;
  struct vusb_connect_descriptors* conn_descs =
      (struct vusb_connect_descriptors*)a3;
  if (!dev)
    return -1;
  struct usb_device_index index;
  memset(&index, 0, sizeof(index));
  int rv = parse_usb_descriptor(dev, dev_len, &index);
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
    char* response_data = NULL;
    uint32_t response_length = 0;
    unsigned ep;
    uint8_t str_idx;
    struct usb_fuzzer_control_event event;
    event.inner.type = 0;
    event.inner.length = sizeof(event.ctrl);
    rv = usb_fuzzer_ep0_read(fd, (struct usb_fuzzer_event*)&event);
    if (rv < 0)
      return -1;
    if (event.inner.type != USB_FUZZER_EVENT_CONTROL)
      continue;
    switch (event.ctrl.bRequestType & USB_TYPE_MASK) {
    case USB_TYPE_STANDARD:
      switch (event.ctrl.bRequest) {
      case USB_REQ_GET_DESCRIPTOR:
        switch (event.ctrl.wValue >> 8) {
        case USB_DT_DEVICE:
          response_data = (char*)index.dev;
          response_length = sizeof(*index.dev);
          goto reply;
        case USB_DT_CONFIG:
          response_data = (char*)index.config;
          response_length = index.config_length;
          goto reply;
        case USB_DT_STRING:
          str_idx = (uint8_t)event.ctrl.wValue;
          if (str_idx >= conn_descs->strs_len)
            goto reply;
          response_data = conn_descs->strs[str_idx].str;
          response_length = conn_descs->strs[str_idx].len;
          goto reply;
        case USB_DT_BOS:
          response_data = conn_descs->bos;
          response_length = conn_descs->bos_len;
          goto reply;
        case USB_DT_DEVICE_QUALIFIER:
          response_data = conn_descs->qual;
          response_length = conn_descs->qual_len;
          goto reply;
        default:
          exit(1);
          continue;
        }
        break;
      case USB_REQ_SET_CONFIGURATION:
        rv = usb_fuzzer_vbus_draw(fd, index.config->bMaxPower);
        if (rv < 0)
          return -1;
        rv = usb_fuzzer_configure(fd);
        if (rv < 0)
          return -1;
        for (ep = 0; ep < index.eps_num; ep++) {
          rv = usb_fuzzer_ep_enable(fd, index.eps[ep]);
          if (rv < 0)
            exit(1);
        }
        done = true;
        goto reply;
      default:
        exit(1);
        continue;
      }
      break;
    default:
      exit(1);
      continue;
    }
    struct usb_fuzzer_ep_io_data response;
  reply:
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
  uint8_t req = event.ctrl.bRequest;
  uint8_t req_type = event.ctrl.bRequestType & USB_TYPE_MASK;
  uint8_t desc_type = event.ctrl.wValue >> 8;
  char* response_data = NULL;
  uint32_t response_length = 0;
  if (req == USB_REQ_GET_DESCRIPTOR) {
    int i;
    int descs_num = (descs->len - offsetof(struct vusb_descriptors, descs)) /
                    sizeof(descs->descs[0]);
    for (i = 0; i < descs_num; i++) {
      struct vusb_descriptor* desc = descs->descs[i];
      if (!desc)
        continue;
      if (desc->req_type == req_type && desc->desc_type == desc_type) {
        response_length = desc->len;
        if (response_length != 0)
          response_data = &desc->data[0];
        goto reply;
      }
    }
    if (descs->generic) {
      response_data = &descs->generic->data[0];
      response_length = descs->generic->len;
      goto reply;
    }
  } else {
    int i;
    int resps_num = (resps->len - offsetof(struct vusb_responses, resps)) /
                    sizeof(resps->resps[0]);
    for (i = 0; i < resps_num; i++) {
      struct vusb_response* resp = resps->resps[i];
      if (!resp)
        continue;
      if (resp->type == req_type && resp->req == req) {
        response_length = resp->len;
        if (response_length != 0)
          response_data = &resp->data[0];
        goto reply;
      }
    }
    if (resps->generic) {
      response_data = &resps->generic->data[0];
      response_length = resps->generic->len;
      goto reply;
    }
  }
  return -1;
  struct usb_fuzzer_ep_io_data response;

reply:
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

#define SYZ_HAVE_SETUP_TEST 1
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
  long res = 0;
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
  res = syz_usb_connect(8, 0x24, 0x20000000, 0);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x200025c0 = 0x54;
  *(uint64_t*)0x200025c4 = 0x200022c0;
  *(uint8_t*)0x200022c0 = 0x20;
  *(uint8_t*)0x200022c1 = 0xf;
  *(uint32_t*)0x200022c2 = 0x8a;
  memcpy((void*)0x200022c6,
         "\x3d\xfd\x24\x56\xb2\x56\xc2\x7c\xbf\x90\x63\x92\x2c\x45\xd7\x24\x3e"
         "\xcd\xf3\x76\xb1\x19\x88\xcf\x9d\x6a\x0a\x5a\x48\x85\xa1\x61\x5f\x80"
         "\x0f\x1e\x23\x28\xa6\x3c\xe2\x39\x1b\x1c\x00\x5b\x06\xd3\x2d\xa1\x78"
         "\x85\x0f\x98\x4e\x7e\x2a\xc9\xaf\xc2\x20\x41\xbd\x36\x9e\x63\x8f\x43"
         "\x55\x98\x9c\xb5\x5a\xbe\x24\x17\x6b\x57\x54\x44\x9c\x01\x4b\xe0\x8d"
         "\x5a\xf8\x58\xd5\x17\xd1\x7e\x21\x1e\x71\xe4\xd5\xc0\xea\xd0\x7b\x5a"
         "\xfc\x6e\x8a\xb2\xd4\x35\x6a\x86\xc4\xbe\xb6\xa4\xb8\x05\xd4\x80\x7a"
         "\x66\xee\x1f\x82\x62\x87\x4c\x0f\x40\xa2\xca\xfe\x4f\x9f\xad\x2c\xfe"
         "\xd8\x7e",
         138);
  *(uint64_t*)0x200025cc = 0;
  *(uint64_t*)0x200025d4 = 0x200023c0;
  *(uint8_t*)0x200023c0 = 0x20;
  *(uint8_t*)0x200023c1 = 0xa;
  *(uint32_t*)0x200023c2 = 1;
  *(uint8_t*)0x200023c6 = 0;
  *(uint64_t*)0x200025dc = 0;
  *(uint64_t*)0x200025e4 = 0;
  *(uint64_t*)0x200025ec = 0;
  *(uint64_t*)0x200025f4 = 0;
  *(uint64_t*)0x200025fc = 0;
  *(uint64_t*)0x20002604 = 0;
  *(uint64_t*)0x2000260c = 0x20002580;
  *(uint8_t*)0x20002580 = 0x20;
  *(uint8_t*)0x20002581 = 0x80;
  *(uint32_t*)0x20002582 = 0x1c;
  *(uint16_t*)0x20002586 = 6;
  *(uint16_t*)0x20002588 = 0;
  *(uint32_t*)0x2000258a = 7;
  *(uint16_t*)0x2000258e = 0x40;
  *(uint16_t*)0x20002590 = 0;
  *(uint16_t*)0x20002592 = 0x7fff;
  *(uint16_t*)0x20002594 = 3;
  *(uint32_t*)0x20002596 = 5;
  *(uint16_t*)0x2000259a = 0x1ff;
  *(uint16_t*)0x2000259c = 0x8000;
  *(uint16_t*)0x2000259e = 1;
  *(uint16_t*)0x200025a0 = 0xfffa;
  syz_usb_control_io(r[0], 0, 0x200025c0);
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
