// https://syzkaller.appspot.com/bug?id=fa9be2f72b2aee60b3acbbcffecc698d8b160b10
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
  *(uint8_t*)0x20000980 = 0x12;
  *(uint8_t*)0x20000981 = 1;
  *(uint16_t*)0x20000982 = 0;
  *(uint8_t*)0x20000984 = 0x9d;
  *(uint8_t*)0x20000985 = 0xb6;
  *(uint8_t*)0x20000986 = 0xa6;
  *(uint8_t*)0x20000987 = 8;
  *(uint16_t*)0x20000988 = 0x182d;
  *(uint16_t*)0x2000098a = 0x21c;
  *(uint16_t*)0x2000098c = 0x7d73;
  *(uint8_t*)0x2000098e = 0;
  *(uint8_t*)0x2000098f = 0;
  *(uint8_t*)0x20000990 = 0;
  *(uint8_t*)0x20000991 = 1;
  *(uint8_t*)0x20000992 = 9;
  *(uint8_t*)0x20000993 = 2;
  *(uint16_t*)0x20000994 = 0x6d;
  *(uint8_t*)0x20000996 = 1;
  *(uint8_t*)0x20000997 = 0;
  *(uint8_t*)0x20000998 = 0;
  *(uint8_t*)0x20000999 = 0;
  *(uint8_t*)0x2000099a = 0;
  *(uint8_t*)0x2000099b = 9;
  *(uint8_t*)0x2000099c = 4;
  *(uint8_t*)0x2000099d = 0xed;
  *(uint8_t*)0x2000099e = 0;
  *(uint8_t*)0x2000099f = 9;
  *(uint8_t*)0x200009a0 = 0x9d;
  *(uint8_t*)0x200009a1 = 0x69;
  *(uint8_t*)0x200009a2 = 0x34;
  *(uint8_t*)0x200009a3 = 7;
  *(uint8_t*)0x200009a4 = 9;
  *(uint8_t*)0x200009a5 = 5;
  *(uint8_t*)0x200009a6 = 0xd;
  *(uint8_t*)0x200009a7 = 0x13;
  *(uint16_t*)0x200009a8 = 0x100;
  *(uint8_t*)0x200009aa = 0;
  *(uint8_t*)0x200009ab = 0x1f;
  *(uint8_t*)0x200009ac = 7;
  *(uint8_t*)0x200009ad = 2;
  *(uint8_t*)0x200009ae = 0x29;
  *(uint8_t*)0x200009af = 9;
  *(uint8_t*)0x200009b0 = 5;
  *(uint8_t*)0x200009b1 = 0;
  *(uint8_t*)0x200009b2 = 2;
  *(uint16_t*)0x200009b3 = 6;
  *(uint8_t*)0x200009b5 = 0x73;
  *(uint8_t*)0x200009b6 = 0;
  *(uint8_t*)0x200009b7 = 1;
  *(uint8_t*)0x200009b8 = 9;
  *(uint8_t*)0x200009b9 = 5;
  *(uint8_t*)0x200009ba = 0xe;
  *(uint8_t*)0x200009bb = 0x10;
  *(uint16_t*)0x200009bc = 0;
  *(uint8_t*)0x200009be = -1;
  *(uint8_t*)0x200009bf = 1;
  *(uint8_t*)0x200009c0 = 4;
  *(uint8_t*)0x200009c1 = 2;
  *(uint8_t*)0x200009c2 = 0;
  *(uint8_t*)0x200009c3 = 9;
  *(uint8_t*)0x200009c4 = 5;
  *(uint8_t*)0x200009c5 = 4;
  *(uint8_t*)0x200009c6 = 8;
  *(uint16_t*)0x200009c7 = 0xfff9;
  *(uint8_t*)0x200009c9 = 4;
  *(uint8_t*)0x200009ca = 6;
  *(uint8_t*)0x200009cb = 3;
  *(uint8_t*)0x200009cc = 9;
  *(uint8_t*)0x200009cd = 5;
  *(uint8_t*)0x200009ce = 0xb;
  *(uint8_t*)0x200009cf = 0;
  *(uint16_t*)0x200009d0 = 0xe6d;
  *(uint8_t*)0x200009d2 = 0x94;
  *(uint8_t*)0x200009d3 = 3;
  *(uint8_t*)0x200009d4 = 9;
  *(uint8_t*)0x200009d5 = 9;
  *(uint8_t*)0x200009d6 = 5;
  *(uint8_t*)0x200009d7 = 0;
  *(uint8_t*)0x200009d8 = 0;
  *(uint16_t*)0x200009d9 = 0x2a8;
  *(uint8_t*)0x200009db = 4;
  *(uint8_t*)0x200009dc = 3;
  *(uint8_t*)0x200009dd = 3;
  *(uint8_t*)0x200009de = 2;
  *(uint8_t*)0x200009df = 7;
  *(uint8_t*)0x200009e0 = 2;
  *(uint8_t*)0x200009e1 = 0x22;
  *(uint8_t*)0x200009e2 = 9;
  *(uint8_t*)0x200009e3 = 5;
  *(uint8_t*)0x200009e4 = 0xf;
  *(uint8_t*)0x200009e5 = 0;
  *(uint16_t*)0x200009e6 = 2;
  *(uint8_t*)0x200009e8 = 0;
  *(uint8_t*)0x200009e9 = 2;
  *(uint8_t*)0x200009ea = 1;
  *(uint8_t*)0x200009eb = 9;
  *(uint8_t*)0x200009ec = 5;
  *(uint8_t*)0x200009ed = 6;
  *(uint8_t*)0x200009ee = 1;
  *(uint16_t*)0x200009ef = 0;
  *(uint8_t*)0x200009f1 = -1;
  *(uint8_t*)0x200009f2 = 0xfe;
  *(uint8_t*)0x200009f3 = 0;
  *(uint8_t*)0x200009f4 = 2;
  *(uint8_t*)0x200009f5 = 0x37;
  *(uint8_t*)0x200009f6 = 9;
  *(uint8_t*)0x200009f7 = 5;
  *(uint8_t*)0x200009f8 = 2;
  *(uint8_t*)0x200009f9 = 1;
  *(uint16_t*)0x200009fa = 5;
  *(uint8_t*)0x200009fc = 8;
  *(uint8_t*)0x200009fd = 0x4e;
  *(uint8_t*)0x200009fe = 0x7e;
  syz_usb_connect(3, 0x7f, 0x20000980, 0);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
