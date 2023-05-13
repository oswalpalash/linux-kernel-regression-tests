// https://syzkaller.appspot.com/bug?id=cabffad18eb74197f84871802fd2c5117b61febf
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

static unsigned long long procid;

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

#define MAX_FDS 30

#define USB_MAX_IFACE_NUM 4
#define USB_MAX_EP_NUM 32
#define USB_MAX_FDS 6

struct usb_endpoint_index {
  struct usb_endpoint_descriptor desc;
  int handle;
};

struct usb_iface_index {
  struct usb_interface_descriptor* iface;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bInterfaceClass;
  struct usb_endpoint_index eps[USB_MAX_EP_NUM];
  int eps_num;
};

struct usb_device_index {
  struct usb_device_descriptor* dev;
  struct usb_config_descriptor* config;
  uint8_t bDeviceClass;
  uint8_t bMaxPower;
  int config_length;
  struct usb_iface_index ifaces[USB_MAX_IFACE_NUM];
  int ifaces_num;
  int iface_cur;
};

struct usb_info {
  int fd;
  struct usb_device_index index;
};

static struct usb_info usb_devices[USB_MAX_FDS];
static int usb_devices_num;

static bool parse_usb_descriptor(const char* buffer, size_t length,
                                 struct usb_device_index* index)
{
  if (length < sizeof(*index->dev) + sizeof(*index->config))
    return false;
  memset(index, 0, sizeof(*index));
  index->dev = (struct usb_device_descriptor*)buffer;
  index->config = (struct usb_config_descriptor*)(buffer + sizeof(*index->dev));
  index->bDeviceClass = index->dev->bDeviceClass;
  index->bMaxPower = index->config->bMaxPower;
  index->config_length = length - sizeof(*index->dev);
  index->iface_cur = -1;
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
      index->ifaces[index->ifaces_num].iface = iface;
      index->ifaces[index->ifaces_num].bInterfaceNumber =
          iface->bInterfaceNumber;
      index->ifaces[index->ifaces_num].bAlternateSetting =
          iface->bAlternateSetting;
      index->ifaces[index->ifaces_num].bInterfaceClass = iface->bInterfaceClass;
      index->ifaces_num++;
    }
    if (desc_type == USB_DT_ENDPOINT && index->ifaces_num > 0) {
      struct usb_iface_index* iface = &index->ifaces[index->ifaces_num - 1];
      if (iface->eps_num < USB_MAX_EP_NUM) {
        memcpy(&iface->eps[iface->eps_num].desc, buffer + offset,
               sizeof(iface->eps[iface->eps_num].desc));
        iface->eps_num++;
      }
    }
    offset += desc_length;
  }
  return true;
}

static struct usb_device_index* add_usb_index(int fd, const char* dev,
                                              size_t dev_len)
{
  int i = __atomic_fetch_add(&usb_devices_num, 1, __ATOMIC_RELAXED);
  if (i >= USB_MAX_FDS)
    return NULL;
  if (!parse_usb_descriptor(dev, dev_len, &usb_devices[i].index))
    return NULL;
  __atomic_store_n(&usb_devices[i].fd, fd, __ATOMIC_RELEASE);
  return &usb_devices[i].index;
}

static struct usb_device_index* lookup_usb_index(int fd)
{
  for (int i = 0; i < USB_MAX_FDS; i++) {
    if (__atomic_load_n(&usb_devices[i].fd, __ATOMIC_ACQUIRE) == fd) {
      return &usb_devices[i].index;
    }
  }
  return NULL;
}

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

static bool
lookup_connect_response_in(int fd, const struct vusb_connect_descriptors* descs,
                           const struct usb_ctrlrequest* ctrl,
                           char** response_data, uint32_t* response_length)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  uint8_t str_idx;
  if (!index)
    return false;
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
        if (!descs->qual) {
          struct usb_qualifier_descriptor* qual =
              (struct usb_qualifier_descriptor*)response_data;
          qual->bLength = sizeof(*qual);
          qual->bDescriptorType = USB_DT_DEVICE_QUALIFIER;
          qual->bcdUSB = index->dev->bcdUSB;
          qual->bDeviceClass = index->dev->bDeviceClass;
          qual->bDeviceSubClass = index->dev->bDeviceSubClass;
          qual->bDeviceProtocol = index->dev->bDeviceProtocol;
          qual->bMaxPacketSize0 = index->dev->bMaxPacketSize0;
          qual->bNumConfigurations = index->dev->bNumConfigurations;
          qual->bRESERVED = 0;
          *response_length = sizeof(*qual);
          return true;
        }
        *response_data = descs->qual;
        *response_length = descs->qual_len;
        return true;
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  return false;
}

typedef bool (*lookup_connect_out_response_t)(
    int fd, const struct vusb_connect_descriptors* descs,
    const struct usb_ctrlrequest* ctrl, bool* done);

#define ATH9K_FIRMWARE_DOWNLOAD 0x30
#define ATH9K_FIRMWARE_DOWNLOAD_COMP 0x31

static bool lookup_connect_response_out_ath9k(
    int fd, const struct vusb_connect_descriptors* descs,
    const struct usb_ctrlrequest* ctrl, bool* done)
{
  switch (ctrl->bRequestType & USB_TYPE_MASK) {
  case USB_TYPE_STANDARD:
    switch (ctrl->bRequest) {
    case USB_REQ_SET_CONFIGURATION:
      return true;
    default:
      break;
    }
    break;
  case USB_TYPE_VENDOR:
    switch (ctrl->bRequest) {
    case ATH9K_FIRMWARE_DOWNLOAD:
      return true;
    case ATH9K_FIRMWARE_DOWNLOAD_COMP:
      *done = true;
      return true;
    default:
      break;
    }
    break;
  }
  return false;
}

#define UDC_NAME_LENGTH_MAX 128

struct usb_raw_init {
  __u8 driver_name[UDC_NAME_LENGTH_MAX];
  __u8 device_name[UDC_NAME_LENGTH_MAX];
  __u8 speed;
};

enum usb_raw_event_type {
  USB_RAW_EVENT_INVALID = 0,
  USB_RAW_EVENT_CONNECT = 1,
  USB_RAW_EVENT_CONTROL = 2,
};

struct usb_raw_event {
  __u32 type;
  __u32 length;
  __u8 data[0];
};

struct usb_raw_ep_io {
  __u16 ep;
  __u16 flags;
  __u32 length;
  __u8 data[0];
};

#define USB_RAW_EPS_NUM_MAX 30
#define USB_RAW_EP_NAME_MAX 16
#define USB_RAW_EP_ADDR_ANY 0xff

struct usb_raw_ep_caps {
  __u32 type_control : 1;
  __u32 type_iso : 1;
  __u32 type_bulk : 1;
  __u32 type_int : 1;
  __u32 dir_in : 1;
  __u32 dir_out : 1;
};

struct usb_raw_ep_limits {
  __u16 maxpacket_limit;
  __u16 max_streams;
  __u32 reserved;
};

struct usb_raw_ep_info {
  __u8 name[USB_RAW_EP_NAME_MAX];
  __u32 addr;
  struct usb_raw_ep_caps caps;
  struct usb_raw_ep_limits limits;
};

struct usb_raw_eps_info {
  struct usb_raw_ep_info eps[USB_RAW_EPS_NUM_MAX];
};

#define USB_RAW_IOCTL_INIT _IOW('U', 0, struct usb_raw_init)
#define USB_RAW_IOCTL_RUN _IO('U', 1)
#define USB_RAW_IOCTL_EVENT_FETCH _IOR('U', 2, struct usb_raw_event)
#define USB_RAW_IOCTL_EP0_WRITE _IOW('U', 3, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP0_READ _IOWR('U', 4, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_ENABLE _IOW('U', 5, struct usb_endpoint_descriptor)
#define USB_RAW_IOCTL_EP_DISABLE _IOW('U', 6, __u32)
#define USB_RAW_IOCTL_EP_WRITE _IOW('U', 7, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_EP_READ _IOWR('U', 8, struct usb_raw_ep_io)
#define USB_RAW_IOCTL_CONFIGURE _IO('U', 9)
#define USB_RAW_IOCTL_VBUS_DRAW _IOW('U', 10, __u32)
#define USB_RAW_IOCTL_EPS_INFO _IOR('U', 11, struct usb_raw_eps_info)
#define USB_RAW_IOCTL_EP0_STALL _IO('U', 12)
#define USB_RAW_IOCTL_EP_SET_HALT _IOW('U', 13, __u32)
#define USB_RAW_IOCTL_EP_CLEAR_HALT _IOW('U', 14, __u32)
#define USB_RAW_IOCTL_EP_SET_WEDGE _IOW('U', 15, __u32)

static int usb_raw_open()
{
  return open("/dev/raw-gadget", O_RDWR);
}

static int usb_raw_init(int fd, uint32_t speed, const char* driver,
                        const char* device)
{
  struct usb_raw_init arg;
  strncpy((char*)&arg.driver_name[0], driver, sizeof(arg.driver_name));
  strncpy((char*)&arg.device_name[0], device, sizeof(arg.device_name));
  arg.speed = speed;
  return ioctl(fd, USB_RAW_IOCTL_INIT, &arg);
}

static int usb_raw_run(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_RUN, 0);
}

static int usb_raw_event_fetch(int fd, struct usb_raw_event* event)
{
  return ioctl(fd, USB_RAW_IOCTL_EVENT_FETCH, event);
}

static int usb_raw_ep0_write(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_WRITE, io);
}

static int usb_raw_ep0_read(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_READ, io);
}

static int usb_raw_ep_write(int fd, struct usb_raw_ep_io* io)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_WRITE, io);
}

static int usb_raw_ep_enable(int fd, struct usb_endpoint_descriptor* desc)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_ENABLE, desc);
}

static int usb_raw_ep_disable(int fd, int ep)
{
  return ioctl(fd, USB_RAW_IOCTL_EP_DISABLE, ep);
}

static int usb_raw_configure(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_CONFIGURE, 0);
}

static int usb_raw_vbus_draw(int fd, uint32_t power)
{
  return ioctl(fd, USB_RAW_IOCTL_VBUS_DRAW, power);
}

static int usb_raw_ep0_stall(int fd)
{
  return ioctl(fd, USB_RAW_IOCTL_EP0_STALL, 0);
}

static int lookup_endpoint(int fd, uint8_t bEndpointAddress)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  if (!index)
    return -1;
  if (index->iface_cur < 0)
    return -1;
  for (int ep = 0; index->ifaces[index->iface_cur].eps_num; ep++)
    if (index->ifaces[index->iface_cur].eps[ep].desc.bEndpointAddress ==
        bEndpointAddress)
      return index->ifaces[index->iface_cur].eps[ep].handle;
  return -1;
}

static void set_interface(int fd, int n)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  if (!index)
    return;
  if (index->iface_cur >= 0 && index->iface_cur < index->ifaces_num) {
    for (int ep = 0; ep < index->ifaces[index->iface_cur].eps_num; ep++) {
      int rv = usb_raw_ep_disable(
          fd, index->ifaces[index->iface_cur].eps[ep].handle);
      if (rv < 0) {
      } else {
      }
    }
  }
  if (n >= 0 && n < index->ifaces_num) {
    for (int ep = 0; ep < index->ifaces[n].eps_num; ep++) {
      int rv = usb_raw_ep_enable(fd, &index->ifaces[n].eps[ep].desc);
      if (rv < 0) {
      } else {
        index->ifaces[n].eps[ep].handle = rv;
      }
    }
    index->iface_cur = n;
  }
}

static int configure_device(int fd)
{
  struct usb_device_index* index = lookup_usb_index(fd);
  if (!index)
    return -1;
  int rv = usb_raw_vbus_draw(fd, index->bMaxPower);
  if (rv < 0) {
    return rv;
  }
  rv = usb_raw_configure(fd);
  if (rv < 0) {
    return rv;
  }
  set_interface(fd, 0);
  return 0;
}

#define USB_MAX_PACKET_SIZE 4096

struct usb_raw_control_event {
  struct usb_raw_event inner;
  struct usb_ctrlrequest ctrl;
  char data[USB_MAX_PACKET_SIZE];
};

struct usb_raw_ep_io_data {
  struct usb_raw_ep_io inner;
  char data[USB_MAX_PACKET_SIZE];
};

static volatile long
syz_usb_connect_impl(uint64_t speed, uint64_t dev_len, const char* dev,
                     const struct vusb_connect_descriptors* descs,
                     lookup_connect_out_response_t lookup_connect_response_out)
{
  if (!dev) {
    return -1;
  }
  int fd = usb_raw_open();
  if (fd < 0) {
    return fd;
  }
  if (fd >= MAX_FDS) {
    close(fd);
    return -1;
  }
  struct usb_device_index* index = add_usb_index(fd, dev, dev_len);
  if (!index) {
    return -1;
  }
  char device[32];
  sprintf(&device[0], "dummy_udc.%llu", procid);
  int rv = usb_raw_init(fd, speed, "dummy_udc", &device[0]);
  if (rv < 0) {
    return rv;
  }
  rv = usb_raw_run(fd);
  if (rv < 0) {
    return rv;
  }
  bool done = false;
  while (!done) {
    struct usb_raw_control_event event;
    event.inner.type = 0;
    event.inner.length = sizeof(event.ctrl);
    rv = usb_raw_event_fetch(fd, (struct usb_raw_event*)&event);
    if (rv < 0) {
      return rv;
    }
    if (event.inner.type != USB_RAW_EVENT_CONTROL)
      continue;
    char* response_data = NULL;
    uint32_t response_length = 0;
    if (event.ctrl.bRequestType & USB_DIR_IN) {
      if (!lookup_connect_response_in(fd, descs, &event.ctrl, &response_data,
                                      &response_length)) {
        usb_raw_ep0_stall(fd);
        continue;
      }
    } else {
      if (!lookup_connect_response_out(fd, descs, &event.ctrl, &done)) {
        usb_raw_ep0_stall(fd);
        continue;
      }
      response_data = NULL;
      response_length = event.ctrl.wLength;
    }
    if ((event.ctrl.bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD &&
        event.ctrl.bRequest == USB_REQ_SET_CONFIGURATION) {
      rv = configure_device(fd);
      if (rv < 0) {
        return rv;
      }
    }
    struct usb_raw_ep_io_data response;
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
      rv = usb_raw_ep0_write(fd, (struct usb_raw_ep_io*)&response);
    } else {
      rv = usb_raw_ep0_read(fd, (struct usb_raw_ep_io*)&response);
    }
    if (rv < 0) {
      return rv;
    }
  }
  sleep_ms(200);
  return fd;
}

static volatile long syz_usb_connect_ath9k(volatile long a0, volatile long a1,
                                           volatile long a2, volatile long a3)
{
  uint64_t speed = a0;
  uint64_t dev_len = a1;
  const char* dev = (const char*)a2;
  const struct vusb_connect_descriptors* descs =
      (const struct vusb_connect_descriptors*)a3;
  return syz_usb_connect_impl(speed, dev_len, dev, descs,
                              &lookup_connect_response_out_ath9k);
}

static volatile long syz_usb_ep_write(volatile long a0, volatile long a1,
                                      volatile long a2, volatile long a3)
{
  int fd = a0;
  uint8_t ep = a1;
  uint32_t len = a2;
  char* data = (char*)a3;
  int ep_handle = lookup_endpoint(fd, ep);
  if (ep_handle < 0) {
    return -1;
  }
  struct usb_raw_ep_io_data io_data;
  io_data.inner.ep = ep_handle;
  io_data.inner.flags = 0;
  if (len > sizeof(io_data.data))
    len = sizeof(io_data.data);
  io_data.inner.length = len;
  memcpy(&io_data.data[0], data, len);
  int rv = usb_raw_ep_write(fd, (struct usb_raw_ep_io*)&io_data);
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
  for (int i = 0; i < 100; i++) {
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
  int iter = 0;
  for (;; iter++) {
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
  *(uint8_t*)0x20000200 = 0x12;
  *(uint8_t*)0x20000201 = 1;
  *(uint16_t*)0x20000202 = 0x200;
  *(uint8_t*)0x20000204 = -1;
  *(uint8_t*)0x20000205 = -1;
  *(uint8_t*)0x20000206 = -1;
  *(uint8_t*)0x20000207 = 0x40;
  *(uint16_t*)0x20000208 = 0xcf3;
  *(uint16_t*)0x2000020a = 0x9271;
  *(uint16_t*)0x2000020c = 0x108;
  *(uint8_t*)0x2000020e = 1;
  *(uint8_t*)0x2000020f = 2;
  *(uint8_t*)0x20000210 = 3;
  *(uint8_t*)0x20000211 = 1;
  *(uint8_t*)0x20000212 = 9;
  *(uint8_t*)0x20000213 = 2;
  *(uint16_t*)0x20000214 = 0x48;
  *(uint8_t*)0x20000216 = 1;
  *(uint8_t*)0x20000217 = 1;
  *(uint8_t*)0x20000218 = 0;
  *(uint8_t*)0x20000219 = 0x80;
  *(uint8_t*)0x2000021a = 0xfa;
  *(uint8_t*)0x2000021b = 9;
  *(uint8_t*)0x2000021c = 4;
  *(uint8_t*)0x2000021d = 0;
  *(uint8_t*)0x2000021e = 0;
  *(uint8_t*)0x2000021f = 6;
  *(uint8_t*)0x20000220 = -1;
  *(uint8_t*)0x20000221 = 0;
  *(uint8_t*)0x20000222 = 0;
  *(uint8_t*)0x20000223 = 0;
  *(uint8_t*)0x20000224 = 9;
  *(uint8_t*)0x20000225 = 5;
  *(uint8_t*)0x20000226 = 1;
  *(uint8_t*)0x20000227 = 2;
  *(uint16_t*)0x20000228 = 0x200;
  *(uint8_t*)0x2000022a = 0;
  *(uint8_t*)0x2000022b = 0;
  *(uint8_t*)0x2000022c = 0;
  *(uint8_t*)0x2000022d = 9;
  *(uint8_t*)0x2000022e = 5;
  *(uint8_t*)0x2000022f = 0x82;
  *(uint8_t*)0x20000230 = 2;
  *(uint16_t*)0x20000231 = 0x200;
  *(uint8_t*)0x20000233 = 0;
  *(uint8_t*)0x20000234 = 0;
  *(uint8_t*)0x20000235 = 0;
  *(uint8_t*)0x20000236 = 9;
  *(uint8_t*)0x20000237 = 5;
  *(uint8_t*)0x20000238 = 0x83;
  *(uint8_t*)0x20000239 = 3;
  *(uint16_t*)0x2000023a = 0x40;
  *(uint8_t*)0x2000023c = 1;
  *(uint8_t*)0x2000023d = 0;
  *(uint8_t*)0x2000023e = 0;
  *(uint8_t*)0x2000023f = 9;
  *(uint8_t*)0x20000240 = 5;
  *(uint8_t*)0x20000241 = 4;
  *(uint8_t*)0x20000242 = 3;
  *(uint16_t*)0x20000243 = 0x40;
  *(uint8_t*)0x20000245 = 1;
  *(uint8_t*)0x20000246 = 0;
  *(uint8_t*)0x20000247 = 0;
  *(uint8_t*)0x20000248 = 9;
  *(uint8_t*)0x20000249 = 5;
  *(uint8_t*)0x2000024a = 5;
  *(uint8_t*)0x2000024b = 2;
  *(uint16_t*)0x2000024c = 0x200;
  *(uint8_t*)0x2000024e = 0;
  *(uint8_t*)0x2000024f = 0;
  *(uint8_t*)0x20000250 = 0;
  *(uint8_t*)0x20000251 = 9;
  *(uint8_t*)0x20000252 = 5;
  *(uint8_t*)0x20000253 = 6;
  *(uint8_t*)0x20000254 = 2;
  *(uint16_t*)0x20000255 = 0x200;
  *(uint8_t*)0x20000257 = 0;
  *(uint8_t*)0x20000258 = 0;
  *(uint8_t*)0x20000259 = 0;
  res = -1;
  res = syz_usb_connect_ath9k(3, 0x5a, 0x20000200, 0);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x20001000 = 0x8d;
  *(uint16_t*)0x20001002 = 0x4e00;
  memcpy((void*)0x20001004,
         "\x82\x37\xa9\x7b\xfa\xee\x22\xb0\x1b\x92\x9e\x59\x72\xc1\x3d\xc6\xae"
         "\x07\x1c\x85\xc7\x2f\x63\xc4\x01\xa6\xf9\x32\x33\x79\xbc\x25\x22\xea"
         "\xac\x26\x76\xf8\x65\x2a\x74\x28\x7b\x08\x7d\x8b\xf7\x73\x37\xdc\xaf"
         "\xde\xaf\xef\xfc\x6e\x14\x4c\x76\xe6\x7d\xae\x85\x43\x84\xba\xa4\x5d"
         "\x7d\x59\x8a\xad\x38\xb5\xba\x66\xf6\x46\x08\xd3\xaa\x4b\xcf\x23\x4c"
         "\x8a\x8f\x13\x10\x84\xc1\xd8\xaa\xe4\x97\xff\xe8\x14\x82\x32\xda\xe7"
         "\xab\xe7\x69\xde\xc7\x98\x06\xee\x34\x97\x10\xd6\xf2\x3b\xee\xfd\x53"
         "\xb6\xe1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00",
         141);
  *(uint16_t*)0x20001094 = 0xcd;
  *(uint16_t*)0x20001096 = 0x4e00;
  memcpy((void*)0x20001098,
         "\x6d\x2a\x16\x0e\xae\x2c\x16\x2d\x9c\x9a\x76\x83\x28\xd0\x6e\x4a\x29"
         "\x7b\x36\x0f\xca\x29\x82\x89\xcc\xec\x8a\x7d\xc8\x14\xb0\x29\xf2\xc5"
         "\xa7\xf2\xc5\x05\xef\xb3\xeb\x66\x84\xb4\x39\x30\x69\xca\x1e\xac\x1d"
         "\x9f\xc1\xb9\x15\x4c\x17\xdf\x48\xa9\xa8\xd7\x59\x9b\xa7\x71\xc1\x39"
         "\x09\xd8\xdd\x67\x13\xb8\x6a\x6f\x7b\x5f\x71\xa8\x83\x20\xcd\x5c\xfd"
         "\x19\x2a\xd3\xda\x19\xb3\xdb\x56\x52\x4d\x5d\xed\x9c\xf3\xe9\xe7\x15"
         "\xb9\x3f\x08\x68\xd1\xf0\xa7\xee\x9e\x42\x00\x15\x86\x21\x66\xa7\x7a"
         "\xec\x11\xe1\x46\x28\x1b\x07\x0d\x05\x9b\x0f\x28\xfc\x7d\x46\x7c\xf9"
         "\x12\xcf\x0c\xcd\x4a\x38\x66\x3f\xf9\x0e\x47\x13\xfe\xe6\xb5\x91\x95"
         "\xf2\x32\x9e\x3e\x45\x25\x21\x0e\x80\x3b\xdf\x20\x29\x3d\x54\x33\xbd"
         "\x0f\x2d\x95\xf2\x6a\xba\xfb\x69\x5e\xff\xb5\x0f\x96\x1f\x8e\x76\xe6"
         "\xbb\x44\xa7\xd6\x13\x99\x09\xee\x65\xeb\x6d\x7f\xb8\xae\xda\xd5\x7c"
         "\xc3",
         205);
  *(uint16_t*)0x20001168 = 0xfe51;
  *(uint16_t*)0x2000116a = 0x4e00;
  memcpy((void*)0x2000116c,
         "\xa3\x14\x5e\x0f\x68\x5f\x0d\xaa\x60\x05\xdc\x68\xde\xc2\xbc\x51\x1d"
         "\x8f\x4c\xaa\x5d\x88\x71\x8a\x82\xca\xe1\xe9\x3d\xe7\x24\xec\xb4\x39"
         "\x28\x6d\xde\x61\x90\xb1\x59\x81\xb8\x06\x39\x1d\xe4\xc5\xc7\x72\xb2"
         "\x4e\x24\x2f\x50\x98\x64\x8a\x4a\xf8\x55\x4f\x47\xf7\x0c\xe9\x2e\x6c"
         "\x6a\x51\xd7\xf6\xf1\xa3\x34\x97\x8a\x6a\xcc\x3a\xdb\x04\x25\x42\xcd"
         "\x8d\xa6\xce\x87\x5f\x3b\xce\xf3\x80\x55\xac\x29\x65\x78\x43\x5c\xc1"
         "\xb7\x18\xf7\x30\x1b\xc6\x52\x6b\xb2\xa8\xe8\xd7\xb2\xc3\xbb\x61\x08"
         "\xff\x59\x07\x3a\xe3\x6f\x8a\xf7\xff\x20\x88\x4f\x30\x91\x65\x22\x0d"
         "\xf2\x7d\xba\x83\x5e\xae\x75\x8b\xb9\xfa\xd3\x13\x06\xb1\xd5\xe6\xae"
         "\x1f\xff\xe4\x16\xb0\xe8\xfa\x1a\xce\x62\xfe\x8c\x97\xf0\xb8\x93\xd1"
         "\x53\x21\xa2\xac\xc4\xdc\x76\xa1\x8c\x6d\x5b\x50\x22\x72\xfa\x35\x1a"
         "\xe7\x33",
         189);
  *(uint16_t*)0x2000122c = 0x94;
  *(uint16_t*)0x2000122e = 0x4e00;
  memcpy((void*)0x20001230,
         "\xd6\x07\xdf\xbc\xfb\xea\x35\xcb\x5a\x47\xcb\xa0\xc6\x56\x52\x08\x22"
         "\x7c\x2d\x2a\x4e\x60\x33\xb9\xef\xbf\xb2\x92\x58\xe3\x7f\xfa\xea\x3b"
         "\x7a\x7a\x1e\x54\xfb\x61\xf4\x2c\xcc\xff\xaa\x59\x7b\x96\xc7\xeb\x95"
         "\xc2\x61\x34\xbf\x58\x5e\xf3\x79\xba\x02\x5f\x60\xa0\xb3\x29\x90\x8e"
         "\xdc\x01\xed\x6e\xa4\x29\x34\xa9\xc9\xf8\x2c\xdf\x79\x5b\xfc\x86\x1f"
         "\x4d\x2e\x0e\x14\xc6\xe5\xcc\x0c\xcc\xa3\x9f\x62\x4b\x50\xc2\xc6\x66"
         "\x83\xab\x5c\x36\xa8\x40\xd9\x4f\x68\x5c\x4d\x67\xbc\xb6\xb4\x64\x51"
         "\x99\x6c\x61\x3a\x84\xca\x7c\x12\x25\x5d\x52\xf1\x65\x95\x7f\x6a\xe9"
         "\x65\x84\xc9\x61\x6f\xa8\x34\xfe\x37\xf3\x7b\xfe",
         148);
  *(uint16_t*)0x200012c4 = 0;
  *(uint16_t*)0x200012c6 = 0x4e00;
  *(uint16_t*)0x200012c8 = 0xd7;
  *(uint16_t*)0x200012ca = 0x4e00;
  memcpy((void*)0x200012cc,
         "\x85\x7c\x34\x28\xef\x3c\xe7\xe8\x26\xc4\x88\x3c\xda\xfd\xfa\x0b\x0c"
         "\xc4\xce\x67\x0f\x79\x3f\x4f\x79\xf9\xb3\xf5\x8b\x53\xe0\x82\x90\xc2"
         "\xdc\xc8\x1b\xae\x66\xed\xe5\x82\xcf\x82\x99\x2d\x99\x28\xa1\x05\x48"
         "\x04\x8d\x99\x51\x71\xc4\x6d\x4b\x9e\x5e\xde\xbd\x7b\x50\xa9\x8d\xf4"
         "\x3d\xab\x2e\x1c\xbf\xb4\x34\x30\xe2\xde\x30\x33\x5e\xb3\x9b\xa9\x7d"
         "\xad\x2b\x61\x11\xec\xdf\x75\x54\xc4\x91\x86\xa3\xa4\xf9\x56\x08\x5a"
         "\xd3\x15\x13\xfa\x23\xbf\x2c\x2f\x15\x65\x6e\x8a\xda\x7d\xc4\x90\x0a"
         "\x33\x2f\x7d\x79\x5e\x05\xcd\x67\x54\x37\xef\x27\xa0\xf6\x05\xc2\x13"
         "\x33\x6d\x00\x36\x10\x41\xb5\xc3\xb0\xa6\x9d\xfb\x86\x4c\x7a\x5e\x8d"
         "\xc9\xd9\x65\x6e\xaf\x35\xcd\x57\xd7\xd8\x6d\x63\x05\x6a\xb5\xa0\xfc"
         "\x95\x9e\xf0\x52\x47\x1a\xd8\xc8\x00\x1b\xe3\xc7\xdb\x3a\x45\x6f\xd5"
         "\xbf\xe3\x1e\x67\x6d\xa5\x4f\x38\xb9\x8a\x59\x48\x6a\x67\xb7\x83\x51"
         "\x73\xb1\xf8\x41\x76\x14\x30\x93\x1f\x72\x6a",
         215);
  *(uint16_t*)0x200013a4 = 0xa0;
  *(uint16_t*)0x200013a6 = 0x4e00;
  memcpy((void*)0x200013a8,
         "\xaa\xb9\x18\x6b\x4a\x11\xc9\xc0\x71\xcb\x8b\xa2\xe6\x90\xc0\xff\x8f"
         "\xf4\xb7\x6e\x6f\x42\xd3\x16\x48\xd1\x1b\xb6\x69\x74\xe7\xb8\xfa\xd8"
         "\x20\x40\x0e\x23\xe9\x39\xf6\x5c\xe0\xca\x81\xec\xea\xa1\x13\x5f\x9f"
         "\xdd\x33\x8e\xf1\xa1\x4b\x52\x8a\x66\xda\xf0\xfb\x2f\x8a\x58\x9d\x06"
         "\xb2\xf8\x39\x05\xb5\x11\x33\x9a\x63\x6e\x84\x8d\xd6\xa5\xb7\xa5\xf7"
         "\x9e\x2b\x03\xb2\x21\x20\xd3\xed\xe4\x92\xdc\x12\x7b\xe2\x01\x39\x92"
         "\x2c\xb5\x57\x35\x1e\xa7\xc8\x7e\x32\x8d\x2b\x4f\x40\x5a\xdd\xe1\x48"
         "\x7e\xae\x9b\xb8\x09\xb0\xb6\xb3\x6d\xf4\xde\x24\x7e\xa8\x78\x69\xf7"
         "\xc4\xda\x77\xfd\x6f\x84\xfe\xbb\xbe\xef\x69\x9d\xfe\x02\xd7\x8a\x77"
         "\x65\x23\xd1\x16\x7b\xaa\x7e",
         160);
  syz_usb_ep_write(r[0], 0x82, 0x448, 0x20001000);
  *(uint16_t*)0x20000b40 = 0;
  *(uint16_t*)0x20000b42 = 0x4e00;
  *(uint16_t*)0x20000b44 = 0;
  *(uint16_t*)0x20000b46 = 0x4e00;
  syz_usb_ep_write(r[0], 0x82, 8, 0x20000b40);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
