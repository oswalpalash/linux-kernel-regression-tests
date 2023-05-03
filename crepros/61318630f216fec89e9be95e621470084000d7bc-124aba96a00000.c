// https://syzkaller.appspot.com/bug?id=61318630f216fec89e9be95e621470084000d7bc
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

static long syz_init_net_socket(volatile long domain, volatile long type,
                                volatile long proto)
{
  return syscall(__NR_socket, domain, type, proto);
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

#define KMEMLEAK_FILE "/sys/kernel/debug/kmemleak"

static void setup_leak()
{
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  sleep(5);
  if (!write_file(KMEMLEAK_FILE, "scan"))
    exit(1);
  if (!write_file(KMEMLEAK_FILE, "clear"))
    exit(1);
}

static void check_leaks(void)
{
  int fd = open(KMEMLEAK_FILE, O_RDWR);
  if (fd == -1)
    exit(1);
  uint64_t start = current_time_ms();
  if (write(fd, "scan", 4) != 4)
    exit(1);
  sleep(1);
  while (current_time_ms() - start < 4 * 1000)
    sleep(1);
  if (write(fd, "scan", 4) != 4)
    exit(1);
  static char buf[128 << 10];
  ssize_t n = read(fd, buf, sizeof(buf) - 1);
  if (n < 0)
    exit(1);
  int nleaks = 0;
  if (n != 0) {
    sleep(1);
    if (write(fd, "scan", 4) != 4)
      exit(1);
    if (lseek(fd, 0, SEEK_SET) < 0)
      exit(1);
    n = read(fd, buf, sizeof(buf) - 1);
    if (n < 0)
      exit(1);
    buf[n] = 0;
    char* pos = buf;
    char* end = buf + n;
    while (pos < end) {
      char* next = strstr(pos + 1, "unreferenced object");
      if (!next)
        next = end;
      char prev = *next;
      *next = 0;
      fprintf(stderr, "BUG: memory leak\n%s\n", pos);
      *next = prev;
      pos = next;
      nleaks++;
    }
  }
  if (write(fd, "clear", 5) != 5)
    exit(1);
  close(fd);
  if (nleaks)
    exit(1);
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
  for (call = 0; call < 19; call++) {
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
      event_timedwait(&th->done, 45);
      break;
    }
  }
  for (i = 0; i < 100 && __atomic_load_n(&running, __ATOMIC_RELAXED); i++)
    sleep_ms(1);
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
    check_leaks();
  }
}

uint64_t r[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0,
                 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, 0xa, 1, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    *(uint16_t*)0x20000180 = 0xa;
    *(uint16_t*)0x20000182 = htobe16(0x4e22);
    *(uint32_t*)0x20000184 = htobe32(0);
    *(uint64_t*)0x20000188 = htobe64(0);
    *(uint64_t*)0x20000190 = htobe64(1);
    *(uint32_t*)0x20000198 = 0;
    syscall(__NR_bind, r[0], 0x20000180, 0x1c);
    break;
  case 2:
    syscall(__NR_listen, r[0], 0);
    break;
  case 3:
    memcpy((void*)0x200001c0, "/dev/input/mouse#\000", 18);
    res = syz_open_dev(0x200001c0, 0xd32e, 0);
    if (res != -1)
      r[1] = res;
    break;
  case 4:
    memcpy((void*)0x20000300, "nbd\000", 4);
    res = syz_genetlink_get_family_id(0x20000300);
    if (res != -1)
      r[2] = res;
    break;
  case 5:
    memcpy((void*)0x20000340, "/dev/btrfs-control\000", 19);
    syscall(__NR_openat, 0xffffffffffffff9c, 0x20000340, 0, 0);
    break;
  case 6:
    syscall(__NR_dup2, r[0], r[0]);
    break;
  case 7:
    syscall(__NR_socketpair, 8, 0xa, 0x58, 0x20000380);
    break;
  case 8:
    memcpy((void*)0x200003c0, "/dev/btrfs-control\000", 19);
    syscall(__NR_openat, 0xffffffffffffff9c, 0x200003c0, 0x400, 0);
    break;
  case 9:
    memcpy((void*)0x20000400, "/dev/vsock\000", 11);
    syscall(__NR_openat, 0xffffffffffffff9c, 0x20000400, 0x18002, 0);
    break;
  case 10:
    *(uint32_t*)0x200004c0 = 0x80;
    syscall(__NR_accept, r[0], 0x20000440, 0x200004c0);
    break;
  case 11:
    *(uint64_t*)0x20000600 = 0;
    *(uint32_t*)0x20000608 = 0;
    *(uint64_t*)0x20000610 = 0x200005c0;
    *(uint64_t*)0x200005c0 = 0x20000500;
    *(uint32_t*)0x20000500 = 0x64;
    *(uint16_t*)0x20000504 = r[2];
    *(uint16_t*)0x20000506 = 0x300;
    *(uint32_t*)0x20000508 = 0x70bd25;
    *(uint32_t*)0x2000050c = 0x25dfdbfd;
    *(uint8_t*)0x20000510 = 2;
    *(uint8_t*)0x20000511 = 0;
    *(uint16_t*)0x20000512 = 0;
    *(uint16_t*)0x20000514 = 8;
    *(uint16_t*)0x20000516 = 1;
    *(uint32_t*)0x20000518 = 0;
    *(uint16_t*)0x2000051c = 0xc;
    *(uint16_t*)0x2000051e = 4;
    *(uint64_t*)0x20000520 = 0xffffffffffffff8b;
    *(uint16_t*)0x20000528 = 0xc;
    *(uint16_t*)0x2000052a = 6;
    *(uint64_t*)0x2000052c = 3;
    *(uint16_t*)0x20000534 = 0xc;
    *(uint16_t*)0x20000536 = 6;
    *(uint64_t*)0x20000538 = 3;
    *(uint16_t*)0x20000540 = 0xc;
    *(uint16_t*)0x20000542 = 5;
    *(uint64_t*)0x20000544 = 0x2b;
    *(uint16_t*)0x2000054c = 0xc;
    *(uint16_t*)0x2000054e = 5;
    *(uint64_t*)0x20000550 = 0;
    *(uint16_t*)0x20000558 = 0xc;
    *(uint16_t*)0x2000055a = 5;
    *(uint64_t*)0x2000055c = 0x120;
    *(uint64_t*)0x200005c8 = 0x64;
    *(uint64_t*)0x20000618 = 1;
    *(uint64_t*)0x20000620 = 0;
    *(uint64_t*)0x20000628 = 0;
    *(uint32_t*)0x20000630 = 0x20000000;
    syscall(__NR_sendmsg, r[1], 0x20000600, 0);
    break;
  case 12:
    syscall(__NR_socket, 0xa, 1, 0);
    break;
  case 13:
    res = syz_init_net_socket(0x1a, 1, 0);
    if (res != -1)
      r[3] = res;
    break;
  case 14:
    *(uint16_t*)0x20000040 = 0x1a;
    *(uint16_t*)0x20000042 = 0;
    *(uint8_t*)0x20000044 = 0x1f;
    *(uint8_t*)0x20000045 = 0;
    *(uint8_t*)0x20000046 = 0;
    *(uint8_t*)0x20000047 = 0;
    *(uint8_t*)0x20000048 = 0;
    *(uint8_t*)0x20000049 = 0;
    *(uint8_t*)0x2000004a = 0;
    *(uint8_t*)0x2000004b = 0;
    *(uint8_t*)0x2000004c = 0;
    *(uint8_t*)0x2000004d = 0;
    syscall(__NR_bind, r[3], 0x20000040, 0x10);
    break;
  case 15:
    *(uint64_t*)0x20000000 = 0;
    *(uint32_t*)0x20000008 = 0;
    *(uint64_t*)0x20000010 = 0x200002c0;
    *(uint64_t*)0x200002c0 = 0;
    *(uint64_t*)0x200002c8 = 0;
    *(uint64_t*)0x20000018 = 1;
    *(uint64_t*)0x20000020 = 0;
    *(uint64_t*)0x20000028 = 0;
    *(uint32_t*)0x20000030 = 0;
    syscall(__NR_sendmsg, r[3], 0x20000000, 0);
    break;
  case 16:
    syscall(__NR_gettid);
    break;
  case 17:
    syscall(__NR_timer_create, 0, 0, 0);
    break;
  case 18:
    syscall(__NR_timer_settime, 0, 0, 0, 0);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  setup_leak();
  loop();
  return 0;
}
