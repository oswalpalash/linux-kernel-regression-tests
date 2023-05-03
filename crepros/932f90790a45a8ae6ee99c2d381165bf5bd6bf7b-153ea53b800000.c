// https://syzkaller.appspot.com/bug?id=932f90790a45a8ae6ee99c2d381165bf5bd6bf7b
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <linux/futex.h>
#include <linux/genetlink.h>
#include <linux/netlink.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static uintptr_t syz_genetlink_get_family_id(uintptr_t name)
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

static void execute_one();
extern unsigned long long procid;

void loop()
{
  while (1) {
    execute_one();
  }
}

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;
static int collide;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        if (collide && call % 2)
          break;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[10] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                  0xffffffffffffffff};
unsigned long long procid;
void execute_call(int call)
{
  long res;
  switch (call) {
  case 0:
    break;
  case 1:
    syscall(__NR_clone, 0, 0x207d1fff, 0x20001ffc, 0x20915ffc, 0x202d5fff);
    break;
  case 2:
    syscall(__NR_read, -1, 0x20009f9c, 0x64);
    break;
  case 3:
    *(uint32_t*)0x20000100 = 0x80;
    syscall(__NR_accept4, -1, 0x20000000, 0x20000100, 0);
    break;
  case 4:
    memcpy((void*)0x20000180, "IPVS", 5);
    syz_genetlink_get_family_id(0x20000180);
    break;
  case 5:
    res = syscall(__NR_socket, 2, 1, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 6:
    res = syscall(__NR_socket, 0x29, 2, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 7:
    *(uint32_t*)0x200002c0 = r[0];
    res = syscall(__NR_ioctl, r[1], 0x89e2, 0x200002c0);
    if (res != -1)
      r[2] = *(uint32_t*)0x200002c0;
    break;
  case 8:
    *(uint32_t*)0x20000080 = r[0];
    syscall(__NR_ioctl, r[2], 0x89e1, 0x20000080);
    break;
  case 9:
    res = syscall(__NR_socket, 0xa, 6, 0);
    if (res != -1)
      r[3] = res;
    break;
  case 10:
    *(uint16_t*)0x20000000 = 0xa;
    *(uint16_t*)0x20000002 = htobe16(3);
    *(uint32_t*)0x20000004 = 0;
    *(uint8_t*)0x20000008 = 0;
    *(uint8_t*)0x20000009 = 0;
    *(uint8_t*)0x2000000a = 0;
    *(uint8_t*)0x2000000b = 0;
    *(uint8_t*)0x2000000c = 0;
    *(uint8_t*)0x2000000d = 0;
    *(uint8_t*)0x2000000e = 0;
    *(uint8_t*)0x2000000f = 0;
    *(uint8_t*)0x20000010 = 0;
    *(uint8_t*)0x20000011 = 0;
    *(uint8_t*)0x20000012 = 0;
    *(uint8_t*)0x20000013 = 0;
    *(uint8_t*)0x20000014 = 0;
    *(uint8_t*)0x20000015 = 0;
    *(uint8_t*)0x20000016 = 0;
    *(uint8_t*)0x20000017 = 0;
    *(uint32_t*)0x20000018 = 0;
    syscall(__NR_bind, r[3], 0x20000000, 0x1c);
    break;
  case 11:
    syscall(__NR_listen, r[3], 0x1000100);
    break;
  case 12:
    res = syscall(__NR_socket, 2, 6, 0);
    if (res != -1)
      r[4] = res;
    break;
  case 13:
    *(uint16_t*)0x20772000 = 2;
    *(uint16_t*)0x20772002 = htobe16(3);
    *(uint32_t*)0x20772004 = htobe32(0);
    *(uint8_t*)0x20772008 = 0;
    *(uint8_t*)0x20772009 = 0;
    *(uint8_t*)0x2077200a = 0;
    *(uint8_t*)0x2077200b = 0;
    *(uint8_t*)0x2077200c = 0;
    *(uint8_t*)0x2077200d = 0;
    *(uint8_t*)0x2077200e = 0;
    *(uint8_t*)0x2077200f = 0;
    syscall(__NR_connect, r[4], 0x20772000, 0x10);
    break;
  case 14:
    *(uint64_t*)0x20001b00 = 0x20000180;
    *(uint16_t*)0x20000180 = 0x26;
    memcpy((void*)0x20000182,
           "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00", 14);
    *(uint32_t*)0x20000190 = 0;
    *(uint32_t*)0x20000194 = 0;
    memcpy((void*)0x20000198,
           "\x63\x74\x72\x28\x61\x65\x73\x2d\x61\x65\x73\x6e\x69\x29\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    *(uint32_t*)0x20001b08 = 0x80;
    *(uint64_t*)0x20001b10 = 0x20000300;
    *(uint64_t*)0x20001b18 = 0;
    *(uint64_t*)0x20001b20 = 0x20000340;
    *(uint64_t*)0x20001b28 = 0;
    *(uint32_t*)0x20001b30 = 0;
    *(uint32_t*)0x20001b38 = 0;
    syscall(__NR_sendmmsg, r[4], 0x20001b00, 1, 0);
    break;
  case 15:
    *(uint32_t*)0x20001100 = 0x80;
    res = syscall(__NR_accept, r[3], 0x20001080, 0x20001100);
    if (res != -1)
      r[5] = res;
    break;
  case 16:
    *(uint64_t*)0x20001840 = 0x20001140;
    *(uint32_t*)0x20001848 = 0x80;
    *(uint64_t*)0x20001850 = 0x200016c0;
    *(uint64_t*)0x20001858 = 0;
    *(uint64_t*)0x20001860 = 0x20001740;
    *(uint64_t*)0x20001868 = 0xeb;
    *(uint32_t*)0x20001870 = 0;
    syscall(__NR_recvmsg, r[5], 0x20001840, 0x60);
    break;
  case 17:
    memcpy((void*)0x20000080, "./cgroup.cpu", 13);
    syscall(__NR_openat, 0xffffffffffffff9c, 0x20000080, 0x200002, 0);
    break;
  case 18:
    syscall(__NR_setsockopt, -1, 0x117, 1, 0x20ff8000, 0);
    break;
  case 19:
    *(uint32_t*)0x20000000 = 1;
    *(uint32_t*)0x20000004 = 5;
    *(uint32_t*)0x20000008 = 3;
    *(uint32_t*)0x2000000c = 5;
    *(uint32_t*)0x20000010 = 0;
    *(uint32_t*)0x20000014 = -1;
    *(uint32_t*)0x20000018 = 0;
    *(uint8_t*)0x2000001c = 0;
    *(uint8_t*)0x2000001d = 0;
    *(uint8_t*)0x2000001e = 0;
    *(uint8_t*)0x2000001f = 0;
    *(uint8_t*)0x20000020 = 0;
    *(uint8_t*)0x20000021 = 0;
    *(uint8_t*)0x20000022 = 0;
    *(uint8_t*)0x20000023 = 0;
    *(uint8_t*)0x20000024 = 0;
    *(uint8_t*)0x20000025 = 0;
    *(uint8_t*)0x20000026 = 0;
    *(uint8_t*)0x20000027 = 0;
    *(uint8_t*)0x20000028 = 0;
    *(uint8_t*)0x20000029 = 0;
    *(uint8_t*)0x2000002a = 0;
    *(uint8_t*)0x2000002b = 0;
    syscall(__NR_bpf, 0, 0x20000000, 0x2c);
    break;
  case 20:
    syscall(__NR_socket, 2, 6, 0);
    break;
  case 21:
    *(uint64_t*)0x20001840 = 0x20001140;
    *(uint32_t*)0x20001848 = 0x80;
    *(uint64_t*)0x20001850 = 0x200016c0;
    *(uint64_t*)0x20001858 = 0;
    *(uint64_t*)0x20001860 = 0x20001740;
    *(uint64_t*)0x20001868 = 0xeb;
    *(uint32_t*)0x20001870 = 0;
    syscall(__NR_recvmsg, -1, 0x20001840, 0x60);
    break;
  case 22:
    syscall(__NR_socket, 0xa, 6, 0);
    break;
  case 23:
    *(uint16_t*)0x20772000 = 2;
    *(uint16_t*)0x20772002 = htobe16(3);
    *(uint32_t*)0x20772004 = htobe32(0);
    *(uint8_t*)0x20772008 = 0;
    *(uint8_t*)0x20772009 = 0;
    *(uint8_t*)0x2077200a = 0;
    *(uint8_t*)0x2077200b = 0;
    *(uint8_t*)0x2077200c = 0;
    *(uint8_t*)0x2077200d = 0;
    *(uint8_t*)0x2077200e = 0;
    *(uint8_t*)0x2077200f = 0;
    syscall(__NR_connect, -1, 0x20772000, 0x10);
    break;
  case 24:
    syscall(__NR_socket, 0x26, 5, 0);
    break;
  case 25:
    res = syscall(__NR_socket, 2, 6, 0);
    if (res != -1)
      r[6] = res;
    break;
  case 26:
    *(uint16_t*)0x20772000 = 2;
    *(uint16_t*)0x20772002 = htobe16(3);
    *(uint32_t*)0x20772004 = htobe32(0);
    *(uint8_t*)0x20772008 = 0;
    *(uint8_t*)0x20772009 = 0;
    *(uint8_t*)0x2077200a = 0;
    *(uint8_t*)0x2077200b = 0;
    *(uint8_t*)0x2077200c = 0;
    *(uint8_t*)0x2077200d = 0;
    *(uint8_t*)0x2077200e = 0;
    *(uint8_t*)0x2077200f = 0;
    syscall(__NR_connect, r[6], 0x20772000, 0x10);
    break;
  case 27:
    *(uint64_t*)0x20001b00 = 0x20000180;
    *(uint16_t*)0x20000180 = 0x26;
    memcpy((void*)0x20000182,
           "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00", 14);
    *(uint32_t*)0x20000190 = 0;
    *(uint32_t*)0x20000194 = 0;
    memcpy((void*)0x20000198,
           "\x63\x74\x72\x28\x61\x65\x73\x2d\x61\x65\x73\x6e\x69\x29\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    *(uint32_t*)0x20001b08 = 0x80;
    *(uint64_t*)0x20001b10 = 0x20000300;
    *(uint64_t*)0x20001b18 = 0;
    *(uint64_t*)0x20001b20 = 0x20000340;
    *(uint64_t*)0x20001b28 = 0;
    *(uint32_t*)0x20001b30 = 0;
    *(uint32_t*)0x20001b38 = 0;
    syscall(__NR_sendmmsg, r[6], 0x20001b00, 1, 0);
    break;
  case 28:
    memcpy((void*)0x20000000, "cpuset.cpus", 12);
    res = syscall(__NR_openat, -1, 0x20000000, 2, 0);
    if (res != -1)
      r[7] = res;
    break;
  case 29:
    *(uint8_t*)0x20000040 = 0x37;
    *(uint8_t*)0x20000041 = 0x2d;
    *(uint8_t*)0x20000042 = 0x36;
    syscall(__NR_write, r[7], 0x20000040, 3);
    break;
  case 30:
    res = syscall(__NR_socket, 0xa, 6, 0);
    if (res != -1)
      r[8] = res;
    break;
  case 31:
    syscall(__NR_listen, r[8], 0x1000100);
    break;
  case 32:
    *(uint64_t*)0x20001b00 = 0x20000180;
    *(uint16_t*)0x20000180 = 0x26;
    memcpy((void*)0x20000182,
           "\x73\x6b\x63\x69\x70\x68\x65\x72\x00\x00\x00\x00\x00\x00", 14);
    *(uint32_t*)0x20000190 = 0;
    *(uint32_t*)0x20000194 = 0;
    memcpy((void*)0x20000198,
           "\x63\x74\x72\x28\x61\x65\x73\x2d\x61\x65\x73\x6e\x69\x29\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
           64);
    *(uint32_t*)0x20001b08 = 0x80;
    *(uint64_t*)0x20001b10 = 0x20000300;
    *(uint64_t*)0x20001b18 = 0;
    *(uint64_t*)0x20001b20 = 0x20000340;
    *(uint64_t*)0x20001b28 = 0;
    *(uint32_t*)0x20001b30 = 0;
    *(uint32_t*)0x20001b38 = 0;
    syscall(__NR_sendmmsg, -1, 0x20001b00, 1, 0);
    break;
  case 33:
    res = syscall(__NR_socket, 2, 2, 0);
    if (res != -1)
      r[9] = res;
    break;
  case 34:
    *(uint32_t*)0x20000000 = htobe32(0xe0000001);
    *(uint32_t*)0x20000004 = htobe32(0);
    syscall(__NR_setsockopt, r[9], 0, 0x23, 0x20000000, 8);
    break;
  }
}

void execute_one()
{
  execute(35);
  collide = 1;
  execute(35);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  for (procid = 0; procid < 8; procid++) {
    if (fork() == 0) {
      for (;;) {
        loop();
      }
    }
  }
  sleep(1000000);
  return 0;
}
