// https://syzkaller.appspot.com/bug?id=dcab7a4f12b3669f9dc70d72a7b7be739a53aca2
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
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
#include <linux/if_addr.h>
#include <linux/if_link.h>
#include <linux/in6.h>
#include <linux/neighbour.h>
#include <linux/net.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/veth.h>

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

static long syz_genetlink_get_family_id(volatile long name,
                                        volatile long sock_arg)
{
  int fd = sock_arg;
  if (fd < 0) {
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (fd == -1) {
      return -1;
    }
  }
  struct nlmsg nlmsg_tmp;
  int ret = netlink_query_family_id(&nlmsg_tmp, fd, (char*)name, false);
  if ((int)sock_arg < 0)
    close(fd);
  if (ret < 0) {
    return -1;
  }
  return ret;
}

#define FS_IOC_SETFLAGS _IOW('f', 2, long)
static void remove_dir(const char* dir)
{
  int iter = 0;
  DIR* dp = 0;
  const int umount_flags = MNT_FORCE | UMOUNT_NOFOLLOW;

retry:
  while (umount2(dir, umount_flags) == 0) {
  }
  dp = opendir(dir);
  if (dp == NULL) {
    if (errno == EMFILE) {
      exit(1);
    }
    exit(1);
  }
  struct dirent* ep = 0;
  while ((ep = readdir(dp))) {
    if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
      continue;
    char filename[FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", dir, ep->d_name);
    while (umount2(filename, umount_flags) == 0) {
    }
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
      if (umount2(filename, umount_flags))
        exit(1);
    }
  }
  closedir(dp);
  for (int i = 0;; i++) {
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
        if (umount2(dir, umount_flags))
          exit(1);
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
  if (symlink("/dev/binderfs", "./binderfs")) {
  }
}

static void setup_sysctl()
{
  int cad_pid = fork();
  if (cad_pid < 0)
    exit(1);
  if (cad_pid == 0) {
    for (;;)
      sleep(100);
  }
  char tmppid[32];
  snprintf(tmppid, sizeof(tmppid), "%d", cad_pid);
  struct {
    const char* name;
    const char* data;
  } files[] = {
      {"/sys/kernel/debug/x86/nmi_longest_ns", "10000000000"},
      {"/proc/sys/kernel/hung_task_check_interval_secs", "20"},
      {"/proc/sys/net/core/bpf_jit_kallsyms", "1"},
      {"/proc/sys/net/core/bpf_jit_harden", "0"},
      {"/proc/sys/kernel/kptr_restrict", "0"},
      {"/proc/sys/kernel/softlockup_all_cpu_backtrace", "1"},
      {"/proc/sys/fs/mount-max", "100"},
      {"/proc/sys/vm/oom_dump_tasks", "0"},
      {"/proc/sys/debug/exception-trace", "0"},
      {"/proc/sys/kernel/printk", "7 4 1 3"},
      {"/proc/sys/kernel/keys/gc_delay", "1"},
      {"/proc/sys/vm/oom_kill_allocating_task", "1"},
      {"/proc/sys/kernel/ctrl-alt-del", "0"},
      {"/proc/sys/kernel/cad_pid", tmppid},
  };
  for (size_t i = 0; i < sizeof(files) / sizeof(files[0]); i++) {
    if (!write_file(files[i].name, files[i].data)) {
    }
  }
  kill(cad_pid, SIGKILL);
  while (waitpid(cad_pid, NULL, 0) != cad_pid)
    ;
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
  if (write(1, "executing program\n", sizeof("executing program\n") - 1)) {
  }
  int i, call, thread;
  for (call = 0; call < 12; call++) {
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
      event_timedwait(&th->done, 50);
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
  int iter = 0;
  for (;; iter++) {
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
      sleep_ms(10);
      if (waitpid(-1, &status, WNOHANG | WAIT_FLAGS) == pid)
        break;
      if (current_time_ms() - start < 5000)
        continue;
      kill_and_wait(pid, &status);
      break;
    }
    remove_dir(cwdbuf);
  }
}

uint64_t r[6] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff,
                 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

void execute_call(int call)
{
  intptr_t res = 0;
  switch (call) {
  case 0:
    res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0x10);
    if (res != -1)
      r[0] = res;
    break;
  case 1:
    memcpy((void*)0x20000600, "TIPCv2\000", 7);
    res = -1;
    res = syz_genetlink_get_family_id(/*name=*/0x20000600, /*fd=*/-1);
    if (res != -1)
      r[1] = res;
    break;
  case 2:
    *(uint64_t*)0x20000780 = 0;
    *(uint32_t*)0x20000788 = 0;
    *(uint64_t*)0x20000790 = 0x20000740;
    *(uint64_t*)0x20000740 = 0x20000a00;
    memcpy((void*)0x20000a00, "\xc4\x00\x00\x00", 4);
    *(uint16_t*)0x20000a04 = r[1];
    memcpy(
        (void*)0x20000a06,
        "\x01\x00\x2a\xbd\x70\x00\xfb\xdb\xdf\x25\x17\x00\x00\x00\x04\x00\x05"
        "\x80\x04\x00\x05\x80\x48\x00\x06\x80\x44\x00\x04\x00\x67\x63\x6d\x28"
        "\x61\x65\x73\x29\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1c\x00\x00\x00\xce\x96"
        "\x8c\x98\x4a\x4e\xe3\x99\xf7\x73\x08\xdc\x21\x96\x96\x5f\x09\x52\x3e"
        "\x5e\x02\x72\x9f\x11\x93\x31\x9f\x62\x34\x00\x09\x80\x08\x00\x02\x00"
        "\x9a\x08\x00\x00\x08\x00\x01\x00\x00\x40\x00\x00\x08\x00\x01\x00\xdc"
        "\x08\x00\x00\x08\x00\x02\x00\x05\x00\x00\x00\x08\x00\x02\x00\x07\x00"
        "\x00\x00\x08\x00\x01\x00\xff\x0d\x00\x00\x18\x00\x07\x80\x0c\x00\x03"
        "\x00\x03\x00\x00\x00\x00\x00\x00\x00\x08\x00\x02\x00\x00\x00\x00\x00"
        "\x14\x00\x03\x80\x08\x00\x01\x00\x09\x31\x76\x00\x08\x00\x01\x00\x00"
        "\x08\x00\x00\xda\x70\xc4\x6b\x7e\x9d\x06\x40\x4a\x87\x27\x64\x5b\x60"
        "\xac\x87\x2a\x1b\x70\xb9\x33\x23\x5d\xf3\x7f\x76\x29\x04\xce\xa4\x7a"
        "\xb5\xf1\x36\x32\x49\x35\x25\x97\xb8\x67\x32\x5e\x42\x91\x96\x4a\x84"
        "\x42\xa0\x30\x9a\x09\xdf\x6e\x3c\x6f\x49\x82\x2b\x23\xf4\x4e\x35\x80"
        "\xaa\x14\xac\xa3\x72\x3c\xae\xf2\xf2\xe5\x53\xcd\xde\xab\x1c\x4a\x88"
        "\x53\x44\xb3\xa5\x41\xc6\x02\xb3\x2f\xad\x97\x8b\x8e\x75\xb3\x35\x1b"
        "\x4e\x49\xdc\x8e\xb5\xbb\x55\xf2\x28\x13\x88\x68\x19\x8f\x8e\xd5\x11"
        "\xd0\xf2\x10\x9a\xf3\x0e\xef\xa9\x63\xd7\x66\xe8\x46\xf5\x9b\x0e\x7e"
        "\xe8\x6f\x1f\x6c\xc7\xfd\xfe\x41\xb0\x59\xd5\x7b\xab\xce\x82\x8f\xd4"
        "\xb1\x15\x81\xb9\x5e\x43\xc3\xda\x02\x72\x77\x64\xc7\xe0\x31\x94\x7e"
        "\x56\x63\xb1\x51\x17\x66\x07\xb2\xed\x00\x6d\xd5\x79\x42\x36\xa3\x11"
        "\x3b\x6e\xe0\x80\x1c\x03\x41\x85\xa8\x44\xe9\x8e\xa7\xe1\x23\x12\x35"
        "\x75\xff\x17\x9a\x72\x04\x8c\x46\x27\x6c\x79",
        402);
    *(uint64_t*)0x20000748 = 0xc4;
    *(uint64_t*)0x20000798 = 1;
    *(uint64_t*)0x200007a0 = 0;
    *(uint64_t*)0x200007a8 = 0;
    *(uint32_t*)0x200007b0 = 0x20000000;
    syscall(__NR_sendmsg, /*fd=*/r[0], /*msg=*/0x20000780ul,
            /*f=MSG_PROBE*/ 0x10ul);
    break;
  case 3:
    memcpy((void*)0x20000080, "/dev/net/tun\000", 13);
    res = syscall(
        __NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000080ul,
        /*flags=O_TRUNC|O_SYNC|O_NOCTTY|O_NOATIME|O_CREAT|O_CLOEXEC|0x1*/
        0x1c1341, /*mode=*/0);
    if (res != -1)
      r[2] = res;
    break;
  case 4:
    memcpy((void*)0x200000c0, "syzkaller0\000\000\000\000\000\000", 16);
    *(uint16_t*)0x200000d0 = 0xb7f2;
    syscall(__NR_ioctl, /*fd=*/r[2], /*cmd=*/0x400454ca, /*arg=*/0x200000c0ul);
    break;
  case 5:
    memcpy((void*)0x20000400, "/dev/net/tun\000", 13);
    res = syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul,
                  /*file=*/0x20000400ul, /*flags=*/0, /*mode=*/0);
    if (res != -1)
      r[3] = res;
    break;
  case 6:
    syscall(__NR_close, /*fd=*/r[3]);
    break;
  case 7:
    res = syscall(__NR_socket, /*domain=*/0x10ul, /*type=*/3ul, /*proto=*/0x10);
    if (res != -1)
      r[4] = res;
    break;
  case 8:
    memcpy((void*)0x20000200, "TIPC\000", 5);
    res = -1;
    res = syz_genetlink_get_family_id(/*name=*/0x20000200, /*fd=*/-1);
    if (res != -1)
      r[5] = res;
    break;
  case 9:
    *(uint64_t*)0x200002c0 = 0;
    *(uint32_t*)0x200002c8 = 0;
    *(uint64_t*)0x200002d0 = 0x20000280;
    *(uint64_t*)0x20000280 = 0x20000680;
    memcpy((void*)0x20000680, "8\000\000\000", 4);
    *(uint16_t*)0x20000684 = r[5];
    memcpy((void*)0x20000686,
           "\x01\x00\x00\x00\x0d\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00"
           "\x00\x00\x01\x41\x00\x00\x00\x1c\x00\x17\x00\x00\x00\x00\x00\x00"
           "\x00\x00\x65\x74\x68\x3a\x73\x79\x7a\x6b\x61\x6c\x6c\x65\x72\x30",
           48);
    *(uint64_t*)0x20000288 = 0x38;
    *(uint64_t*)0x200002d8 = 1;
    *(uint64_t*)0x200002e0 = 0;
    *(uint64_t*)0x200002e8 = 0;
    *(uint32_t*)0x200002f0 = 0;
    syscall(__NR_sendmsg, /*fd=*/r[4], /*msg=*/0x200002c0ul, /*f=*/0ul);
    break;
  case 10:
    memcpy((void*)0x20002280, "syzkaller0\000\000\000\000\000\000", 16);
    memset((void*)0x20002290, 187, 6);
    syscall(__NR_ioctl, /*fd=*/r[3], /*cmd=*/0x8914, /*arg=*/0x20002280ul);
    break;
  case 11:
    *(uint64_t*)0x200001c0 = 0x20000000;
    memcpy((void*)0x20000000,
           "\x89\xe7\xee\x2c\x7c\xda\xd9\xb4\xb4\x73\x80\xc9\x88\xca\xfb\xe8"
           "\x63\xca\xc5\x05\x80\x31\xc9\x7d\x70\xcc\xd0\xf7\x6d\xb9",
           30);
    *(uint64_t*)0x200001c8 = 0x1e;
    *(uint64_t*)0x200001d0 = 0x20000100;
    memcpy((void*)0x20000100,
           "\xe3\xf4\xe0\x1b\xb6\x4d\x88\xaf\xf4\xc9\x92\x18\xe5\x45\x0d\xf5"
           "\x39\x2e\x79\x78\x9c\x39\x68\xb1\xc7\x2f\xc3\x09\x61",
           29);
    *(uint64_t*)0x200001d8 = 0x1d;
    syscall(__NR_writev, /*fd=*/r[2], /*vec=*/0x200001c0ul, /*vlen=*/2ul);
    break;
  }
}
int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  setup_sysctl();
  const char* reason;
  (void)reason;
  for (procid = 0; procid < 5; procid++) {
    if (fork() == 0) {
      use_temporary_dir();
      loop();
    }
  }
  sleep(1000000);
  return 0;
}
