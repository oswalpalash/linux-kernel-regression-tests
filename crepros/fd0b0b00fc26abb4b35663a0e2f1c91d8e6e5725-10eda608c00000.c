// https://syzkaller.appspot.com/bug?id=fd0b0b00fc26abb4b35663a0e2f1c91d8e6e5725
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
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
#include <linux/netlink.h>

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

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;
  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];
  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];
  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

static long syz_init_net_socket(long domain, long type, long proto)
{
  return syscall(__NR_socket, domain, type, proto);
}

static long syz_genetlink_get_family_id(long name)
{
  char buf[512] = {0};
  struct nlmsghdr* hdr = (struct nlmsghdr*)buf;
  struct genlmsghdr* genlhdr = (struct genlmsghdr*)NLMSG_DATA(hdr);
  struct nlattr* attr = (struct nlattr*)(genlhdr + 1);
  hdr->nlmsg_len =
      sizeof(*hdr) + sizeof(*genlhdr) + sizeof(*attr) + GENL_NAMSIZ;
  hdr->nlmsg_type = GENL_ID_CTRL;
  hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
  genlhdr->cmd = CTRL_CMD_GETFAMILY;
  attr->nla_type = CTRL_ATTR_FAMILY_NAME;
  attr->nla_len = sizeof(*attr) + GENL_NAMSIZ;
  NONFAILING(strncpy((char*)(attr + 1), (char*)name, GENL_NAMSIZ));
  struct iovec iov = {hdr, hdr->nlmsg_len};
  struct sockaddr_nl addr = {0};
  addr.nl_family = AF_NETLINK;
  int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
  if (fd == -1) {
    return -1;
  }
  struct msghdr msg = {&addr, sizeof(addr), &iov, 1, NULL, 0, 0};
  if (sendmsg(fd, &msg, 0) == -1) {
    close(fd);
    return -1;
  }
  ssize_t n = recv(fd, buf, sizeof(buf), 0);
  close(fd);
  if (n <= 0) {
    return -1;
  }
  if (hdr->nlmsg_type != GENL_ID_CTRL) {
    return -1;
  }
  for (; (char*)attr < buf + n;
       attr = (struct nlattr*)((char*)attr + NLMSG_ALIGN(attr->nla_len))) {
    if (attr->nla_type == CTRL_ATTR_FAMILY_ID)
      return *(uint16_t*)(attr + 1);
  }
  return -1;
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
}

#define SYZ_HAVE_RESET_TEST 1
static void reset_test()
{
  int fd;
  for (fd = 3; fd < 30; fd++)
    close(fd);
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
      reset_test();
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

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff};

void execute_one(void)
{
  long res = 0;
  res = syz_init_net_socket(0xb, 5, 0);
  if (res != -1)
    r[0] = res;
  syz_init_net_socket(0xb, 5, 0);
  NONFAILING(memcpy((void*)0x200001c0,
                    "rose0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16));
  NONFAILING(*(uint16_t*)0x200001d0 = -1);
  syscall(__NR_ioctl, r[0], 0x8914, 0x200001c0);
  syscall(__NR_unshare, 0x24020400);
  syscall(__NR_unshare, 0x4020000);
  syscall(__NR_getsockopt, 0xffffff9c, 1, 0x11, 0x20001280, 0);
  syscall(__NR_bind, -1, 0, 0);
  syscall(__NR_sendmmsg, -1, 0, 0, 0x4000);
  syscall(__NR_unshare, 0x24020400);
  syscall(__NR_unshare, 0x401fffc);
  syscall(__NR_socket, 2, 2, 0x88);
  NONFAILING(memcpy((void*)0x20000b00,
                    "bcsf0\x00\x00\x00\x00\x00\x00h\x11\x00\x00\x00", 16));
  syscall(__NR_ioctl, -1, 0x89a1, 0x20000b00);
  syscall(__NR_socketpair, 0x11, 4, 0, 0);
  syscall(__NR_getsockopt, -1, 0x29, 0x22, 0, 0);
  syscall(__NR_socket, 0x26, 5, 0);
  syscall(__NR_unshare, 0x24020400);
  syscall(__NR_unshare, 0x401fffc);
  NONFAILING(*(uint64_t*)0x20000580 = 0);
  syscall(__NR_getsockopt, -1, 0x112, 9, 0, 0x20000580);
  syscall(__NR_writev, -1, 0, 0);
  NONFAILING(*(uint16_t*)0x20000040 = 2);
  NONFAILING(*(uint16_t*)0x20000042 = htobe16(0));
  NONFAILING(*(uint32_t*)0x20000044 = htobe32(0));
  syscall(__NR_bind, -1, 0x20000040, 0x10);
  res = syscall(__NR_openat, 0xffffff9c, 0, 0x275a, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_mmap, 0x20f44000, 0x50707c, 0x1000006, 0x2013, r[1], 0);
  syscall(__NR_mmap, 0x20000000, 0xe7e000, 0, 0x40031, -1, 0);
  res = syscall(__NR_socket, 0xa, 2, 0);
  if (res != -1)
    r[2] = res;
  syscall(__NR_epoll_wait, -1, 0, 0, 2);
  NONFAILING(*(uint64_t*)0x20008380 = 0);
  NONFAILING(*(uint32_t*)0x20008388 = 0);
  NONFAILING(*(uint64_t*)0x20008390 = 0);
  NONFAILING(*(uint64_t*)0x20008398 = 0);
  NONFAILING(*(uint64_t*)0x200083a0 = 0);
  NONFAILING(*(uint64_t*)0x200083a8 = 0);
  NONFAILING(*(uint32_t*)0x200083b0 = 0);
  NONFAILING(*(uint32_t*)0x200083b8 = 0);
  syscall(__NR_sendmmsg, -1, 0x20008380, 1, 0);
  syscall(__NR_connect, r[2], 0, 0);
  syscall(__NR_connect, -1, 0, 0);
  syscall(__NR_sendmmsg, r[2], 0x20007e00, 0x136a88c8311572c, 0);
  res = syscall(__NR_socket, 2, 2, 0x88);
  if (res != -1)
    r[3] = res;
  syscall(__NR_ioctl, r[3], 0x1000008912, 0);
  NONFAILING(*(uint8_t*)0x20007000 = 0xaa);
  NONFAILING(*(uint8_t*)0x20007001 = 0xaa);
  NONFAILING(*(uint8_t*)0x20007002 = 0xaa);
  NONFAILING(*(uint8_t*)0x20007003 = 0xaa);
  NONFAILING(*(uint8_t*)0x20007004 = 0xaa);
  NONFAILING(*(uint8_t*)0x20007005 = 0xaa);
  NONFAILING(memcpy((void*)0x20007006, "\xcf\x2b\xb4\x3c\x40\xb8", 6));
  NONFAILING(*(uint16_t*)0x2000700c = htobe16(0x800));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000700e, 6, 0, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000700e, 4, 4, 4));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000700f, 0, 0, 2));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000700f, 0, 2, 6));
  NONFAILING(*(uint16_t*)0x20007010 = htobe16(0x222));
  NONFAILING(*(uint16_t*)0x20007012 = htobe16(0));
  NONFAILING(*(uint16_t*)0x20007014 = htobe16(0));
  NONFAILING(*(uint8_t*)0x20007016 = 0);
  NONFAILING(*(uint8_t*)0x20007017 = 0x21);
  NONFAILING(*(uint16_t*)0x20007018 = htobe16(0));
  NONFAILING(*(uint8_t*)0x2000701a = 0xac);
  NONFAILING(*(uint8_t*)0x2000701b = 0x14);
  NONFAILING(*(uint8_t*)0x2000701c = -1);
  NONFAILING(*(uint8_t*)0x2000701d = 0xaa);
  NONFAILING(*(uint32_t*)0x2000701e = htobe32(0xe0000001));
  NONFAILING(*(uint16_t*)0x20007022 = htobe16(0));
  NONFAILING(*(uint16_t*)0x20007024 = htobe16(0));
  NONFAILING(*(uint32_t*)0x20007026 = 0x42424242);
  NONFAILING(*(uint32_t*)0x2000702a = 0x42424242);
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000702e, 0, 0, 1));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000702e, 3, 1, 3));
  NONFAILING(STORE_BY_BITMASK(uint8_t, , 0x2000702e, 8, 4, 4));
  NONFAILING(*(uint8_t*)0x2000702f = 0);
  NONFAILING(*(uint16_t*)0x20007030 = htobe16(0));
  NONFAILING(*(uint16_t*)0x20007032 = htobe16(0));
  NONFAILING(*(uint16_t*)0x20007034 = htobe16(0));
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x0, 4));
  NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x0, 4));
  uint16_t csum_1_chunk_2 = 0x600;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 2);
  uint16_t csum_1_chunk_3 = 0x1400;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 2);
  NONFAILING(csum_inet_update(&csum_1, (const uint8_t*)0x0, 20));
  NONFAILING(*(uint16_t*)0x20007032 = csum_inet_digest(&csum_1));
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  NONFAILING(csum_inet_update(&csum_2, (const uint8_t*)0x2000700e, 20));
  NONFAILING(*(uint16_t*)0x20007018 = csum_inet_digest(&csum_2));
  NONFAILING(memcpy((void*)0x20000040, "TIPC\x00", 5));
  syz_genetlink_get_family_id(0x20000040);
  syscall(__NR_socket, 2, 2, 0x88);
  NONFAILING(*(uint8_t*)0x20000540 = 9);
  syscall(__NR_getsockopt, -1, 0x112, 9, 0x20000540, 0);
  NONFAILING(*(uint32_t*)0x20005080 = 3);
  NONFAILING(*(uint16_t*)0x20005088 = 2);
  NONFAILING(*(uint16_t*)0x2000508a = htobe16(0x4e20));
  NONFAILING(*(uint32_t*)0x2000508c = htobe32(0));
  syscall(__NR_setsockopt, -1, 0, 0x2a, 0x20005080, 0x88);
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  install_segv_handler();
  loop();
  return 0;
}
