// https://syzkaller.appspot.com/bug?id=301b758c0f45c2498f0ffb1e9c91b362c5441d9a
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

#include <linux/futex.h>
#include <linux/genetlink.h>
#include <linux/netlink.h>

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

static void thread_start(void* (*fn)(void*), void* arg)
{
  pthread_t th;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 128 << 10);
  int i = 0;
  for (; i < 100; i++) {
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

#define BITMASK(bf_off, bf_len) (((1ull << (bf_len)) - 1) << (bf_off))
#define STORE_BY_BITMASK(type, htobe, addr, val, bf_off, bf_len)               \
  *(type*)(addr) =                                                             \
      htobe((htobe(*(type*)(addr)) & ~BITMASK((bf_off), (bf_len))) |           \
            (((type)(val) << (bf_off)) & BITMASK((bf_off), (bf_len))))

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
  syscall(SYS_futex, &ev->state, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1000000);
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
    if (__atomic_load_n(&ev->state, __ATOMIC_ACQUIRE))
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

static long syz_genetlink_get_family_id(volatile long name)
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
  strncpy((char*)(attr + 1), (char*)name, GENL_NAMSIZ);
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
  for (call = 0; call < 9; call++) {
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

uint64_t r[6] = {0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0x0,
                 0xffffffffffffffff, 0x0};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    memcpy((void*)0x20000040, "TIPCv2\000", 7);
    res = -1;
    res = syz_genetlink_get_family_id(0x20000040);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    *(uint64_t*)0x20000400 = 0;
    *(uint32_t*)0x20000408 = 0;
    *(uint64_t*)0x20000410 = 0x200003c0;
    *(uint64_t*)0x200003c0 = 0x200001c0;
    memcpy((void*)0x200001c0, "\\\000\000\000", 4);
    *(uint16_t*)0x200001c4 = r[1];
    memcpy((void*)0x200001c6,
           "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x17\x00\x00\x00\x48\x00"
           "\x06\x80\x44\x00\x04\x00\x67\x63\x6d\x28\x61\x65\x73\x29\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x1c\x00\x00\x00\x9d\x86\x2b\x0e\x79\x14"
           "\x61\xd8\xdb\xc5\xc8\xcb\xa7\x05\xb2\x14\xfa\xe7\x37\x2a\x84\xb6"
           "\x71\x77\xcc\x1b\x79\x13\x10\x28\x16\x48\x98\x7b\x30\x0e\xf1\xd0"
           "\xd4\xd1\x46\x18\x1d\xe2\x2d\x32\xc4\x04\x9f\x56\xb4\x89\xca\x2c"
           "\x40\x07\xff\x31\xe3\x1c\xbf\xdb\xaf\x00\x5d\xb0\xf1\x28\x5e\x86"
           "\x6a\xfe\x6f\xaa\x66\x34\x69\xc2\x29\xd4\x05\xe6\xf3\x1a\x40\xd6"
           "\x5a\xd8\x58\xbb\x8b\x37\x18\xf4\x9c\x4a\x9d\x7e\xb5\x5a\x55\x3f"
           "\x5e\x38\x67\x0f\xae\xfa\x36\x2e\x5d\xf8\x9a\xdb\x65\x32\x3a\x72"
           "\x17\xa5\xaf\xc0\xe3\xf3\x70\x4d\xff\x64\xa7\xd3\x51\x93\x87\x05"
           "\xaa\xc6\xe9\x54\xed\xb9\x88\x01\x31\x6a\xe9\x87\x35\x81\xbc\xde"
           "\xf9\x71\x59\x06\xd8\xaf\xe8\xb2\x3b\x2d\xfd\x35\xa6\x13\x05\x83"
           "\x35\xdc\xe0\x59\x6f\x73\x57\xdb\xad\x09\xd0\xb9\xda\xd1\x5d\x8b"
           "\x11\xe6\xdc\x7c\xc9\xec\xc1\x2d\xb0\xdf\x4f\x77\x69\xbe\xcd\x04"
           "\x01",
           257);
    *(uint64_t*)0x200003c8 = 0x5c;
    *(uint64_t*)0x20000418 = 1;
    *(uint64_t*)0x20000420 = 0;
    *(uint64_t*)0x20000428 = 0;
    *(uint32_t*)0x20000430 = 0;
    syscall(__NR_sendmsg, r[0], 0x20000400ul, 0ul);
    break;
  case 3:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
    if (res != -1)
      r[2] = res;
    break;
  case 4:
    memcpy((void*)0x20000040, "TIPCv2\000", 7);
    res = -1;
    res = syz_genetlink_get_family_id(0x20000040);
    if (res != -1)
      r[3] = res;
    break;
  case 5:
    *(uint64_t*)0x20000300 = 0;
    *(uint32_t*)0x20000308 = 0;
    *(uint64_t*)0x20000310 = 0x200002c0;
    *(uint64_t*)0x200002c0 = 0x200000c0;
    *(uint32_t*)0x200000c0 = 0x1d0;
    *(uint16_t*)0x200000c4 = r[3];
    *(uint16_t*)0x200000c6 = 1;
    *(uint32_t*)0x200000c8 = 0;
    *(uint32_t*)0x200000cc = 0x25dfdbfd;
    *(uint8_t*)0x200000d0 = 3;
    *(uint8_t*)0x200000d1 = 0;
    *(uint16_t*)0x200000d2 = 0;
    *(uint16_t*)0x200000d4 = 0x3c;
    STORE_BY_BITMASK(uint16_t, , 0x200000d6, 7, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x200000d7, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x200000d7, 1, 7, 1);
    *(uint16_t*)0x200000d8 = 8;
    *(uint16_t*)0x200000da = 2;
    *(uint32_t*)0x200000dc = 0x200;
    *(uint16_t*)0x200000e0 = 8;
    *(uint16_t*)0x200000e2 = 1;
    *(uint32_t*)0x200000e4 = 0x22de;
    *(uint16_t*)0x200000e8 = 8;
    *(uint16_t*)0x200000ea = 1;
    *(uint32_t*)0x200000ec = 9;
    *(uint16_t*)0x200000f0 = 8;
    *(uint16_t*)0x200000f2 = 2;
    *(uint32_t*)0x200000f4 = 3;
    *(uint16_t*)0x200000f8 = 8;
    *(uint16_t*)0x200000fa = 2;
    *(uint32_t*)0x200000fc = 7;
    *(uint16_t*)0x20000100 = 8;
    *(uint16_t*)0x20000102 = 2;
    *(uint32_t*)0x20000104 = 1;
    *(uint16_t*)0x20000108 = 8;
    *(uint16_t*)0x2000010a = 2;
    *(uint32_t*)0x2000010c = 8;
    *(uint16_t*)0x20000110 = 0xd8;
    STORE_BY_BITMASK(uint16_t, , 0x20000112, 6, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20000113, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20000113, 1, 7, 1);
    *(uint16_t*)0x20000114 = 8;
    *(uint16_t*)0x20000116 = 1;
    *(uint32_t*)0x20000118 = 0x100;
    *(uint16_t*)0x2000011c = 4;
    *(uint16_t*)0x2000011e = 2;
    *(uint16_t*)0x20000120 = 8;
    *(uint16_t*)0x20000122 = 1;
    *(uint32_t*)0x20000124 = 9;
    *(uint16_t*)0x20000128 = 0x44;
    *(uint16_t*)0x2000012a = 4;
    memcpy((void*)0x2000012c, "gcm(aes)"
                              "\000\000\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000\000\000\000\000\000"
                              "\000",
           32);
    *(uint32_t*)0x2000014c = 0x1c;
    memcpy((void*)0x20000150, "\x66\x99\x82\x06\x7e\xf3\xcc\xf5\xb7\x11\x98\xbd"
                              "\xdc\x82\x14\x4a\xb6\x36\x52\xeb\x30\xbd\x22\xf0"
                              "\x92\xa4\x79\x3a",
           28);
    *(uint16_t*)0x2000016c = 4;
    *(uint16_t*)0x2000016e = 2;
    *(uint16_t*)0x20000170 = 0x73;
    *(uint16_t*)0x20000172 = 3;
    memcpy((void*)0x20000174,
           "\x5d\x27\xc9\x8a\x00\xcb\xbd\x69\xc3\x00\x6b\x48\xa6\x08\x37\x2e"
           "\xa2\xa4\x58\x06\x17\xc2\x96\xfd\x52\x31\xa1\x7b\xe9\xc5\x0c\xa6"
           "\xc3\xd9\x93\x75\x68\x01\x0b\xaa\x03\x39\x7a\xd3\x5a\x73\x71\xf1"
           "\x32\xc9\x0e\x99\xb3\xa7\x8d\x27\x8e\xc2\x9c\x14\x6e\x32\x07\x97"
           "\x02\x5e\x20\x20\x4d\x44\x8c\xd8\x56\x59\x93\x3f\xa0\x16\xd6\xff"
           "\x42\x1d\x17\x12\x04\xcc\x5d\x52\xb3\x7e\xb7\xe7\xa7\x9c\x2b\xaf"
           "\x17\x13\x09\x71\x0a\xe8\x70\x51\xd9\xcf\x41\x18\xa4\x57\x75",
           111);
    *(uint16_t*)0x200001e4 = 4;
    *(uint16_t*)0x200001e6 = 3;
    *(uint16_t*)0x200001e8 = 0x94;
    STORE_BY_BITMASK(uint16_t, , 0x200001ea, 1, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x200001eb, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x200001eb, 1, 7, 1);
    *(uint16_t*)0x200001ec = 8;
    *(uint16_t*)0x200001ee = 3;
    *(uint32_t*)0x200001f0 = 3;
    *(uint16_t*)0x200001f4 = 0x44;
    *(uint16_t*)0x200001f6 = 4;
    *(uint16_t*)0x200001f8 = 0x20;
    *(uint16_t*)0x200001fa = 1;
    *(uint16_t*)0x200001fc = 0xa;
    *(uint16_t*)0x200001fe = htobe16(0x4e21);
    *(uint32_t*)0x20000200 = htobe32(0x3015);
    *(uint64_t*)0x20000204 = htobe64(0);
    *(uint64_t*)0x2000020c = htobe64(1);
    *(uint32_t*)0x20000214 = 8;
    *(uint16_t*)0x20000218 = 0x20;
    *(uint16_t*)0x2000021a = 2;
    *(uint16_t*)0x2000021c = 0xa;
    *(uint16_t*)0x2000021e = htobe16(0x4e20);
    *(uint32_t*)0x20000220 = htobe32(0xffff7fff);
    *(uint8_t*)0x20000224 = 0xfc;
    *(uint8_t*)0x20000225 = 1;
    *(uint8_t*)0x20000226 = 0;
    *(uint8_t*)0x20000227 = 0;
    *(uint8_t*)0x20000228 = 0;
    *(uint8_t*)0x20000229 = 0;
    *(uint8_t*)0x2000022a = 0;
    *(uint8_t*)0x2000022b = 0;
    *(uint8_t*)0x2000022c = 0;
    *(uint8_t*)0x2000022d = 0;
    *(uint8_t*)0x2000022e = 0;
    *(uint8_t*)0x2000022f = 0;
    *(uint8_t*)0x20000230 = 0;
    *(uint8_t*)0x20000231 = 0;
    *(uint8_t*)0x20000232 = 0;
    *(uint8_t*)0x20000233 = 1;
    *(uint32_t*)0x20000234 = 7;
    *(uint16_t*)0x20000238 = 0x12;
    *(uint16_t*)0x2000023a = 1;
    memcpy((void*)0x2000023c, "ib", 2);
    *(uint8_t*)0x2000023e = 0x3a;
    memcpy((void*)0x2000023f, "veth1_vlan\000", 11);
    *(uint16_t*)0x2000024c = 0xf;
    *(uint16_t*)0x2000024e = 1;
    memcpy((void*)0x20000250, "ib", 2);
    *(uint8_t*)0x20000252 = 0x3a;
    memcpy((void*)0x20000253, "erspan0\000", 8);
    *(uint16_t*)0x2000025c = 0xd;
    *(uint16_t*)0x2000025e = 1;
    memcpy((void*)0x20000260, "udp:syz1\000", 9);
    *(uint16_t*)0x2000026c = 0xd;
    *(uint16_t*)0x2000026e = 1;
    memcpy((void*)0x20000270, "udp:syz1\000", 9);
    *(uint16_t*)0x2000027c = 0x14;
    STORE_BY_BITMASK(uint16_t, , 0x2000027e, 9, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x2000027f, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x2000027f, 1, 7, 1);
    *(uint16_t*)0x20000280 = 8;
    *(uint16_t*)0x20000282 = 1;
    *(uint32_t*)0x20000284 = 8;
    *(uint16_t*)0x20000288 = 8;
    *(uint16_t*)0x2000028a = 1;
    *(uint32_t*)0x2000028c = 0x10000;
    *(uint64_t*)0x200002c8 = 0x1d0;
    *(uint64_t*)0x20000318 = 1;
    *(uint64_t*)0x20000320 = 0;
    *(uint64_t*)0x20000328 = 0;
    *(uint32_t*)0x20000330 = 0x40010;
    syscall(__NR_sendmsg, r[2], 0x20000300ul, 0ul);
    break;
  case 6:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
    if (res != -1)
      r[4] = res;
    break;
  case 7:
    memcpy((void*)0x20000040, "TIPCv2\000", 7);
    res = -1;
    res = syz_genetlink_get_family_id(0x20000040);
    if (res != -1)
      r[5] = res;
    break;
  case 8:
    *(uint64_t*)0x20000300 = 0;
    *(uint32_t*)0x20000308 = 0;
    *(uint64_t*)0x20000310 = 0x200002c0;
    *(uint64_t*)0x200002c0 = 0x200000c0;
    *(uint32_t*)0x200000c0 = 0x124;
    *(uint16_t*)0x200000c4 = r[5];
    *(uint16_t*)0x200000c6 = 1;
    *(uint32_t*)0x200000c8 = 0;
    *(uint32_t*)0x200000cc = 0;
    *(uint8_t*)0x200000d0 = 3;
    *(uint8_t*)0x200000d1 = 0;
    *(uint16_t*)0x200000d2 = 0;
    *(uint16_t*)0x200000d4 = 0x3c;
    STORE_BY_BITMASK(uint16_t, , 0x200000d6, 7, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x200000d7, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x200000d7, 1, 7, 1);
    *(uint16_t*)0x200000d8 = 8;
    *(uint16_t*)0x200000da = 2;
    *(uint32_t*)0x200000dc = 0x200;
    *(uint16_t*)0x200000e0 = 8;
    *(uint16_t*)0x200000e2 = 1;
    *(uint32_t*)0x200000e4 = 0x22de;
    *(uint16_t*)0x200000e8 = 8;
    *(uint16_t*)0x200000ea = 1;
    *(uint32_t*)0x200000ec = 9;
    *(uint16_t*)0x200000f0 = 8;
    *(uint16_t*)0x200000f2 = 2;
    *(uint32_t*)0x200000f4 = 3;
    *(uint16_t*)0x200000f8 = 8;
    *(uint16_t*)0x200000fa = 2;
    *(uint32_t*)0x200000fc = 7;
    *(uint16_t*)0x20000100 = 8;
    *(uint16_t*)0x20000102 = 2;
    *(uint32_t*)0x20000104 = 1;
    *(uint16_t*)0x20000108 = 8;
    *(uint16_t*)0x2000010a = 2;
    *(uint32_t*)0x2000010c = 8;
    *(uint16_t*)0x20000110 = 0x60;
    STORE_BY_BITMASK(uint16_t, , 0x20000112, 6, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20000113, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20000113, 1, 7, 1);
    *(uint16_t*)0x20000114 = 8;
    *(uint16_t*)0x20000116 = 1;
    *(uint32_t*)0x20000118 = 0x100;
    *(uint16_t*)0x2000011c = 4;
    *(uint16_t*)0x2000011e = 2;
    *(uint16_t*)0x20000120 = 8;
    *(uint16_t*)0x20000122 = 1;
    *(uint32_t*)0x20000124 = 9;
    *(uint16_t*)0x20000128 = 0x3c;
    *(uint16_t*)0x2000012a = 4;
    memcpy((void*)0x2000012c, "gcm(aes)"
                              "\000\000\000\000\000\000\000\000\000\000\000\000"
                              "\000\000\000\000\000\000\000\000\000\000\000"
                              "\000",
           32);
    *(uint32_t*)0x2000014c = 0x14;
    memcpy((void*)0x20000150, "\x66\x99\x82\x06\x7e\xf3\xcc\xf5\xb7\x11\x98\xbd"
                              "\xdc\x82\x14\x4a\xb6\x36\x52\xeb",
           20);
    *(uint16_t*)0x20000164 = 4;
    *(uint16_t*)0x20000166 = 2;
    *(uint16_t*)0x20000168 = 4;
    *(uint16_t*)0x2000016a = 3;
    *(uint16_t*)0x2000016c = 4;
    *(uint16_t*)0x2000016e = 3;
    *(uint16_t*)0x20000170 = 0x74;
    STORE_BY_BITMASK(uint16_t, , 0x20000172, 1, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20000173, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20000173, 1, 7, 1);
    *(uint16_t*)0x20000174 = 8;
    *(uint16_t*)0x20000176 = 3;
    *(uint32_t*)0x20000178 = 3;
    *(uint16_t*)0x2000017c = 0x44;
    *(uint16_t*)0x2000017e = 4;
    *(uint16_t*)0x20000180 = 0x20;
    *(uint16_t*)0x20000182 = 1;
    *(uint16_t*)0x20000184 = 0xa;
    *(uint16_t*)0x20000186 = htobe16(0x4e21);
    *(uint32_t*)0x20000188 = htobe32(0x3015);
    *(uint64_t*)0x2000018c = htobe64(0);
    *(uint64_t*)0x20000194 = htobe64(1);
    *(uint32_t*)0x2000019c = 8;
    *(uint16_t*)0x200001a0 = 0x20;
    *(uint16_t*)0x200001a2 = 2;
    *(uint16_t*)0x200001a4 = 0xa;
    *(uint16_t*)0x200001a6 = htobe16(0x4e20);
    *(uint32_t*)0x200001a8 = htobe32(0xffff7fff);
    *(uint8_t*)0x200001ac = 0xfc;
    *(uint8_t*)0x200001ad = 1;
    *(uint8_t*)0x200001ae = 0;
    *(uint8_t*)0x200001af = 0;
    *(uint8_t*)0x200001b0 = 0;
    *(uint8_t*)0x200001b1 = 0;
    *(uint8_t*)0x200001b2 = 0;
    *(uint8_t*)0x200001b3 = 0;
    *(uint8_t*)0x200001b4 = 0;
    *(uint8_t*)0x200001b5 = 0;
    *(uint8_t*)0x200001b6 = 0;
    *(uint8_t*)0x200001b7 = 0;
    *(uint8_t*)0x200001b8 = 0;
    *(uint8_t*)0x200001b9 = 0;
    *(uint8_t*)0x200001ba = 0;
    *(uint8_t*)0x200001bb = 1;
    *(uint32_t*)0x200001bc = 7;
    *(uint16_t*)0x200001c0 = 0x12;
    *(uint16_t*)0x200001c2 = 1;
    memcpy((void*)0x200001c4, "ib", 2);
    *(uint8_t*)0x200001c6 = 0x3a;
    memcpy((void*)0x200001c7, "veth1_vlan\000", 11);
    *(uint16_t*)0x200001d4 = 0xf;
    *(uint16_t*)0x200001d6 = 1;
    memcpy((void*)0x200001d8, "ib", 2);
    *(uint8_t*)0x200001da = 0x3a;
    memcpy((void*)0x200001db, "erspan0\000", 8);
    *(uint64_t*)0x200002c8 = 0x124;
    *(uint64_t*)0x20000318 = 1;
    *(uint64_t*)0x20000320 = 0;
    *(uint64_t*)0x20000328 = 0;
    *(uint32_t*)0x20000330 = 0x40010;
    syscall(__NR_sendmsg, r[4], 0x20000300ul, 0ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  for (procid = 0; procid < 6; procid++) {
    if (fork() == 0) {
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
