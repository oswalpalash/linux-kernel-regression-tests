// https://syzkaller.appspot.com/bug?id=8f642fdfde0614fc3d7ae0eb665d91058b285058
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <dirent.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

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

uint64_t r[1] = {0xffffffffffffffff};

void execute_one(void)
{
  intptr_t res = 0;
  memcpy((void*)0x20000040, "/dev/vmci\000", 10);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000040ul, 2ul, 0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000140 = 0xa0000;
  syscall(__NR_ioctl, r[0], 0x7a7, 0x20000140ul);
  *(uint32_t*)0x200000c0 = 2;
  *(uint32_t*)0x200000c4 = 0;
  syscall(__NR_ioctl, r[0], 0x7a0, 0x200000c0ul);
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint16_t*)0x20000008 = 0;
  *(uint16_t*)0x2000000a = 1;
  *(uint32_t*)0x2000000c = 0;
  syscall(__NR_ioctl, r[0], 0x7a5, 0x20000000ul);
  *(uint64_t*)0x20001540 = 0x20000540;
  *(uint32_t*)0x20000540 = 0;
  *(uint32_t*)0x20000544 = 0;
  *(uint32_t*)0x20000548 = 0;
  *(uint32_t*)0x2000054c = 0;
  *(uint32_t*)0x20000550 = 0;
  *(uint32_t*)0x20000554 = 0;
  *(uint32_t*)0x20000558 = 0;
  *(uint32_t*)0x2000055c = 0;
  *(uint32_t*)0x20000560 = 0;
  *(uint32_t*)0x20000564 = 0;
  *(uint32_t*)0x20000568 = 0;
  *(uint32_t*)0x2000056c = 0;
  *(uint32_t*)0x20000570 = 0;
  *(uint32_t*)0x20000574 = 0;
  *(uint32_t*)0x20000578 = 0;
  *(uint32_t*)0x2000057c = 0;
  *(uint32_t*)0x20000580 = 0;
  *(uint32_t*)0x20000584 = 0;
  *(uint32_t*)0x20000588 = 0;
  *(uint32_t*)0x2000058c = 0;
  *(uint32_t*)0x20000590 = 0;
  *(uint32_t*)0x20000594 = 0;
  *(uint32_t*)0x20000598 = 0;
  *(uint32_t*)0x2000059c = 0;
  *(uint32_t*)0x200005a0 = 0;
  *(uint32_t*)0x200005a4 = 0;
  *(uint32_t*)0x200005a8 = 0;
  *(uint32_t*)0x200005ac = 0;
  *(uint32_t*)0x200005b0 = 0;
  *(uint32_t*)0x200005b4 = 0;
  *(uint32_t*)0x200005b8 = 0;
  *(uint32_t*)0x200005bc = 0;
  *(uint32_t*)0x200005c0 = 0;
  *(uint32_t*)0x200005c4 = 0;
  *(uint32_t*)0x200005c8 = 0;
  *(uint32_t*)0x200005cc = 0;
  *(uint32_t*)0x200005d0 = 0;
  *(uint32_t*)0x200005d4 = 0;
  *(uint32_t*)0x200005d8 = 0;
  *(uint32_t*)0x200005dc = 0;
  *(uint32_t*)0x200005e0 = 0;
  *(uint32_t*)0x200005e4 = 0;
  *(uint32_t*)0x200005e8 = 0;
  *(uint32_t*)0x200005ec = 0;
  *(uint32_t*)0x200005f0 = 0;
  *(uint32_t*)0x200005f4 = 0;
  *(uint32_t*)0x200005f8 = 0;
  *(uint32_t*)0x200005fc = 0;
  *(uint32_t*)0x20000600 = 0;
  *(uint32_t*)0x20000604 = 0;
  *(uint32_t*)0x20000608 = 0;
  *(uint32_t*)0x2000060c = 0;
  *(uint32_t*)0x20000610 = 0;
  *(uint32_t*)0x20000614 = 0;
  *(uint32_t*)0x20000618 = 0;
  *(uint32_t*)0x2000061c = 0;
  *(uint32_t*)0x20000620 = 0;
  *(uint32_t*)0x20000624 = 0;
  *(uint32_t*)0x20000628 = 0;
  *(uint32_t*)0x2000062c = 0;
  *(uint32_t*)0x20000630 = 0;
  *(uint32_t*)0x20000634 = 0;
  *(uint32_t*)0x20000638 = 0;
  *(uint32_t*)0x2000063c = 0;
  *(uint32_t*)0x20000640 = 0;
  *(uint32_t*)0x20000644 = 0;
  *(uint32_t*)0x20000648 = 0;
  *(uint32_t*)0x2000064c = 0;
  *(uint32_t*)0x20000650 = 0;
  *(uint32_t*)0x20000654 = 0;
  *(uint32_t*)0x20000658 = 0;
  *(uint32_t*)0x2000065c = 0;
  *(uint32_t*)0x20000660 = 0;
  *(uint32_t*)0x20000664 = 0;
  *(uint32_t*)0x20000668 = 0;
  *(uint32_t*)0x2000066c = 0;
  *(uint32_t*)0x20000670 = 0;
  *(uint32_t*)0x20000674 = 0;
  *(uint32_t*)0x20000678 = 0;
  *(uint32_t*)0x2000067c = 0;
  *(uint32_t*)0x20000680 = 0;
  *(uint32_t*)0x20000684 = 0;
  *(uint32_t*)0x20000688 = 0;
  *(uint32_t*)0x2000068c = 0;
  *(uint32_t*)0x20000690 = 0;
  *(uint32_t*)0x20000694 = 0;
  *(uint32_t*)0x20000698 = 0;
  *(uint32_t*)0x2000069c = 0;
  *(uint32_t*)0x200006a0 = 0;
  *(uint32_t*)0x200006a4 = 0;
  *(uint32_t*)0x200006a8 = 0;
  *(uint32_t*)0x200006ac = 0;
  *(uint32_t*)0x200006b0 = 0;
  *(uint32_t*)0x200006b4 = 0;
  *(uint32_t*)0x200006b8 = 0;
  *(uint32_t*)0x200006bc = 0;
  *(uint32_t*)0x200006c0 = 0;
  *(uint32_t*)0x200006c4 = 0;
  *(uint32_t*)0x200006c8 = 0;
  *(uint32_t*)0x200006cc = 0;
  *(uint32_t*)0x200006d0 = 0;
  *(uint32_t*)0x200006d4 = 0;
  *(uint32_t*)0x200006d8 = 0;
  *(uint32_t*)0x200006dc = 0;
  *(uint32_t*)0x200006e0 = 0;
  *(uint32_t*)0x200006e4 = 0;
  *(uint32_t*)0x200006e8 = 0;
  *(uint32_t*)0x200006ec = 0;
  *(uint32_t*)0x200006f0 = 0;
  *(uint32_t*)0x200006f4 = 0;
  *(uint32_t*)0x200006f8 = 0;
  *(uint32_t*)0x200006fc = 0;
  *(uint32_t*)0x20000700 = 0;
  *(uint32_t*)0x20000704 = 0;
  *(uint32_t*)0x20000708 = 0;
  *(uint32_t*)0x2000070c = 0;
  *(uint32_t*)0x20000710 = 0;
  *(uint32_t*)0x20000714 = 0;
  *(uint32_t*)0x20000718 = 0;
  *(uint32_t*)0x2000071c = 0;
  *(uint32_t*)0x20000720 = 0;
  *(uint32_t*)0x20000724 = 0;
  *(uint32_t*)0x20000728 = 0;
  *(uint32_t*)0x2000072c = 0;
  *(uint32_t*)0x20000730 = 0;
  *(uint32_t*)0x20000734 = 0;
  *(uint32_t*)0x20000738 = 0;
  *(uint32_t*)0x2000073c = 0;
  *(uint32_t*)0x20000740 = 0;
  *(uint32_t*)0x20000744 = 0;
  *(uint32_t*)0x20000748 = 0;
  *(uint32_t*)0x2000074c = 0;
  *(uint32_t*)0x20000750 = 0;
  *(uint32_t*)0x20000754 = 0;
  *(uint32_t*)0x20000758 = 0;
  *(uint32_t*)0x2000075c = 0;
  *(uint32_t*)0x20000760 = 0;
  *(uint32_t*)0x20000764 = 0;
  *(uint32_t*)0x20000768 = 0;
  *(uint32_t*)0x2000076c = 0;
  *(uint32_t*)0x20000770 = 0;
  *(uint32_t*)0x20000774 = 0;
  *(uint32_t*)0x20000778 = 0;
  *(uint32_t*)0x2000077c = 0;
  *(uint32_t*)0x20000780 = 0;
  *(uint32_t*)0x20000784 = 0;
  *(uint32_t*)0x20000788 = 0;
  *(uint32_t*)0x2000078c = 0;
  *(uint32_t*)0x20000790 = 0;
  *(uint32_t*)0x20000794 = 0;
  *(uint32_t*)0x20000798 = 0;
  *(uint32_t*)0x2000079c = 0;
  *(uint32_t*)0x200007a0 = 0;
  *(uint32_t*)0x200007a4 = 0;
  *(uint32_t*)0x200007a8 = 0;
  *(uint32_t*)0x200007ac = 0;
  *(uint32_t*)0x200007b0 = 0;
  *(uint32_t*)0x200007b4 = 0;
  *(uint32_t*)0x200007b8 = 0;
  *(uint32_t*)0x200007bc = 0;
  *(uint32_t*)0x200007c0 = 0;
  *(uint32_t*)0x200007c4 = 0;
  *(uint32_t*)0x200007c8 = 0;
  *(uint32_t*)0x200007cc = 0;
  *(uint32_t*)0x200007d0 = 0;
  *(uint32_t*)0x200007d4 = 0;
  *(uint32_t*)0x200007d8 = 0;
  *(uint32_t*)0x200007dc = 0;
  *(uint32_t*)0x200007e0 = 0;
  *(uint32_t*)0x200007e4 = 0;
  *(uint32_t*)0x200007e8 = 0;
  *(uint32_t*)0x200007ec = 0;
  *(uint32_t*)0x200007f0 = 0;
  *(uint32_t*)0x200007f4 = 0;
  *(uint32_t*)0x200007f8 = 0;
  *(uint32_t*)0x200007fc = 0;
  *(uint32_t*)0x20000800 = 0;
  *(uint32_t*)0x20000804 = 0;
  *(uint32_t*)0x20000808 = 0;
  *(uint32_t*)0x2000080c = 0;
  *(uint32_t*)0x20000810 = 0;
  *(uint32_t*)0x20000814 = 0;
  *(uint32_t*)0x20000818 = 0;
  *(uint32_t*)0x2000081c = 0;
  *(uint32_t*)0x20000820 = 0;
  *(uint32_t*)0x20000824 = 0;
  *(uint32_t*)0x20000828 = 0;
  *(uint32_t*)0x2000082c = 0;
  *(uint32_t*)0x20000830 = 0;
  *(uint32_t*)0x20000834 = 0;
  *(uint32_t*)0x20000838 = 0;
  *(uint32_t*)0x2000083c = 0;
  *(uint32_t*)0x20000840 = 0;
  *(uint32_t*)0x20000844 = 0;
  *(uint32_t*)0x20000848 = 0;
  *(uint32_t*)0x2000084c = 0;
  *(uint32_t*)0x20000850 = 0;
  *(uint32_t*)0x20000854 = 0;
  *(uint32_t*)0x20000858 = 0;
  *(uint32_t*)0x2000085c = 0;
  *(uint32_t*)0x20000860 = 0;
  *(uint32_t*)0x20000864 = 0;
  *(uint32_t*)0x20000868 = 0;
  *(uint32_t*)0x2000086c = 0;
  *(uint32_t*)0x20000870 = 0;
  *(uint32_t*)0x20000874 = 0;
  *(uint32_t*)0x20000878 = 0;
  *(uint32_t*)0x2000087c = 0;
  *(uint32_t*)0x20000880 = 0;
  *(uint32_t*)0x20000884 = 0;
  *(uint32_t*)0x20000888 = 0;
  *(uint32_t*)0x2000088c = 0;
  *(uint32_t*)0x20000890 = 0;
  *(uint32_t*)0x20000894 = 0;
  *(uint32_t*)0x20000898 = 0;
  *(uint32_t*)0x2000089c = 0;
  *(uint32_t*)0x200008a0 = 0;
  *(uint32_t*)0x200008a4 = 0;
  *(uint32_t*)0x200008a8 = 0;
  *(uint32_t*)0x200008ac = 0;
  *(uint32_t*)0x200008b0 = 0;
  *(uint32_t*)0x200008b4 = 0;
  *(uint32_t*)0x200008b8 = 0;
  *(uint32_t*)0x200008bc = 0;
  *(uint32_t*)0x200008c0 = 0;
  *(uint32_t*)0x200008c4 = 0;
  *(uint32_t*)0x200008c8 = 0;
  *(uint32_t*)0x200008cc = 0;
  *(uint32_t*)0x200008d0 = 0;
  *(uint32_t*)0x200008d4 = 0;
  *(uint32_t*)0x200008d8 = 0;
  *(uint32_t*)0x200008dc = 0;
  *(uint32_t*)0x200008e0 = 0;
  *(uint32_t*)0x200008e4 = 0;
  *(uint32_t*)0x200008e8 = 0;
  *(uint32_t*)0x200008ec = 0;
  *(uint32_t*)0x200008f0 = 0;
  *(uint32_t*)0x200008f4 = 0;
  *(uint32_t*)0x200008f8 = 0;
  *(uint32_t*)0x200008fc = 0;
  *(uint32_t*)0x20000900 = 0;
  *(uint32_t*)0x20000904 = 0;
  *(uint32_t*)0x20000908 = 0;
  *(uint32_t*)0x2000090c = 0;
  *(uint32_t*)0x20000910 = 0;
  *(uint32_t*)0x20000914 = 0;
  *(uint32_t*)0x20000918 = 0;
  *(uint32_t*)0x2000091c = 0;
  *(uint32_t*)0x20000920 = 0;
  *(uint32_t*)0x20000924 = 0;
  *(uint32_t*)0x20000928 = 0;
  *(uint32_t*)0x2000092c = 0;
  *(uint32_t*)0x20000930 = 0;
  *(uint32_t*)0x20000934 = 0;
  *(uint32_t*)0x20000938 = 0;
  *(uint32_t*)0x2000093c = 0;
  *(uint32_t*)0x20000940 = 0;
  *(uint32_t*)0x20000944 = 0;
  *(uint32_t*)0x20000948 = 0;
  *(uint32_t*)0x2000094c = 0;
  *(uint32_t*)0x20000950 = 0;
  *(uint32_t*)0x20000954 = 0;
  *(uint32_t*)0x20000958 = 0;
  *(uint32_t*)0x2000095c = 0;
  *(uint32_t*)0x20000960 = 0;
  *(uint32_t*)0x20000964 = 0;
  *(uint32_t*)0x20000968 = 0;
  *(uint32_t*)0x2000096c = 0;
  *(uint32_t*)0x20000970 = 0;
  *(uint32_t*)0x20000974 = 0;
  *(uint32_t*)0x20000978 = 0;
  *(uint32_t*)0x2000097c = 0;
  *(uint32_t*)0x20000980 = 0;
  *(uint32_t*)0x20000984 = 0;
  *(uint32_t*)0x20000988 = 0;
  *(uint32_t*)0x2000098c = 0;
  *(uint32_t*)0x20000990 = 0;
  *(uint32_t*)0x20000994 = 0;
  *(uint32_t*)0x20000998 = 0;
  *(uint32_t*)0x2000099c = 0;
  *(uint32_t*)0x200009a0 = 0;
  *(uint32_t*)0x200009a4 = 0;
  *(uint32_t*)0x200009a8 = 0;
  *(uint32_t*)0x200009ac = 0;
  *(uint32_t*)0x200009b0 = 0;
  *(uint32_t*)0x200009b4 = 0;
  *(uint32_t*)0x200009b8 = 0;
  *(uint32_t*)0x200009bc = 0;
  *(uint32_t*)0x200009c0 = 0;
  *(uint32_t*)0x200009c4 = 0;
  *(uint32_t*)0x200009c8 = 0;
  *(uint32_t*)0x200009cc = 0;
  *(uint32_t*)0x200009d0 = 0;
  *(uint32_t*)0x200009d4 = 0;
  *(uint32_t*)0x200009d8 = 0;
  *(uint32_t*)0x200009dc = 0;
  *(uint32_t*)0x200009e0 = 0;
  *(uint32_t*)0x200009e4 = 0;
  *(uint32_t*)0x200009e8 = 0;
  *(uint32_t*)0x200009ec = 0;
  *(uint32_t*)0x200009f0 = 0;
  *(uint32_t*)0x200009f4 = 0;
  *(uint32_t*)0x200009f8 = 0;
  *(uint32_t*)0x200009fc = 0;
  *(uint32_t*)0x20000a00 = 0;
  *(uint32_t*)0x20000a04 = 0;
  *(uint32_t*)0x20000a08 = 0;
  *(uint32_t*)0x20000a0c = 0;
  *(uint32_t*)0x20000a10 = 0;
  *(uint32_t*)0x20000a14 = 0;
  *(uint32_t*)0x20000a18 = 0;
  *(uint32_t*)0x20000a1c = 0;
  *(uint32_t*)0x20000a20 = 0;
  *(uint32_t*)0x20000a24 = 0;
  *(uint32_t*)0x20000a28 = 0;
  *(uint32_t*)0x20000a2c = 0;
  *(uint32_t*)0x20000a30 = 0;
  *(uint32_t*)0x20000a34 = 0;
  *(uint32_t*)0x20000a38 = 0;
  *(uint32_t*)0x20000a3c = 0;
  *(uint32_t*)0x20000a40 = 0;
  *(uint32_t*)0x20000a44 = 0;
  *(uint32_t*)0x20000a48 = 0;
  *(uint32_t*)0x20000a4c = 0;
  *(uint32_t*)0x20000a50 = 0;
  *(uint32_t*)0x20000a54 = 0;
  *(uint32_t*)0x20000a58 = 0;
  *(uint32_t*)0x20000a5c = 0;
  *(uint32_t*)0x20000a60 = 0;
  *(uint32_t*)0x20000a64 = 0;
  *(uint32_t*)0x20000a68 = 0;
  *(uint32_t*)0x20000a6c = 0;
  *(uint32_t*)0x20000a70 = 0;
  *(uint32_t*)0x20000a74 = 0;
  *(uint32_t*)0x20000a78 = 0;
  *(uint32_t*)0x20000a7c = 0;
  *(uint32_t*)0x20000a80 = 0;
  *(uint32_t*)0x20000a84 = 0;
  *(uint32_t*)0x20000a88 = 0;
  *(uint32_t*)0x20000a8c = 0;
  *(uint32_t*)0x20000a90 = 0;
  *(uint32_t*)0x20000a94 = 0;
  *(uint32_t*)0x20000a98 = 0;
  *(uint32_t*)0x20000a9c = 0;
  *(uint32_t*)0x20000aa0 = 0;
  *(uint32_t*)0x20000aa4 = 0;
  *(uint32_t*)0x20000aa8 = 0;
  *(uint32_t*)0x20000aac = 0;
  *(uint32_t*)0x20000ab0 = 0;
  *(uint32_t*)0x20000ab4 = 0;
  *(uint32_t*)0x20000ab8 = 0;
  *(uint32_t*)0x20000abc = 0;
  *(uint32_t*)0x20000ac0 = 0;
  *(uint32_t*)0x20000ac4 = 0;
  *(uint32_t*)0x20000ac8 = 0;
  *(uint32_t*)0x20000acc = 0;
  *(uint32_t*)0x20000ad0 = 0;
  *(uint32_t*)0x20000ad4 = 0;
  *(uint32_t*)0x20000ad8 = 0;
  *(uint32_t*)0x20000adc = 0;
  *(uint32_t*)0x20000ae0 = 0;
  *(uint32_t*)0x20000ae4 = 0;
  *(uint32_t*)0x20000ae8 = 0;
  *(uint32_t*)0x20000aec = 0;
  *(uint32_t*)0x20000af0 = 0;
  *(uint32_t*)0x20000af4 = 0;
  *(uint32_t*)0x20000af8 = 0;
  *(uint32_t*)0x20000afc = 0;
  *(uint32_t*)0x20000b00 = 0;
  *(uint32_t*)0x20000b04 = 0;
  *(uint32_t*)0x20000b08 = 0;
  *(uint32_t*)0x20000b0c = 0;
  *(uint32_t*)0x20000b10 = 0;
  *(uint32_t*)0x20000b14 = 0;
  *(uint32_t*)0x20000b18 = 0;
  *(uint32_t*)0x20000b1c = 0;
  *(uint32_t*)0x20000b20 = 0;
  *(uint32_t*)0x20000b24 = 0;
  *(uint32_t*)0x20000b28 = 0;
  *(uint32_t*)0x20000b2c = 0;
  *(uint32_t*)0x20000b30 = 0;
  *(uint32_t*)0x20000b34 = 0;
  *(uint32_t*)0x20000b38 = 0;
  *(uint32_t*)0x20000b3c = 0;
  *(uint32_t*)0x20000b40 = 0;
  *(uint32_t*)0x20000b44 = 0;
  *(uint32_t*)0x20000b48 = 0;
  *(uint32_t*)0x20000b4c = 0;
  *(uint32_t*)0x20000b50 = 0;
  *(uint32_t*)0x20000b54 = 0;
  *(uint32_t*)0x20000b58 = 0;
  *(uint32_t*)0x20000b5c = 0;
  *(uint32_t*)0x20000b60 = 0;
  *(uint32_t*)0x20000b64 = 0;
  *(uint32_t*)0x20000b68 = 0;
  *(uint32_t*)0x20000b6c = 0;
  *(uint32_t*)0x20000b70 = 0;
  *(uint32_t*)0x20000b74 = 0;
  *(uint32_t*)0x20000b78 = 0;
  *(uint32_t*)0x20000b7c = 0;
  *(uint32_t*)0x20000b80 = 0;
  *(uint32_t*)0x20000b84 = 0;
  *(uint32_t*)0x20000b88 = 0;
  *(uint32_t*)0x20000b8c = 0;
  *(uint32_t*)0x20000b90 = 0;
  *(uint32_t*)0x20000b94 = 0;
  *(uint32_t*)0x20000b98 = 0;
  *(uint32_t*)0x20000b9c = 0;
  *(uint32_t*)0x20000ba0 = 0;
  *(uint32_t*)0x20000ba4 = 0;
  *(uint32_t*)0x20000ba8 = 0;
  *(uint32_t*)0x20000bac = 0;
  *(uint32_t*)0x20000bb0 = 0;
  *(uint32_t*)0x20000bb4 = 0;
  *(uint32_t*)0x20000bb8 = 0;
  *(uint32_t*)0x20000bbc = 0;
  *(uint32_t*)0x20000bc0 = 0;
  *(uint32_t*)0x20000bc4 = 0;
  *(uint32_t*)0x20000bc8 = 0;
  *(uint32_t*)0x20000bcc = 0;
  *(uint32_t*)0x20000bd0 = 0;
  *(uint32_t*)0x20000bd4 = 0;
  *(uint32_t*)0x20000bd8 = 0;
  *(uint32_t*)0x20000bdc = 0;
  *(uint32_t*)0x20000be0 = 0;
  *(uint32_t*)0x20000be4 = 0;
  *(uint32_t*)0x20000be8 = 0;
  *(uint32_t*)0x20000bec = 0;
  *(uint32_t*)0x20000bf0 = 0;
  *(uint32_t*)0x20000bf4 = 0;
  *(uint32_t*)0x20000bf8 = 0;
  *(uint32_t*)0x20000bfc = 0;
  *(uint32_t*)0x20000c00 = 0;
  *(uint32_t*)0x20000c04 = 0;
  *(uint32_t*)0x20000c08 = 0;
  *(uint32_t*)0x20000c0c = 0;
  *(uint32_t*)0x20000c10 = 0;
  *(uint32_t*)0x20000c14 = 0;
  *(uint32_t*)0x20000c18 = 0;
  *(uint32_t*)0x20000c1c = 0;
  *(uint32_t*)0x20000c20 = 0;
  *(uint32_t*)0x20000c24 = 0;
  *(uint32_t*)0x20000c28 = 0;
  *(uint32_t*)0x20000c2c = 0;
  *(uint32_t*)0x20000c30 = 0;
  *(uint32_t*)0x20000c34 = 0;
  *(uint32_t*)0x20000c38 = 0;
  *(uint32_t*)0x20000c3c = 0;
  *(uint32_t*)0x20000c40 = 0;
  *(uint32_t*)0x20000c44 = 0;
  *(uint32_t*)0x20000c48 = 0;
  *(uint32_t*)0x20000c4c = 0;
  *(uint32_t*)0x20000c50 = 0;
  *(uint32_t*)0x20000c54 = 0;
  *(uint32_t*)0x20000c58 = 0;
  *(uint32_t*)0x20000c5c = 0;
  *(uint32_t*)0x20000c60 = 0;
  *(uint32_t*)0x20000c64 = 0;
  *(uint32_t*)0x20000c68 = 0;
  *(uint32_t*)0x20000c6c = 0;
  *(uint32_t*)0x20000c70 = 0;
  *(uint32_t*)0x20000c74 = 0;
  *(uint32_t*)0x20000c78 = 0;
  *(uint32_t*)0x20000c7c = 0;
  *(uint32_t*)0x20000c80 = 0;
  *(uint32_t*)0x20000c84 = 0;
  *(uint32_t*)0x20000c88 = 0;
  *(uint32_t*)0x20000c8c = 0;
  *(uint32_t*)0x20000c90 = 0;
  *(uint32_t*)0x20000c94 = 0;
  *(uint32_t*)0x20000c98 = 0;
  *(uint32_t*)0x20000c9c = 0;
  *(uint32_t*)0x20000ca0 = 0;
  *(uint32_t*)0x20000ca4 = 0;
  *(uint32_t*)0x20000ca8 = 0;
  *(uint32_t*)0x20000cac = 0;
  *(uint32_t*)0x20000cb0 = 0;
  *(uint32_t*)0x20000cb4 = 0;
  *(uint32_t*)0x20000cb8 = 0;
  *(uint32_t*)0x20000cbc = 0;
  *(uint32_t*)0x20000cc0 = 0;
  *(uint32_t*)0x20000cc4 = 0;
  *(uint32_t*)0x20000cc8 = 0;
  *(uint32_t*)0x20000ccc = 0;
  *(uint32_t*)0x20000cd0 = 0;
  *(uint32_t*)0x20000cd4 = 0;
  *(uint32_t*)0x20000cd8 = 0;
  *(uint32_t*)0x20000cdc = 0;
  *(uint32_t*)0x20000ce0 = 0;
  *(uint32_t*)0x20000ce4 = 0;
  *(uint32_t*)0x20000ce8 = 0;
  *(uint32_t*)0x20000cec = 0;
  *(uint32_t*)0x20000cf0 = 0;
  *(uint32_t*)0x20000cf4 = 0;
  *(uint32_t*)0x20000cf8 = 0;
  *(uint32_t*)0x20000cfc = 0;
  *(uint32_t*)0x20000d00 = 0;
  *(uint32_t*)0x20000d04 = 0;
  *(uint32_t*)0x20000d08 = 0;
  *(uint32_t*)0x20000d0c = 0;
  *(uint32_t*)0x20000d10 = 0;
  *(uint32_t*)0x20000d14 = 0;
  *(uint32_t*)0x20000d18 = 0;
  *(uint32_t*)0x20000d1c = 0;
  *(uint32_t*)0x20000d20 = 0;
  *(uint32_t*)0x20000d24 = 0;
  *(uint32_t*)0x20000d28 = 0;
  *(uint32_t*)0x20000d2c = 0;
  *(uint32_t*)0x20000d30 = 0;
  *(uint32_t*)0x20000d34 = 0;
  *(uint32_t*)0x20000d38 = 0;
  *(uint32_t*)0x20000d3c = 0;
  *(uint32_t*)0x20000d40 = 0;
  *(uint32_t*)0x20000d44 = 0;
  *(uint32_t*)0x20000d48 = 0;
  *(uint32_t*)0x20000d4c = 0;
  *(uint32_t*)0x20000d50 = 0;
  *(uint32_t*)0x20000d54 = 0;
  *(uint32_t*)0x20000d58 = 0;
  *(uint32_t*)0x20000d5c = 0;
  *(uint32_t*)0x20000d60 = 0;
  *(uint32_t*)0x20000d64 = 0;
  *(uint32_t*)0x20000d68 = 0;
  *(uint32_t*)0x20000d6c = 0;
  *(uint32_t*)0x20000d70 = 0;
  *(uint32_t*)0x20000d74 = 0;
  *(uint32_t*)0x20000d78 = 0;
  *(uint32_t*)0x20000d7c = 0;
  *(uint32_t*)0x20000d80 = 0;
  *(uint32_t*)0x20000d84 = 0;
  *(uint32_t*)0x20000d88 = 0;
  *(uint32_t*)0x20000d8c = 0;
  *(uint32_t*)0x20000d90 = 0;
  *(uint32_t*)0x20000d94 = 0;
  *(uint32_t*)0x20000d98 = 0;
  *(uint32_t*)0x20000d9c = 0;
  *(uint32_t*)0x20000da0 = 0;
  *(uint32_t*)0x20000da4 = 0;
  *(uint32_t*)0x20000da8 = 0;
  *(uint32_t*)0x20000dac = 0;
  *(uint32_t*)0x20000db0 = 0;
  *(uint32_t*)0x20000db4 = 0;
  *(uint32_t*)0x20000db8 = 0;
  *(uint32_t*)0x20000dbc = 0;
  *(uint32_t*)0x20000dc0 = 0;
  *(uint32_t*)0x20000dc4 = 0;
  *(uint32_t*)0x20000dc8 = 0;
  *(uint32_t*)0x20000dcc = 0;
  *(uint32_t*)0x20000dd0 = 0;
  *(uint32_t*)0x20000dd4 = 0;
  *(uint32_t*)0x20000dd8 = 0;
  *(uint32_t*)0x20000ddc = 0;
  *(uint32_t*)0x20000de0 = 0;
  *(uint32_t*)0x20000de4 = 0;
  *(uint32_t*)0x20000de8 = 0;
  *(uint32_t*)0x20000dec = 0;
  *(uint32_t*)0x20000df0 = 0;
  *(uint32_t*)0x20000df4 = 0;
  *(uint32_t*)0x20000df8 = 0;
  *(uint32_t*)0x20000dfc = 0;
  *(uint32_t*)0x20000e00 = 0;
  *(uint32_t*)0x20000e04 = 0;
  *(uint32_t*)0x20000e08 = 0;
  *(uint32_t*)0x20000e0c = 0;
  *(uint32_t*)0x20000e10 = 0;
  *(uint32_t*)0x20000e14 = 0;
  *(uint32_t*)0x20000e18 = 0;
  *(uint32_t*)0x20000e1c = 0;
  *(uint32_t*)0x20000e20 = 0;
  *(uint32_t*)0x20000e24 = 0;
  *(uint32_t*)0x20000e28 = 0;
  *(uint32_t*)0x20000e2c = 0;
  *(uint32_t*)0x20000e30 = 0;
  *(uint32_t*)0x20000e34 = 0;
  *(uint32_t*)0x20000e38 = 0;
  *(uint32_t*)0x20000e3c = 0;
  *(uint32_t*)0x20000e40 = 0;
  *(uint32_t*)0x20000e44 = 0;
  *(uint32_t*)0x20000e48 = 0;
  *(uint32_t*)0x20000e4c = 0;
  *(uint32_t*)0x20000e50 = 0;
  *(uint32_t*)0x20000e54 = 0;
  *(uint32_t*)0x20000e58 = 0;
  *(uint32_t*)0x20000e5c = 0;
  *(uint32_t*)0x20000e60 = 0;
  *(uint32_t*)0x20000e64 = 0;
  *(uint32_t*)0x20000e68 = 0;
  *(uint32_t*)0x20000e6c = 0;
  *(uint32_t*)0x20000e70 = 0;
  *(uint32_t*)0x20000e74 = 0;
  *(uint32_t*)0x20000e78 = 0;
  *(uint32_t*)0x20000e7c = 0;
  *(uint32_t*)0x20000e80 = 0;
  *(uint32_t*)0x20000e84 = 0;
  *(uint32_t*)0x20000e88 = 0;
  *(uint32_t*)0x20000e8c = 0;
  *(uint32_t*)0x20000e90 = 0;
  *(uint32_t*)0x20000e94 = 0;
  *(uint32_t*)0x20000e98 = 0;
  *(uint32_t*)0x20000e9c = 0;
  *(uint32_t*)0x20000ea0 = 0;
  *(uint32_t*)0x20000ea4 = 0;
  *(uint32_t*)0x20000ea8 = 0;
  *(uint32_t*)0x20000eac = 0;
  *(uint32_t*)0x20000eb0 = 0;
  *(uint32_t*)0x20000eb4 = 0;
  *(uint32_t*)0x20000eb8 = 0;
  *(uint32_t*)0x20000ebc = 0;
  *(uint32_t*)0x20000ec0 = 0;
  *(uint32_t*)0x20000ec4 = 0;
  *(uint32_t*)0x20000ec8 = 0;
  *(uint32_t*)0x20000ecc = 0;
  *(uint32_t*)0x20000ed0 = 0;
  *(uint32_t*)0x20000ed4 = 0;
  *(uint32_t*)0x20000ed8 = 0;
  *(uint32_t*)0x20000edc = 0;
  *(uint32_t*)0x20000ee0 = 0;
  *(uint32_t*)0x20000ee4 = 0;
  *(uint32_t*)0x20000ee8 = 0;
  *(uint32_t*)0x20000eec = 0;
  *(uint32_t*)0x20000ef0 = 0;
  *(uint32_t*)0x20000ef4 = 0;
  *(uint32_t*)0x20000ef8 = 0;
  *(uint32_t*)0x20000efc = 0;
  *(uint32_t*)0x20000f00 = 0;
  *(uint32_t*)0x20000f04 = 0;
  *(uint32_t*)0x20000f08 = 0;
  *(uint32_t*)0x20000f0c = 0;
  *(uint32_t*)0x20000f10 = 0;
  *(uint32_t*)0x20000f14 = 0;
  *(uint32_t*)0x20000f18 = 0;
  *(uint32_t*)0x20000f1c = 0;
  *(uint32_t*)0x20000f20 = 0;
  *(uint32_t*)0x20000f24 = 0;
  *(uint32_t*)0x20000f28 = 0;
  *(uint32_t*)0x20000f2c = 0;
  *(uint32_t*)0x20000f30 = 0;
  *(uint32_t*)0x20000f34 = 0;
  *(uint32_t*)0x20000f38 = 0;
  *(uint32_t*)0x20000f3c = 0;
  *(uint32_t*)0x20000f40 = 0;
  *(uint32_t*)0x20000f44 = 0;
  *(uint32_t*)0x20000f48 = 0;
  *(uint32_t*)0x20000f4c = 0;
  *(uint32_t*)0x20000f50 = 0;
  *(uint32_t*)0x20000f54 = 0;
  *(uint32_t*)0x20000f58 = 0;
  *(uint32_t*)0x20000f5c = 0;
  *(uint32_t*)0x20000f60 = 0;
  *(uint32_t*)0x20000f64 = 0;
  *(uint32_t*)0x20000f68 = 0;
  *(uint32_t*)0x20000f6c = 0;
  *(uint32_t*)0x20000f70 = 0;
  *(uint32_t*)0x20000f74 = 0;
  *(uint32_t*)0x20000f78 = 0;
  *(uint32_t*)0x20000f7c = 0;
  *(uint32_t*)0x20000f80 = 0;
  *(uint32_t*)0x20000f84 = 0;
  *(uint32_t*)0x20000f88 = 0;
  *(uint32_t*)0x20000f8c = 0;
  *(uint32_t*)0x20000f90 = 0;
  *(uint32_t*)0x20000f94 = 0;
  *(uint32_t*)0x20000f98 = 0;
  *(uint32_t*)0x20000f9c = 0;
  *(uint32_t*)0x20000fa0 = 0;
  *(uint32_t*)0x20000fa4 = 0;
  *(uint32_t*)0x20000fa8 = 0;
  *(uint32_t*)0x20000fac = 0;
  *(uint32_t*)0x20000fb0 = 0;
  *(uint32_t*)0x20000fb4 = 0;
  *(uint32_t*)0x20000fb8 = 0;
  *(uint32_t*)0x20000fbc = 0;
  *(uint32_t*)0x20000fc0 = 0;
  *(uint32_t*)0x20000fc4 = 0;
  *(uint32_t*)0x20000fc8 = 0;
  *(uint32_t*)0x20000fcc = 0;
  *(uint32_t*)0x20000fd0 = 0;
  *(uint32_t*)0x20000fd4 = 0;
  *(uint32_t*)0x20000fd8 = 0;
  *(uint32_t*)0x20000fdc = 0;
  *(uint32_t*)0x20000fe0 = 0;
  *(uint32_t*)0x20000fe4 = 0;
  *(uint32_t*)0x20000fe8 = 0;
  *(uint32_t*)0x20000fec = 0;
  *(uint32_t*)0x20000ff0 = 0;
  *(uint32_t*)0x20000ff4 = 0;
  *(uint32_t*)0x20000ff8 = 0;
  *(uint32_t*)0x20000ffc = 0;
  *(uint32_t*)0x20001000 = 0;
  *(uint32_t*)0x20001004 = 0;
  *(uint32_t*)0x20001008 = 0;
  *(uint32_t*)0x2000100c = 0;
  *(uint32_t*)0x20001010 = 0;
  *(uint32_t*)0x20001014 = 0;
  *(uint32_t*)0x20001018 = 0;
  *(uint32_t*)0x2000101c = 0;
  *(uint32_t*)0x20001020 = 0;
  *(uint32_t*)0x20001024 = 0;
  *(uint32_t*)0x20001028 = 0;
  *(uint32_t*)0x2000102c = 0;
  *(uint32_t*)0x20001030 = 0;
  *(uint32_t*)0x20001034 = 0;
  *(uint32_t*)0x20001038 = 0;
  *(uint32_t*)0x2000103c = 0;
  *(uint32_t*)0x20001040 = 0;
  *(uint32_t*)0x20001044 = 0;
  *(uint32_t*)0x20001048 = 0;
  *(uint32_t*)0x2000104c = 0;
  *(uint32_t*)0x20001050 = 0;
  *(uint32_t*)0x20001054 = 0;
  *(uint32_t*)0x20001058 = 0;
  *(uint32_t*)0x2000105c = 0;
  *(uint32_t*)0x20001060 = 0;
  *(uint32_t*)0x20001064 = 0;
  *(uint32_t*)0x20001068 = 0;
  *(uint32_t*)0x2000106c = 0;
  *(uint32_t*)0x20001070 = 0;
  *(uint32_t*)0x20001074 = 0;
  *(uint32_t*)0x20001078 = 0;
  *(uint32_t*)0x2000107c = 0;
  *(uint32_t*)0x20001080 = 0;
  *(uint32_t*)0x20001084 = 0;
  *(uint32_t*)0x20001088 = 0;
  *(uint32_t*)0x2000108c = 0;
  *(uint32_t*)0x20001090 = 0;
  *(uint32_t*)0x20001094 = 0;
  *(uint32_t*)0x20001098 = 0;
  *(uint32_t*)0x2000109c = 0;
  *(uint32_t*)0x200010a0 = 0;
  *(uint32_t*)0x200010a4 = 0;
  *(uint32_t*)0x200010a8 = 0;
  *(uint32_t*)0x200010ac = 0;
  *(uint32_t*)0x200010b0 = 0;
  *(uint32_t*)0x200010b4 = 0;
  *(uint32_t*)0x200010b8 = 0;
  *(uint32_t*)0x200010bc = 0;
  *(uint32_t*)0x200010c0 = 0;
  *(uint32_t*)0x200010c4 = 0;
  *(uint32_t*)0x200010c8 = 0;
  *(uint32_t*)0x200010cc = 0;
  *(uint32_t*)0x200010d0 = 0;
  *(uint32_t*)0x200010d4 = 0;
  *(uint32_t*)0x200010d8 = 0;
  *(uint32_t*)0x200010dc = 0;
  *(uint32_t*)0x200010e0 = 0;
  *(uint32_t*)0x200010e4 = 0;
  *(uint32_t*)0x200010e8 = 0;
  *(uint32_t*)0x200010ec = 0;
  *(uint32_t*)0x200010f0 = 0;
  *(uint32_t*)0x200010f4 = 0;
  *(uint32_t*)0x200010f8 = 0;
  *(uint32_t*)0x200010fc = 0;
  *(uint32_t*)0x20001100 = 0;
  *(uint32_t*)0x20001104 = 0;
  *(uint32_t*)0x20001108 = 0;
  *(uint32_t*)0x2000110c = 0;
  *(uint32_t*)0x20001110 = 0;
  *(uint32_t*)0x20001114 = 0;
  *(uint32_t*)0x20001118 = 0;
  *(uint32_t*)0x2000111c = 0;
  *(uint32_t*)0x20001120 = 0;
  *(uint32_t*)0x20001124 = 0;
  *(uint32_t*)0x20001128 = 0;
  *(uint32_t*)0x2000112c = 0;
  *(uint32_t*)0x20001130 = 0;
  *(uint32_t*)0x20001134 = 0;
  *(uint32_t*)0x20001138 = 0;
  *(uint32_t*)0x2000113c = 0;
  *(uint32_t*)0x20001140 = 0;
  *(uint32_t*)0x20001144 = 0;
  *(uint32_t*)0x20001148 = 0;
  *(uint32_t*)0x2000114c = 0;
  *(uint32_t*)0x20001150 = 0;
  *(uint32_t*)0x20001154 = 0;
  *(uint32_t*)0x20001158 = 0;
  *(uint32_t*)0x2000115c = 0;
  *(uint32_t*)0x20001160 = 0;
  *(uint32_t*)0x20001164 = 0;
  *(uint32_t*)0x20001168 = 0;
  *(uint32_t*)0x2000116c = 0;
  *(uint32_t*)0x20001170 = 0;
  *(uint32_t*)0x20001174 = 0;
  *(uint32_t*)0x20001178 = 0;
  *(uint32_t*)0x2000117c = 0;
  *(uint32_t*)0x20001180 = 0;
  *(uint32_t*)0x20001184 = 0;
  *(uint32_t*)0x20001188 = 0;
  *(uint32_t*)0x2000118c = 0;
  *(uint32_t*)0x20001190 = 0;
  *(uint32_t*)0x20001194 = 0;
  *(uint32_t*)0x20001198 = 0;
  *(uint32_t*)0x2000119c = 0;
  *(uint32_t*)0x200011a0 = 0;
  *(uint32_t*)0x200011a4 = 0;
  *(uint32_t*)0x200011a8 = 0;
  *(uint32_t*)0x200011ac = 0;
  *(uint32_t*)0x200011b0 = 0;
  *(uint32_t*)0x200011b4 = 0;
  *(uint32_t*)0x200011b8 = 0;
  *(uint32_t*)0x200011bc = 0;
  *(uint32_t*)0x200011c0 = 0;
  *(uint32_t*)0x200011c4 = 0;
  *(uint32_t*)0x200011c8 = 0;
  *(uint32_t*)0x200011cc = 0;
  *(uint32_t*)0x200011d0 = 0;
  *(uint32_t*)0x200011d4 = 0;
  *(uint32_t*)0x200011d8 = 0;
  *(uint32_t*)0x200011dc = 0;
  *(uint32_t*)0x200011e0 = 0;
  *(uint32_t*)0x200011e4 = 0;
  *(uint32_t*)0x200011e8 = 0;
  *(uint32_t*)0x200011ec = 0;
  *(uint32_t*)0x200011f0 = 0;
  *(uint32_t*)0x200011f4 = 0;
  *(uint32_t*)0x200011f8 = 0;
  *(uint32_t*)0x200011fc = 0;
  *(uint32_t*)0x20001200 = 0;
  *(uint32_t*)0x20001204 = 0;
  *(uint32_t*)0x20001208 = 0;
  *(uint32_t*)0x2000120c = 0;
  *(uint32_t*)0x20001210 = 0;
  *(uint32_t*)0x20001214 = 0;
  *(uint32_t*)0x20001218 = 0;
  *(uint32_t*)0x2000121c = 0;
  *(uint32_t*)0x20001220 = 0;
  *(uint32_t*)0x20001224 = 0;
  *(uint32_t*)0x20001228 = 0;
  *(uint32_t*)0x2000122c = 0;
  *(uint32_t*)0x20001230 = 0;
  *(uint32_t*)0x20001234 = 0;
  *(uint32_t*)0x20001238 = 0;
  *(uint32_t*)0x2000123c = 0;
  *(uint32_t*)0x20001240 = 0;
  *(uint32_t*)0x20001244 = 0;
  *(uint32_t*)0x20001248 = 0;
  *(uint32_t*)0x2000124c = 0;
  *(uint32_t*)0x20001250 = 0;
  *(uint32_t*)0x20001254 = 0;
  *(uint32_t*)0x20001258 = 0;
  *(uint32_t*)0x2000125c = 0;
  *(uint32_t*)0x20001260 = 0;
  *(uint32_t*)0x20001264 = 0;
  *(uint32_t*)0x20001268 = 0;
  *(uint32_t*)0x2000126c = 0;
  *(uint32_t*)0x20001270 = 0;
  *(uint32_t*)0x20001274 = 0;
  *(uint32_t*)0x20001278 = 0;
  *(uint32_t*)0x2000127c = 0;
  *(uint32_t*)0x20001280 = 0;
  *(uint32_t*)0x20001284 = 0;
  *(uint32_t*)0x20001288 = 0;
  *(uint32_t*)0x2000128c = 0;
  *(uint32_t*)0x20001290 = 0;
  *(uint32_t*)0x20001294 = 0;
  *(uint32_t*)0x20001298 = 0;
  *(uint32_t*)0x2000129c = 0;
  *(uint32_t*)0x200012a0 = 0;
  *(uint32_t*)0x200012a4 = 0;
  *(uint32_t*)0x200012a8 = 0;
  *(uint32_t*)0x200012ac = 0;
  *(uint32_t*)0x200012b0 = 0;
  *(uint32_t*)0x200012b4 = 0;
  *(uint32_t*)0x200012b8 = 0;
  *(uint32_t*)0x200012bc = 0;
  *(uint32_t*)0x200012c0 = 0;
  *(uint32_t*)0x200012c4 = 0;
  *(uint32_t*)0x200012c8 = 0;
  *(uint32_t*)0x200012cc = 0;
  *(uint32_t*)0x200012d0 = 0;
  *(uint32_t*)0x200012d4 = 0;
  *(uint32_t*)0x200012d8 = 0;
  *(uint32_t*)0x200012dc = 0;
  *(uint32_t*)0x200012e0 = 0;
  *(uint32_t*)0x200012e4 = 0;
  *(uint32_t*)0x200012e8 = 0;
  *(uint32_t*)0x200012ec = 0;
  *(uint32_t*)0x200012f0 = 0;
  *(uint32_t*)0x200012f4 = 0;
  *(uint32_t*)0x200012f8 = 0;
  *(uint32_t*)0x200012fc = 0;
  *(uint32_t*)0x20001300 = 0;
  *(uint32_t*)0x20001304 = 0;
  *(uint32_t*)0x20001308 = 0;
  *(uint32_t*)0x2000130c = 0;
  *(uint32_t*)0x20001310 = 0;
  *(uint32_t*)0x20001314 = 0;
  *(uint32_t*)0x20001318 = 0;
  *(uint32_t*)0x2000131c = 0;
  *(uint32_t*)0x20001320 = 0;
  *(uint32_t*)0x20001324 = 0;
  *(uint32_t*)0x20001328 = 0;
  *(uint32_t*)0x2000132c = 0;
  *(uint32_t*)0x20001330 = 0;
  *(uint32_t*)0x20001334 = 0;
  *(uint32_t*)0x20001338 = 0;
  *(uint32_t*)0x2000133c = 0;
  *(uint32_t*)0x20001340 = 0;
  *(uint32_t*)0x20001344 = 0;
  *(uint32_t*)0x20001348 = 0;
  *(uint32_t*)0x2000134c = 0;
  *(uint32_t*)0x20001350 = 0;
  *(uint32_t*)0x20001354 = 0;
  *(uint32_t*)0x20001358 = 0;
  *(uint32_t*)0x2000135c = 0;
  *(uint32_t*)0x20001360 = 0;
  *(uint32_t*)0x20001364 = 0;
  *(uint32_t*)0x20001368 = 0;
  *(uint32_t*)0x2000136c = 0;
  *(uint32_t*)0x20001370 = 0;
  *(uint32_t*)0x20001374 = 0;
  *(uint32_t*)0x20001378 = 0;
  *(uint32_t*)0x2000137c = 0;
  *(uint32_t*)0x20001380 = 0;
  *(uint32_t*)0x20001384 = 0;
  *(uint32_t*)0x20001388 = 0;
  *(uint32_t*)0x2000138c = 0;
  *(uint32_t*)0x20001390 = 0;
  *(uint32_t*)0x20001394 = 0;
  *(uint32_t*)0x20001398 = 0;
  *(uint32_t*)0x2000139c = 0;
  *(uint32_t*)0x200013a0 = 0;
  *(uint32_t*)0x200013a4 = 0;
  *(uint32_t*)0x200013a8 = 0;
  *(uint32_t*)0x200013ac = 0;
  *(uint32_t*)0x200013b0 = 0;
  *(uint32_t*)0x200013b4 = 0;
  *(uint32_t*)0x200013b8 = 0;
  *(uint32_t*)0x200013bc = 0;
  *(uint32_t*)0x200013c0 = 0;
  *(uint32_t*)0x200013c4 = 0;
  *(uint32_t*)0x200013c8 = 0;
  *(uint32_t*)0x200013cc = 0;
  *(uint32_t*)0x200013d0 = 0;
  *(uint32_t*)0x200013d4 = 0;
  *(uint32_t*)0x200013d8 = 0;
  *(uint32_t*)0x200013dc = 0;
  *(uint32_t*)0x200013e0 = 0;
  *(uint32_t*)0x200013e4 = 0;
  *(uint32_t*)0x200013e8 = 0;
  *(uint32_t*)0x200013ec = 0;
  *(uint32_t*)0x200013f0 = 0;
  *(uint32_t*)0x200013f4 = 0;
  *(uint32_t*)0x200013f8 = 0;
  *(uint32_t*)0x200013fc = 0;
  *(uint32_t*)0x20001400 = 0;
  *(uint32_t*)0x20001404 = 0;
  *(uint32_t*)0x20001408 = 0;
  *(uint32_t*)0x2000140c = 0;
  *(uint32_t*)0x20001410 = 0;
  *(uint32_t*)0x20001414 = 0;
  *(uint32_t*)0x20001418 = 0;
  *(uint32_t*)0x2000141c = 0;
  *(uint32_t*)0x20001420 = 0;
  *(uint32_t*)0x20001424 = 0;
  *(uint32_t*)0x20001428 = 0;
  *(uint32_t*)0x2000142c = 0;
  *(uint32_t*)0x20001430 = 0;
  *(uint32_t*)0x20001434 = 0;
  *(uint32_t*)0x20001438 = 0;
  *(uint32_t*)0x2000143c = 0;
  *(uint32_t*)0x20001440 = 0;
  *(uint32_t*)0x20001444 = 0;
  *(uint32_t*)0x20001448 = 0;
  *(uint32_t*)0x2000144c = 0;
  *(uint32_t*)0x20001450 = 0;
  *(uint32_t*)0x20001454 = 0;
  *(uint32_t*)0x20001458 = 0;
  *(uint32_t*)0x2000145c = 0;
  *(uint32_t*)0x20001460 = 0;
  *(uint32_t*)0x20001464 = 0;
  *(uint32_t*)0x20001468 = 0;
  *(uint32_t*)0x2000146c = 0;
  *(uint32_t*)0x20001470 = 0;
  *(uint32_t*)0x20001474 = 0;
  *(uint32_t*)0x20001478 = 0;
  *(uint32_t*)0x2000147c = 0;
  *(uint32_t*)0x20001480 = 0;
  *(uint32_t*)0x20001484 = 0;
  *(uint32_t*)0x20001488 = 0;
  *(uint32_t*)0x2000148c = 0;
  *(uint32_t*)0x20001490 = 0;
  *(uint32_t*)0x20001494 = 0;
  *(uint32_t*)0x20001498 = 0;
  *(uint32_t*)0x2000149c = 0;
  *(uint32_t*)0x200014a0 = 0;
  *(uint32_t*)0x200014a4 = 0;
  *(uint32_t*)0x200014a8 = 0;
  *(uint32_t*)0x200014ac = 0;
  *(uint32_t*)0x200014b0 = 0;
  *(uint32_t*)0x200014b4 = 0;
  *(uint32_t*)0x200014b8 = 0;
  *(uint32_t*)0x200014bc = 0;
  *(uint32_t*)0x200014c0 = 0;
  *(uint32_t*)0x200014c4 = 0;
  *(uint32_t*)0x200014c8 = 0;
  *(uint32_t*)0x200014cc = 0;
  *(uint32_t*)0x200014d0 = 0;
  *(uint32_t*)0x200014d4 = 0;
  *(uint32_t*)0x200014d8 = 0;
  *(uint32_t*)0x200014dc = 0;
  *(uint32_t*)0x200014e0 = 0;
  *(uint32_t*)0x200014e4 = 0;
  *(uint32_t*)0x200014e8 = 0;
  *(uint32_t*)0x200014ec = 0;
  *(uint32_t*)0x200014f0 = 0;
  *(uint32_t*)0x200014f4 = 0;
  *(uint32_t*)0x200014f8 = 0;
  *(uint32_t*)0x200014fc = 0;
  *(uint32_t*)0x20001500 = 0;
  *(uint32_t*)0x20001504 = 0;
  *(uint32_t*)0x20001508 = 0;
  *(uint32_t*)0x2000150c = 0;
  *(uint32_t*)0x20001510 = 0;
  *(uint32_t*)0x20001514 = 0;
  *(uint32_t*)0x20001518 = 0;
  *(uint32_t*)0x2000151c = 0;
  *(uint32_t*)0x20001520 = 0;
  *(uint32_t*)0x20001524 = 0;
  *(uint32_t*)0x20001528 = 0;
  *(uint32_t*)0x2000152c = 0;
  *(uint32_t*)0x20001530 = 0;
  *(uint32_t*)0x20001534 = 0;
  *(uint32_t*)0x20001538 = 0;
  *(uint32_t*)0x2000153c = 0;
  *(uint32_t*)0x20001548 = 6;
  *(uint32_t*)0x2000154c = 0x400;
  *(uint32_t*)0x20001550 = 0;
  *(uint32_t*)0x20001554 = 0;
  syscall(__NR_ioctl, r[0], 0x7b1, 0x20001540ul);
}
int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  loop();
  return 0;
}
