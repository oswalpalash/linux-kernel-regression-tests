// https://syzkaller.appspot.com/bug?id=4f3a544c4a1f113216b11517018d36d2007c85d6
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
#include <sys/ioctl.h>
#include <sys/mount.h>
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

static void use_temporary_dir(void)
{
  char tmpdir_template[] = "./syzkaller.XXXXXX";
  char* tmpdir = mkdtemp(tmpdir_template);
  if (!tmpdir)
    exit(1);
  if (chmod(tmpdir, 0777))
    exit(1);
  if (chdir(tmpdir))
    exit(1);
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

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  DIR* dp;
  struct dirent* ep;
  int iter = 0;
retry:
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    struct stat st;
    if (lstat(filename, &st))
      exit(1);
    if (S_ISDIR(st.st_mode)) {
      remove_dir(filename);
      continue;
    }
    int i;
    for (i = 0;; i++) {
      if (unlink(filename) == 0)
        break;
      if (errno == EPERM) {
        int fd = open(filename, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno != EBUSY || i > 100)
        exit(1);
    }
  }
  closedir(dp);
  int i;
  for (i = 0;; i++) {
    if (rmdir(dir) == 0)
      break;
    if (i < 100) {
      if (errno == EPERM) {
        int fd = open(dir, O_RDONLY);
        if (fd != -1) {
          long flags = 0;
          if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == 0) {
          }
          close(fd);
          continue;
        }
      }
      if (errno == EROFS) {
        break;
      }
      if (errno == EBUSY) {
        continue;
      }
      if (errno == ENOTEMPTY) {
        if (iter < 100) {
          iter++;
          goto retry;
        }
      }
    }
    exit(1);
  }
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
    char cwdbuf[32];
    sprintf(cwdbuf, "./%d", iter);
    if (mkdir(cwdbuf, 0777))
      exit(1);
    int pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) {
      if (chdir(cwdbuf))
        exit(1);
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
    remove_dir(cwdbuf);
  }
}

uint64_t r[7] = {0xffffffffffffffff, 0x0, 0xffffffffffffffff,
                 0xffffffffffffffff, 0x0, 0x0,
                 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    syscall(__NR_unshare, 0x40000000ul);
    break;
  case 1:
    syscall(__NR_mmap, 0x20000000ul, 0xb36000ul, 3ul, 0x8031ul, -1, 0ul);
    break;
  case 2:
    res = syscall(__NR_socket, 0x11ul, 0x800000003ul, 0);
    if (res != -1)
      r[0] = res;
    break;
  case 3:
    *(uint16_t*)0x20000280 = 0x11;
    memcpy((void*)0x20000282,
           "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a"
           "\x49\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01"
           "\x2e\x0b\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3"
           "\xff\x5f\x16\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00"
           "\x00\x01\x01\x01\x3c\x58\x11\x03\x08\xd9\x12\x31\x27\xec\xce\x66"
           "\xfd\x79\x2b\xbf\x0e\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00"
           "\x01\x00\x00\x00\x00\x00\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00"
           "\x00\x06\xad\x8e\x5e\xcc\x32\x6d\x3a\x09\xff\xc2\xc6\x54",
           126);
    syscall(__NR_bind, r[0], 0x20000280ul, 0x80ul);
    break;
  case 4:
    *(uint32_t*)0x20000140 = 0x14;
    res = syscall(__NR_getsockname, r[0], 0x20000000ul, 0x20000140ul);
    if (res != -1)
      r[1] = *(uint32_t*)0x20000004;
    break;
  case 5:
    *(uint16_t*)0x20000040 = 0x11;
    *(uint16_t*)0x20000042 = htobe16(0xf8);
    *(uint32_t*)0x20000044 = r[1];
    *(uint16_t*)0x20000048 = 1;
    *(uint8_t*)0x2000004a = 0;
    *(uint8_t*)0x2000004b = 6;
    *(uint8_t*)0x2000004c = 0xaa;
    *(uint8_t*)0x2000004d = 0xaa;
    *(uint8_t*)0x2000004e = 0xaa;
    *(uint8_t*)0x2000004f = 0xaa;
    *(uint8_t*)0x20000050 = 0xaa;
    *(uint8_t*)0x20000051 = 0xaa;
    *(uint8_t*)0x20000052 = 0;
    *(uint8_t*)0x20000053 = 0;
    syscall(__NR_bind, r[0], 0x20000040ul, 0x14ul);
    break;
  case 6:
    *(uint32_t*)0x20000200 = 0;
    res = syscall(__NR_accept, -1, 0ul, 0x20000200ul);
    if (res != -1)
      r[2] = res;
    break;
  case 7:
    res = syscall(__NR_socket, 0x10ul, 3ul, 0x10);
    if (res != -1)
      r[3] = res;
    break;
  case 8:
    memcpy((void*)0x20000080, "IPVS\000", 5);
    res = syz_genetlink_get_family_id(0x20000080);
    if (res != -1)
      r[4] = res;
    break;
  case 9:
    syscall(__NR_sendmsg, r[3], 0ul, 0ul);
    break;
  case 10:
    *(uint64_t*)0x20000400 = 0x20000480;
    *(uint16_t*)0x20000480 = 0x10;
    *(uint16_t*)0x20000482 = 0;
    *(uint32_t*)0x20000484 = 0;
    *(uint32_t*)0x20000488 = 0x4000000;
    *(uint32_t*)0x20000408 = 0xc;
    *(uint64_t*)0x20000410 = 0x200003c0;
    *(uint64_t*)0x200003c0 = 0x20000740;
    *(uint32_t*)0x20000740 = 0x94;
    *(uint16_t*)0x20000744 = r[4];
    *(uint16_t*)0x20000746 = 0x100;
    *(uint32_t*)0x20000748 = 0x70bd26;
    *(uint32_t*)0x2000074c = 0x25dfdbfd;
    *(uint8_t*)0x20000750 = 4;
    *(uint8_t*)0x20000751 = 0;
    *(uint16_t*)0x20000752 = 0;
    *(uint16_t*)0x20000754 = 8;
    *(uint16_t*)0x20000756 = 6;
    *(uint32_t*)0x20000758 = 3;
    *(uint16_t*)0x2000075c = 8;
    *(uint16_t*)0x2000075e = 4;
    *(uint32_t*)0x20000760 = 5;
    *(uint16_t*)0x20000764 = 8;
    *(uint16_t*)0x20000766 = 5;
    *(uint32_t*)0x20000768 = 3;
    *(uint16_t*)0x2000076c = 0x60;
    STORE_BY_BITMASK(uint16_t, , 0x2000076e, 1, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x2000076f, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x2000076f, 1, 7, 1);
    *(uint16_t*)0x20000770 = 0x14;
    *(uint16_t*)0x20000772 = 3;
    *(uint8_t*)0x20000774 = 0xfe;
    *(uint8_t*)0x20000775 = 0x80;
    *(uint8_t*)0x20000776 = 0;
    *(uint8_t*)0x20000777 = 0;
    *(uint8_t*)0x20000778 = 0;
    *(uint8_t*)0x20000779 = 0;
    *(uint8_t*)0x2000077a = 0;
    *(uint8_t*)0x2000077b = 0;
    *(uint8_t*)0x2000077c = 0;
    *(uint8_t*)0x2000077d = 0;
    *(uint8_t*)0x2000077e = 0;
    *(uint8_t*)0x2000077f = 0;
    *(uint8_t*)0x20000780 = 0;
    *(uint8_t*)0x20000781 = 0;
    *(uint8_t*)0x20000782 = 0;
    *(uint8_t*)0x20000783 = 0x17;
    *(uint16_t*)0x20000784 = 0xc;
    *(uint16_t*)0x20000786 = 7;
    *(uint32_t*)0x20000788 = 0;
    *(uint32_t*)0x2000078c = 0x10;
    *(uint16_t*)0x20000790 = 8;
    *(uint16_t*)0x20000792 = 5;
    *(uint32_t*)0x20000794 = 2;
    *(uint16_t*)0x20000798 = 0xc;
    *(uint16_t*)0x2000079a = 7;
    *(uint32_t*)0x2000079c = 0x12;
    *(uint32_t*)0x200007a0 = 0x29;
    *(uint16_t*)0x200007a4 = 0x14;
    *(uint16_t*)0x200007a6 = 3;
    *(uint8_t*)0x200007a8 = 0xac;
    *(uint8_t*)0x200007a9 = 0x14;
    *(uint8_t*)0x200007aa = 0x14;
    *(uint8_t*)0x200007ab = 0xe;
    *(uint16_t*)0x200007b8 = 0x14;
    *(uint16_t*)0x200007ba = 3;
    *(uint8_t*)0x200007bc = 0xac;
    *(uint8_t*)0x200007bd = 0x14;
    *(uint8_t*)0x200007be = 0x14;
    *(uint8_t*)0x200007bf = 0xaa;
    *(uint16_t*)0x200007cc = 8;
    *(uint16_t*)0x200007ce = 4;
    *(uint32_t*)0x200007d0 = 6;
    *(uint64_t*)0x200003c8 = 0x94;
    *(uint64_t*)0x20000418 = 1;
    *(uint64_t*)0x20000420 = 0;
    *(uint64_t*)0x20000428 = 0;
    *(uint32_t*)0x20000430 = 0;
    syscall(__NR_sendmsg, r[2], 0x20000400ul, 0x800ul);
    break;
  case 11:
    memcpy((void*)0x20000000,
           "lo\000\000\000\000\000\000\000\000\000\000\000\000\000\000", 16);
    *(uint32_t*)0x20000010 = r[1];
    res = syscall(__NR_ioctl, -1, 0x8914, 0x20000000ul);
    if (res != -1)
      r[5] = *(uint32_t*)0x20000010;
    break;
  case 12:
    res = syscall(__NR_openat, 0xffffff9c, 0ul, 0x275aul, 0ul);
    if (res != -1)
      r[6] = res;
    break;
  case 13:
    *(uint64_t*)0x20000240 = 0;
    *(uint64_t*)0x20000248 = 0;
    *(uint64_t*)0x20000250 = 0x4002be20000;
    syscall(__NR_ioctl, r[6], 0x40305829, 0x20000240ul);
    break;
  case 14:
    *(uint8_t*)0x20001a80 = 0;
    *(uint8_t*)0x20001a81 = 0;
    *(uint8_t*)0x20001a82 = 0;
    *(uint8_t*)0x20001a83 = 0;
    *(uint8_t*)0x20001a84 = 0;
    *(uint8_t*)0x20001a85 = 0;
    *(uint8_t*)0x20001a86 = 0;
    *(uint8_t*)0x20001a87 = 0;
    *(uint8_t*)0x20001a88 = 0;
    *(uint8_t*)0x20001a89 = 0;
    *(uint8_t*)0x20001a8a = -1;
    *(uint8_t*)0x20001a8b = -1;
    *(uint8_t*)0x20001a8c = 0xac;
    *(uint8_t*)0x20001a8d = 0x14;
    *(uint8_t*)0x20001a8e = 0x14;
    *(uint8_t*)0x20001a8f = 0xbb;
    *(uint32_t*)0x20001a90 = 0;
    syscall(__NR_setsockopt, -1, 0x29, 0x14, 0x20001a80ul, 0x14ul);
    break;
  case 15:
    syscall(__NR_sendmsg, r[2], 0ul, 4ul);
    break;
  case 16:
    *(uint64_t*)0x20000080 = 0;
    *(uint32_t*)0x20000088 = 0x25;
    *(uint64_t*)0x20000090 = 0x20000140;
    *(uint64_t*)0x20000140 = 0x20000340;
    *(uint32_t*)0x20000340 = 0x44;
    *(uint16_t*)0x20000344 = 0x10;
    *(uint16_t*)0x20000346 = 0x401;
    *(uint32_t*)0x20000348 = 0;
    *(uint32_t*)0x2000034c = 0;
    *(uint8_t*)0x20000350 = 0;
    *(uint8_t*)0x20000351 = 0;
    *(uint16_t*)0x20000352 = 0;
    *(uint32_t*)0x20000354 = 0;
    *(uint32_t*)0x20000358 = 0;
    *(uint32_t*)0x2000035c = 0;
    *(uint16_t*)0x20000360 = 0x14;
    STORE_BY_BITMASK(uint16_t, , 0x20000362, 0x12, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20000363, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20000363, 1, 7, 1);
    *(uint16_t*)0x20000364 = 0xc;
    *(uint16_t*)0x20000366 = 1;
    memcpy((void*)0x20000368, "macvlan\000", 8);
    *(uint16_t*)0x20000370 = 4;
    STORE_BY_BITMASK(uint16_t, , 0x20000372, 2, 0, 14);
    STORE_BY_BITMASK(uint16_t, , 0x20000373, 0, 6, 1);
    STORE_BY_BITMASK(uint16_t, , 0x20000373, 1, 7, 1);
    *(uint16_t*)0x20000374 = 8;
    *(uint16_t*)0x20000376 = 5;
    *(uint32_t*)0x20000378 = 0;
    *(uint16_t*)0x2000037c = 8;
    *(uint16_t*)0x2000037e = 0xa;
    *(uint32_t*)0x20000380 = 0;
    *(uint64_t*)0x20000148 = 0x44;
    *(uint64_t*)0x20000098 = 1;
    *(uint64_t*)0x200000a0 = 0;
    *(uint64_t*)0x200000a8 = 0;
    *(uint32_t*)0x200000b0 = 0;
    syscall(__NR_sendmsg, -1, 0x20000080ul, 0ul);
    break;
  case 17:
    *(uint64_t*)0x200005c0 = 0;
    *(uint32_t*)0x200005c8 = 0;
    *(uint64_t*)0x200005d0 = 0x20000580;
    *(uint64_t*)0x20000580 = 0x20000500;
    *(uint32_t*)0x20000500 = 0x48;
    *(uint16_t*)0x20000504 = 0;
    *(uint16_t*)0x20000506 = 0x8257;
    *(uint32_t*)0x20000508 = 0x70bd25;
    *(uint32_t*)0x2000050c = 0x25dfdbfd;
    *(uint8_t*)0x20000510 = 0x6b;
    *(uint8_t*)0x20000511 = 0;
    *(uint16_t*)0x20000512 = 0;
    *(uint16_t*)0x20000514 = 8;
    *(uint16_t*)0x20000516 = 3;
    *(uint32_t*)0x20000518 = 0;
    *(uint16_t*)0x2000051c = 0xa;
    *(uint16_t*)0x2000051e = 6;
    *(uint8_t*)0x20000520 = 0xbb;
    *(uint8_t*)0x20000521 = 0xbb;
    *(uint8_t*)0x20000522 = 0xbb;
    *(uint8_t*)0x20000523 = 0xbb;
    *(uint8_t*)0x20000524 = 0xbb;
    *(uint8_t*)0x20000525 = 0xbb;
    *(uint16_t*)0x20000528 = 0xa;
    *(uint16_t*)0x2000052a = 0x1a;
    *(uint8_t*)0x2000052c = 0xaa;
    *(uint8_t*)0x2000052d = 0xaa;
    *(uint8_t*)0x2000052e = 0xaa;
    *(uint8_t*)0x2000052f = 0xaa;
    *(uint8_t*)0x20000530 = 0xaa;
    *(uint8_t*)0x20000531 = 0xbb;
    *(uint16_t*)0x20000534 = 8;
    *(uint16_t*)0x20000536 = 3;
    *(uint32_t*)0x20000538 = r[5];
    *(uint16_t*)0x2000053c = 0xc;
    *(uint16_t*)0x2000053e = 0x99;
    *(uint32_t*)0x20000540 = 5;
    *(uint32_t*)0x20000544 = 1;
    *(uint64_t*)0x20000588 = 0x48;
    *(uint64_t*)0x200005d8 = 1;
    *(uint64_t*)0x200005e0 = 0;
    *(uint64_t*)0x200005e8 = 0;
    *(uint32_t*)0x200005f0 = 0x4040040;
    syscall(__NR_sendmsg, -1, 0x200005c0ul, 4ul);
    break;
  case 18:
    syscall(__NR_socket, 0xaul, 0x80002ul, 0);
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
      use_temporary_dir();
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
