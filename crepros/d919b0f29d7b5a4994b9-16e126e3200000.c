// https://syzkaller.appspot.com/bug?id=6c806c0b1d9ef3354334625ecc3c8643854b28eb
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
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

#include <linux/futex.h>
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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i;
  for (i = 0; i < 100; i++) {
    if (pthread_create(&th, &attr, fn, arg) == 0) {
      pthread_attr_destroy(&attr);
      return;
    }
    if (errno == EAGAIN) {
      usleep(50);
      continue;
    }
    break;
  }
  exit(1);
}

typedef struct {
  int state;
} event_t;

static void event_init(event_t* ev)
{
  ev->state = 0;
}

static void event_reset(event_t* ev)
{
  ev->state = 0;
}

static void event_set(event_t* ev)
{
  if (ev->state)
    exit(1);
  __atomic_store_n(&ev->state, 1, __ATOMIC_RELEASE);
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG);
}

static void event_wait(event_t* ev)
{
  while (!__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, 0);
}

static int event_isset(event_t* ev)
{
  return __atomic_load_n(&ev->state, __ATOMIC_ACQUIRE);
}

static int event_timedwait(event_t* ev, uint64_t timeout)
{
  uint64_t start = current_time_ms();
  uint64_t now = start;
  for (;;) {
    uint64_t remain = timeout - (now - start);
    struct timespec ts;
    ts.tv_sec = remain / 1000;
    ts.tv_nsec = (remain % 1000) * 1000 * 1000;
    syscall(SYS_futex, &ev->state, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 0, &ts);
    if (__atomic_load_n(&ev->state, __ATOMIC_RELAXED))
      return 1;
    now = current_time_ms();
    if (now - start > timeout)
      return 0;
  }
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

struct thread_t {
  int created, call;
  event_t ready, done;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    event_wait(&th->ready);
    event_reset(&th->ready);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    event_set(&th->done);
  }
  return 0;
}

static void execute_one(void)
{
  int i, call, thread;
  int collide = 0;
again:
  for (call = 0; call < 2; call++) {
    for (thread = 0; thread < (int)(sizeof(threads) / sizeof(threads[0]));
         thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        event_init(&th->ready);
        event_init(&th->done);
        event_set(&th->done);
        thread_start(thr, th);
      }
      if (!event_isset(&th->done))
        continue;
      event_reset(&th->done);
      th->call = call;
      __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
      event_set(&th->ready);
      if (collide && (call % 2) == 0)
        break;
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
  if (!collide) {
    collide = 1;
    goto again;
  }
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

void execute_call(int call)
{
  switch (call) {
  case 0:
    *(uint8_t*)0x20000080 = 0x12;
    *(uint8_t*)0x20000081 = 1;
    *(uint16_t*)0x20000082 = 3;
    *(uint8_t*)0x20000084 = 0x5b;
    *(uint8_t*)0x20000085 = 0xe3;
    *(uint8_t*)0x20000086 = 0x3c;
    *(uint8_t*)0x20000087 = 4;
    *(uint16_t*)0x20000088 = 0x19d2;
    *(uint16_t*)0x2000008a = 0x121;
    *(uint16_t*)0x2000008c = 0x6a8c;
    *(uint8_t*)0x2000008e = 0x3f;
    *(uint8_t*)0x2000008f = 1;
    *(uint8_t*)0x20000090 = 6;
    *(uint8_t*)0x20000091 = 1;
    *(uint8_t*)0x20000092 = 9;
    *(uint8_t*)0x20000093 = 2;
    *(uint16_t*)0x20000094 = 0x9c;
    *(uint8_t*)0x20000096 = 1;
    *(uint8_t*)0x20000097 = 0;
    *(uint8_t*)0x20000098 = 4;
    *(uint8_t*)0x20000099 = 0x40;
    *(uint8_t*)0x2000009a = 0;
    *(uint8_t*)0x2000009b = 9;
    *(uint8_t*)0x2000009c = 4;
    *(uint8_t*)0x2000009d = 5;
    *(uint8_t*)0x2000009e = 3;
    *(uint8_t*)0x2000009f = 0xc;
    *(uint8_t*)0x200000a0 = 6;
    *(uint8_t*)0x200000a1 = 0x5d;
    *(uint8_t*)0x200000a2 = 0x98;
    *(uint8_t*)0x200000a3 = 9;
    *(uint8_t*)0x200000a4 = 2;
    *(uint8_t*)0x200000a5 = 0xf;
    *(uint8_t*)0x200000a6 = 7;
    *(uint8_t*)0x200000a7 = 5;
    *(uint8_t*)0x200000a8 = 1;
    *(uint8_t*)0x200000a9 = 0x10;
    *(uint16_t*)0x200000aa = 0x7fff;
    *(uint8_t*)0x200000ac = 7;
    *(uint8_t*)0x200000ad = -1;
    *(uint8_t*)0x200000ae = 0xe0;
    *(uint8_t*)0x200000af = 2;
    *(uint8_t*)0x200000b0 = 5;
    *(uint8_t*)0x200000b1 = 7;
    *(uint8_t*)0x200000b2 = 5;
    *(uint8_t*)0x200000b3 = 0xa6;
    *(uint8_t*)0x200000b4 = 2;
    *(uint16_t*)0x200000b5 = 0;
    *(uint8_t*)0x200000b7 = 0;
    *(uint8_t*)0x200000b8 = 0x91;
    *(uint8_t*)0x200000b9 = 5;
    *(uint8_t*)0x200000ba = 2;
    *(uint8_t*)0x200000bb = 0x23;
    *(uint8_t*)0x200000bc = 2;
    *(uint8_t*)0x200000bd = 0x21;
    *(uint8_t*)0x200000be = 7;
    *(uint8_t*)0x200000bf = 5;
    *(uint8_t*)0x200000c0 = 1;
    *(uint8_t*)0x200000c1 = 0;
    *(uint16_t*)0x200000c2 = 8;
    *(uint8_t*)0x200000c4 = 6;
    *(uint8_t*)0x200000c5 = -1;
    *(uint8_t*)0x200000c6 = 1;
    *(uint8_t*)0x200000c7 = 2;
    *(uint8_t*)0x200000c8 = 6;
    *(uint8_t*)0x200000c9 = 2;
    *(uint8_t*)0x200000ca = 0x21;
    *(uint8_t*)0x200000cb = 7;
    *(uint8_t*)0x200000cc = 5;
    *(uint8_t*)0x200000cd = 8;
    *(uint8_t*)0x200000ce = 0x10;
    *(uint16_t*)0x200000cf = 9;
    *(uint8_t*)0x200000d1 = 0xfb;
    *(uint8_t*)0x200000d2 = 0x20;
    *(uint8_t*)0x200000d3 = 3;
    *(uint8_t*)0x200000d4 = 2;
    *(uint8_t*)0x200000d5 = 0xb;
    *(uint8_t*)0x200000d6 = 7;
    *(uint8_t*)0x200000d7 = 5;
    *(uint8_t*)0x200000d8 = 0xcc;
    *(uint8_t*)0x200000d9 = 3;
    *(uint16_t*)0x200000da = 0x7ee;
    *(uint8_t*)0x200000dc = 6;
    *(uint8_t*)0x200000dd = 0xb5;
    *(uint8_t*)0x200000de = 0x1f;
    *(uint8_t*)0x200000df = 2;
    *(uint8_t*)0x200000e0 = 0;
    *(uint8_t*)0x200000e1 = 7;
    *(uint8_t*)0x200000e2 = 5;
    *(uint8_t*)0x200000e3 = -1;
    *(uint8_t*)0x200000e4 = 0;
    *(uint16_t*)0x200000e5 = 7;
    *(uint8_t*)0x200000e7 = -1;
    *(uint8_t*)0x200000e8 = -1;
    *(uint8_t*)0x200000e9 = 1;
    *(uint8_t*)0x200000ea = 2;
    *(uint8_t*)0x200000eb = 0x10;
    *(uint8_t*)0x200000ec = 7;
    *(uint8_t*)0x200000ed = 5;
    *(uint8_t*)0x200000ee = 0x40;
    *(uint8_t*)0x200000ef = 3;
    *(uint16_t*)0x200000f0 = 5;
    *(uint8_t*)0x200000f2 = 5;
    *(uint8_t*)0x200000f3 = 8;
    *(uint8_t*)0x200000f4 = 1;
    *(uint8_t*)0x200000f5 = 7;
    *(uint8_t*)0x200000f6 = 5;
    *(uint8_t*)0x200000f7 = 0x11;
    *(uint8_t*)0x200000f8 = 0;
    *(uint16_t*)0x200000f9 = 0x52ce;
    *(uint8_t*)0x200000fb = 6;
    *(uint8_t*)0x200000fc = 0;
    *(uint8_t*)0x200000fd = 3;
    *(uint8_t*)0x200000fe = 7;
    *(uint8_t*)0x200000ff = 5;
    *(uint8_t*)0x20000100 = -1;
    *(uint8_t*)0x20000101 = 1;
    *(uint16_t*)0x20000102 = 0x3414;
    *(uint8_t*)0x20000104 = 0x12;
    *(uint8_t*)0x20000105 = 7;
    *(uint8_t*)0x20000106 = -1;
    *(uint8_t*)0x20000107 = 2;
    *(uint8_t*)0x20000108 = 0;
    *(uint8_t*)0x20000109 = 7;
    *(uint8_t*)0x2000010a = 5;
    *(uint8_t*)0x2000010b = 4;
    *(uint8_t*)0x2000010c = 0x10;
    *(uint16_t*)0x2000010d = 0x7ff;
    *(uint8_t*)0x2000010f = 0xfa;
    *(uint8_t*)0x20000110 = 0xf9;
    *(uint8_t*)0x20000111 = 0;
    *(uint8_t*)0x20000112 = 2;
    *(uint8_t*)0x20000113 = 0xb;
    *(uint8_t*)0x20000114 = 2;
    *(uint8_t*)0x20000115 = 0x31;
    *(uint8_t*)0x20000116 = 7;
    *(uint8_t*)0x20000117 = 5;
    *(uint8_t*)0x20000118 = 0x1f;
    *(uint8_t*)0x20000119 = 1;
    *(uint16_t*)0x2000011a = 0x101;
    *(uint8_t*)0x2000011c = 0;
    *(uint8_t*)0x2000011d = 5;
    *(uint8_t*)0x2000011e = 5;
    *(uint8_t*)0x2000011f = 2;
    *(uint8_t*)0x20000120 = 0x2f;
    *(uint8_t*)0x20000121 = 2;
    *(uint8_t*)0x20000122 = 0x23;
    *(uint8_t*)0x20000123 = 7;
    *(uint8_t*)0x20000124 = 5;
    *(uint8_t*)0x20000125 = 1;
    *(uint8_t*)0x20000126 = 3;
    *(uint16_t*)0x20000127 = 6;
    *(uint8_t*)0x20000129 = 8;
    *(uint8_t*)0x2000012a = 8;
    *(uint8_t*)0x2000012b = 2;
    *(uint8_t*)0x2000012c = 2;
    *(uint8_t*)0x2000012d = 0x11;
    syz_usb_connect(0, 0xae, 0x20000080, 0);
    break;
  case 1:
    *(uint8_t*)0x20000040 = 0x12;
    *(uint8_t*)0x20000041 = 1;
    *(uint16_t*)0x20000042 = 0;
    *(uint8_t*)0x20000044 = 0x5f;
    *(uint8_t*)0x20000045 = 0xe4;
    *(uint8_t*)0x20000046 = 0x75;
    *(uint8_t*)0x20000047 = 8;
    *(uint16_t*)0x20000048 = 0xbfd;
    *(uint16_t*)0x2000004a = 0x104;
    *(uint16_t*)0x2000004c = 0x678;
    *(uint8_t*)0x2000004e = 0;
    *(uint8_t*)0x2000004f = 0;
    *(uint8_t*)0x20000050 = 3;
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
    *(uint8_t*)0x2000005d = 0xa1;
    *(uint8_t*)0x2000005e = 0;
    *(uint8_t*)0x2000005f = 1;
    *(uint8_t*)0x20000060 = 0x74;
    *(uint8_t*)0x20000061 = 6;
    *(uint8_t*)0x20000062 = 0x38;
    *(uint8_t*)0x20000063 = 0;
    *(uint8_t*)0x20000064 = 7;
    *(uint8_t*)0x20000065 = 5;
    *(uint8_t*)0x20000066 = 2;
    *(uint8_t*)0x20000067 = 2;
    *(uint16_t*)0x20000068 = 0x7fff;
    *(uint8_t*)0x2000006a = 0;
    *(uint8_t*)0x2000006b = 0;
    *(uint8_t*)0x2000006c = 0;
    syz_usb_connect(1, 0x2d, 0x20000040, 0);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
