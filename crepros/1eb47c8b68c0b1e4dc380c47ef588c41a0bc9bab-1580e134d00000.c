// https://syzkaller.appspot.com/bug?id=1eb47c8b68c0b1e4dc380c47ef588c41a0bc9bab
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <linux/genetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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

struct nlmsg {
  char* pos;
  int nesting;
  struct nlattr* nested[8];
  char buf[4096];
};

static void netlink_init(struct nlmsg* nlmsg, int typ, int flags,
                         const void* data, int size)
{
  memset(nlmsg, 0, sizeof(*nlmsg));
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_type = typ;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
  memcpy(hdr + 1, data, size);
  nlmsg->pos = (char*)(hdr + 1) + NLMSG_ALIGN(size);
}

static void netlink_attr(struct nlmsg* nlmsg, int typ, const void* data,
                         int size)
{
  struct nlattr* attr = (struct nlattr*)nlmsg->pos;
  attr->nla_len = sizeof(*attr) + size;
  attr->nla_type = typ;
  if (size > 0)
    memcpy(attr + 1, data, size);
  nlmsg->pos += NLMSG_ALIGN(attr->nla_len);
}

static int netlink_send_ext(struct nlmsg* nlmsg, int sock, uint16_t reply_type,
                            int* reply_len, bool dofail)
{
  if (nlmsg->pos > nlmsg->buf + sizeof(nlmsg->buf) || nlmsg->nesting)
    exit(1);
  struct nlmsghdr* hdr = (struct nlmsghdr*)nlmsg->buf;
  hdr->nlmsg_len = nlmsg->pos - nlmsg->buf;
  struct sockaddr_nl addr;
  memset(&addr, 0, sizeof(addr));
  addr.nl_family = AF_NETLINK;
  ssize_t n = sendto(sock, nlmsg->buf, hdr->nlmsg_len, 0,
                     (struct sockaddr*)&addr, sizeof(addr));
  if (n != (ssize_t)hdr->nlmsg_len) {
    if (dofail)
      exit(1);
    return -1;
  }
  n = recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  if (reply_len)
    *reply_len = 0;
  if (n < 0) {
    if (dofail)
      exit(1);
    return -1;
  }
  if (n < (ssize_t)sizeof(struct nlmsghdr)) {
    errno = EINVAL;
    if (dofail)
      exit(1);
    return -1;
  }
  if (hdr->nlmsg_type == NLMSG_DONE)
    return 0;
  if (reply_len && hdr->nlmsg_type == reply_type) {
    *reply_len = n;
    return 0;
  }
  if (n < (ssize_t)(sizeof(struct nlmsghdr) + sizeof(struct nlmsgerr))) {
    errno = EINVAL;
    if (dofail)
      exit(1);
    return -1;
  }
  if (hdr->nlmsg_type != NLMSG_ERROR) {
    errno = EINVAL;
    if (dofail)
      exit(1);
    return -1;
  }
  errno = -((struct nlmsgerr*)(hdr + 1))->error;
  return -errno;
}

static int netlink_query_family_id(struct nlmsg* nlmsg, int sock,
                                   const char* family_name, bool dofail)
{
  struct genlmsghdr genlhdr;
  memset(&genlhdr, 0, sizeof(genlhdr));
  genlhdr.cmd = CTRL_CMD_GETFAMILY;
  netlink_init(nlmsg, GENL_ID_CTRL, 0, &genlhdr, sizeof(genlhdr));
  netlink_attr(nlmsg, CTRL_ATTR_FAMILY_NAME, family_name,
               strnlen(family_name, GENL_NAMSIZ - 1) + 1);
  int n = 0;
  int err = netlink_send_ext(nlmsg, sock, GENL_ID_CTRL, &n, dofail);
  if (err < 0) {
    return -1;
  }
  uint16_t id = 0;
  struct nlattr* attr = (struct nlattr*)(nlmsg->buf + NLMSG_HDRLEN +
                                         NLMSG_ALIGN(sizeof(genlhdr)));
  for (; (char*)attr < nlmsg->buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID) {
      id = *(uint16_t*)(attr + 1);
      break;
    }
  }
  if (!id) {
    errno = EINVAL;
    return -1;
  }
  recv(sock, nlmsg->buf, sizeof(nlmsg->buf), 0);
  return id;
}

const int kInitNetNsFd = 239;

static long syz_init_net_socket(volatile long domain, volatile long type,
                                volatile long proto)
{
  return syscall(__NR_socket, domain, type, proto);
}

static long syz_genetlink_get_family_id(volatile long name,
                                        volatile long sock_arg)
{
  bool dofail = false;
  int fd = sock_arg;
  if (fd < 0) {
    dofail = true;
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd == -1) {
      return -1;
    }
  }
  struct nlmsg nlmsg_tmp;
  int ret = netlink_query_family_id(&nlmsg_tmp, fd, (char*)name, dofail);
  if ((int)sock_arg < 0)
    close(fd);
  if (ret < 0) {
    return -1;
  }
  return ret;
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
      if (current_time_ms() - start < 5000) {
        continue;
      }
      kill_and_wait(pid, &status);
      break;
    }
  }
}

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0};

void execute_one(void)
{
  intptr_t res = 0;
  res = -1;
  res = syz_init_net_socket(0x10, 3, 0x10);
  if (res != -1)
    r[0] = res;
  res = -1;
  res = syz_init_net_socket(0x10, 3, 0x10);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000300, "nbd\000", 4);
  res = -1;
  res = syz_genetlink_get_family_id(0x20000300, r[1]);
  if (res != -1)
    r[2] = res;
  *(uint64_t*)0x20002580 = 0;
  *(uint32_t*)0x20002588 = 0;
  *(uint64_t*)0x20002590 = 0x20002540;
  *(uint64_t*)0x20002540 = 0x20002480;
  *(uint32_t*)0x20002480 = 0x8c;
  *(uint16_t*)0x20002484 = r[2];
  *(uint16_t*)0x20002486 = 0x3e91;
  *(uint32_t*)0x20002488 = 0x70bd2b;
  *(uint32_t*)0x2000248c = 0x25dfdbfc;
  *(uint8_t*)0x20002490 = 1;
  *(uint8_t*)0x20002491 = 0;
  *(uint16_t*)0x20002492 = 0;
  *(uint16_t*)0x20002494 = 0x14;
  STORE_BY_BITMASK(uint16_t, , 0x20002496, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x20002497, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x20002497, 1, 7, 1);
  *(uint16_t*)0x20002498 = 8;
  *(uint16_t*)0x2000249a = 1;
  *(uint32_t*)0x2000249c = -1;
  *(uint16_t*)0x200024a0 = 8;
  *(uint16_t*)0x200024a2 = 1;
  *(uint32_t*)0x200024a4 = -1;
  *(uint16_t*)0x200024a8 = 0x34;
  STORE_BY_BITMASK(uint16_t, , 0x200024aa, 7, 0, 14);
  STORE_BY_BITMASK(uint16_t, , 0x200024ab, 0, 6, 1);
  STORE_BY_BITMASK(uint16_t, , 0x200024ab, 1, 7, 1);
  *(uint16_t*)0x200024ac = 8;
  *(uint16_t*)0x200024ae = 1;
  *(uint32_t*)0x200024b0 = -1;
  *(uint16_t*)0x200024b4 = 8;
  *(uint16_t*)0x200024b6 = 1;
  *(uint32_t*)0x200024b8 = -1;
  *(uint16_t*)0x200024bc = 8;
  *(uint16_t*)0x200024be = 1;
  *(uint32_t*)0x200024c0 = -1;
  *(uint16_t*)0x200024c4 = 8;
  *(uint16_t*)0x200024c6 = 1;
  *(uint32_t*)0x200024c8 = -1;
  *(uint16_t*)0x200024cc = 8;
  *(uint16_t*)0x200024ce = 1;
  *(uint32_t*)0x200024d0 = -1;
  *(uint16_t*)0x200024d4 = 8;
  *(uint16_t*)0x200024d6 = 1;
  *(uint32_t*)0x200024d8 = -1;
  *(uint16_t*)0x200024dc = 0xc;
  *(uint16_t*)0x200024de = 6;
  *(uint64_t*)0x200024e0 = 1;
  *(uint16_t*)0x200024e8 = 0xc;
  *(uint16_t*)0x200024ea = 5;
  *(uint64_t*)0x200024ec = 1;
  *(uint16_t*)0x200024f4 = 0xc;
  *(uint16_t*)0x200024f6 = 2;
  *(uint64_t*)0x200024f8 = 4;
  *(uint16_t*)0x20002500 = 0xc;
  *(uint16_t*)0x20002502 = 8;
  *(uint64_t*)0x20002504 = 0xff;
  *(uint64_t*)0x20002548 = 0x8c;
  *(uint64_t*)0x20002598 = 1;
  *(uint64_t*)0x200025a0 = 0;
  *(uint64_t*)0x200025a8 = 0;
  *(uint32_t*)0x200025b0 = 0x40;
  syscall(__NR_sendmsg, r[0], 0x20002580ul, 0x2000800ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
