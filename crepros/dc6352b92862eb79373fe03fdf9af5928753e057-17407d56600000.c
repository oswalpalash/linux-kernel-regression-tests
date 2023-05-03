// https://syzkaller.appspot.com/bug?id=dc6352b92862eb79373fe03fdf9af5928753e057
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <linux/futex.h>
#include <linux/genetlink.h>
#include <linux/netlink.h>

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

static void loop(void)
{
  int i, call, thread;
  int collide = 0;
again:
  for (call = 0; call < 3; call++) {
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

uint64_t r[2] = {0xffffffffffffffff, 0x0};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0x10, 3, 0x10);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    memcpy((void*)0x20000280, "TIPCv2\000", 7);
    res = syz_genetlink_get_family_id(0x20000280);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    *(uint64_t*)0x20000000 = 0;
    *(uint32_t*)0x20000008 = 0x600;
    *(uint64_t*)0x20000010 = 0x200000c0;
    *(uint64_t*)0x200000c0 = 0x20000100;
    *(uint32_t*)0x20000100 = 0x6c;
    *(uint16_t*)0x20000104 = r[1];
    *(uint16_t*)0x20000106 = 1;
    *(uint32_t*)0x20000108 = 0;
    *(uint32_t*)0x2000010c = 0;
    *(uint8_t*)0x20000110 = 3;
    *(uint8_t*)0x20000111 = 0;
    *(uint16_t*)0x20000112 = 0;
    *(uint16_t*)0x20000114 = 0x58;
    *(uint16_t*)0x20000116 = 1;
    *(uint16_t*)0x20000118 = 0x10;
    *(uint16_t*)0x2000011a = 1;
    memcpy((void*)0x2000011c, "udp:syz0\000", 9);
    *(uint16_t*)0x20000128 = 0x44;
    *(uint16_t*)0x2000012a = 4;
    *(uint16_t*)0x2000012c = 0x20;
    *(uint16_t*)0x2000012e = 1;
    *(uint16_t*)0x20000130 = 0xa;
    *(uint16_t*)0x20000132 = htobe16(0);
    *(uint32_t*)0x20000134 = htobe32(0);
    *(uint8_t*)0x20000138 = -1;
    *(uint8_t*)0x20000139 = 3;
    *(uint8_t*)0x2000013a = 0;
    *(uint8_t*)0x2000013b = 0;
    *(uint8_t*)0x2000013c = 0;
    *(uint8_t*)0x2000013d = 0;
    *(uint8_t*)0x2000013e = 0;
    *(uint8_t*)0x2000013f = 0;
    *(uint8_t*)0x20000140 = 0;
    *(uint8_t*)0x20000141 = 0;
    *(uint8_t*)0x20000142 = 0;
    *(uint8_t*)0x20000143 = 0;
    *(uint8_t*)0x20000144 = 0;
    *(uint8_t*)0x20000145 = 0;
    *(uint8_t*)0x20000146 = 0;
    *(uint8_t*)0x20000147 = 1;
    *(uint32_t*)0x20000148 = 0;
    *(uint16_t*)0x2000014c = 0x20;
    *(uint16_t*)0x2000014e = 2;
    *(uint16_t*)0x20000150 = 0xa;
    *(uint16_t*)0x20000152 = htobe16(0);
    *(uint32_t*)0x20000154 = htobe32(0);
    *(uint8_t*)0x20000158 = -1;
    *(uint8_t*)0x20000159 = 1;
    *(uint8_t*)0x2000015a = 0;
    *(uint8_t*)0x2000015b = 0;
    *(uint8_t*)0x2000015c = 0;
    *(uint8_t*)0x2000015d = 0;
    *(uint8_t*)0x2000015e = 0;
    *(uint8_t*)0x2000015f = 0;
    *(uint8_t*)0x20000160 = 0;
    *(uint8_t*)0x20000161 = 0;
    *(uint8_t*)0x20000162 = 0;
    *(uint8_t*)0x20000163 = 0;
    *(uint8_t*)0x20000164 = 0;
    *(uint8_t*)0x20000165 = 0;
    *(uint8_t*)0x20000166 = 0;
    *(uint8_t*)0x20000167 = 1;
    *(uint32_t*)0x20000168 = 0;
    *(uint64_t*)0x200000c8 = 0x6c;
    *(uint64_t*)0x20000018 = 1;
    *(uint64_t*)0x20000020 = 0;
    *(uint64_t*)0x20000028 = 0;
    *(uint32_t*)0x20000030 = 0;
    syscall(__NR_sendmsg, r[0], 0x20000000, 0);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
