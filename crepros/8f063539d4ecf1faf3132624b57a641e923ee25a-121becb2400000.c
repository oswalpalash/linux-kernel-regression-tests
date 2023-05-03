// https://syzkaller.appspot.com/bug?id=8f063539d4ecf1faf3132624b57a641e923ee25a
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

static long syz_open_dev(long a0, long a1, long a2)
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

static long syz_open_procfs(long a0, long a1)
{
  char buf[128];
  memset(buf, 0, sizeof(buf));
  if (a0 == 0) {
    snprintf(buf, sizeof(buf), "/proc/self/%s", (char*)a1);
  } else if (a0 == -1) {
    snprintf(buf, sizeof(buf), "/proc/thread-self/%s", (char*)a1);
  } else {
    snprintf(buf, sizeof(buf), "/proc/self/task/%d/%s", (int)a0, (char*)a1);
  }
  int fd = open(buf, O_RDWR);
  if (fd == -1)
    fd = open(buf, O_RDONLY);
  return fd;
}

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  syscall(__NR_ioctl, -1, 0x8912, 0x20000080);
  memcpy((void*)0x20000040, "syscall", 8);
  syz_open_procfs(-1, 0x20000040);
  syscall(__NR_socket, 0x10, 2, 0);
  syscall(__NR_socketpair, 0, 0, 0, 0x20000140);
  *(uint32_t*)0x20000180 = -1;
  *(uint64_t*)0x20000188 = 0x20000000;
  *(uint64_t*)0x20000190 = 0x20000140;
  *(uint64_t*)0x20000198 = 0;
  syscall(__NR_bpf, 2, 0x20000180, 0x20);
  memcpy((void*)0x20000000, "/dev/kvm", 9);
  res = syscall(__NR_openat, 0xffffffffffffff9c, 0x20000000, 0, 0);
  if (res != -1)
    r[0] = res;
  res = syscall(__NR_ioctl, r[0], 0xae01, 0);
  if (res != -1)
    r[1] = res;
  res = syscall(__NR_ioctl, r[1], 0xae41, 0);
  if (res != -1)
    r[2] = res;
  *(uint32_t*)0x200001c0 = 1;
  *(uint32_t*)0x200001c4 = 0;
  *(uint64_t*)0x200001c8 = 0x40000021;
  *(uint64_t*)0x200001d0 = 3;
  *(uint64_t*)0x200001d8 = 0;
  *(uint64_t*)0x200001e0 = 0;
  *(uint8_t*)0x200001e8 = 0;
  *(uint8_t*)0x200001e9 = 0;
  *(uint8_t*)0x200001ea = 0;
  *(uint8_t*)0x200001eb = 0;
  *(uint8_t*)0x200001ec = 0;
  *(uint8_t*)0x200001ed = 0;
  *(uint8_t*)0x200001ee = 0;
  *(uint8_t*)0x200001ef = 0;
  *(uint8_t*)0x200001f0 = 0;
  *(uint8_t*)0x200001f1 = 0;
  *(uint8_t*)0x200001f2 = 0;
  *(uint8_t*)0x200001f3 = 0;
  *(uint8_t*)0x200001f4 = 0;
  *(uint8_t*)0x200001f5 = 0;
  *(uint8_t*)0x200001f6 = 0;
  *(uint8_t*)0x200001f7 = 0;
  *(uint8_t*)0x200001f8 = 0;
  *(uint8_t*)0x200001f9 = 0;
  *(uint8_t*)0x200001fa = 0;
  *(uint8_t*)0x200001fb = 0;
  *(uint8_t*)0x200001fc = 0;
  *(uint8_t*)0x200001fd = 0;
  *(uint8_t*)0x200001fe = 0;
  *(uint8_t*)0x200001ff = 0;
  *(uint8_t*)0x20000200 = 0;
  *(uint8_t*)0x20000201 = 0;
  *(uint8_t*)0x20000202 = 0;
  *(uint8_t*)0x20000203 = 0;
  *(uint8_t*)0x20000204 = 0;
  *(uint8_t*)0x20000205 = 0;
  *(uint8_t*)0x20000206 = 0;
  *(uint8_t*)0x20000207 = 0;
  *(uint8_t*)0x20000208 = 0;
  *(uint8_t*)0x20000209 = 0;
  *(uint8_t*)0x2000020a = 0;
  *(uint8_t*)0x2000020b = 0;
  *(uint8_t*)0x2000020c = 0;
  *(uint8_t*)0x2000020d = 0;
  *(uint8_t*)0x2000020e = 0;
  *(uint8_t*)0x2000020f = 0;
  *(uint8_t*)0x20000210 = 0;
  *(uint8_t*)0x20000211 = 0;
  *(uint8_t*)0x20000212 = 0;
  *(uint8_t*)0x20000213 = 0;
  *(uint8_t*)0x20000214 = 0;
  *(uint8_t*)0x20000215 = 0;
  *(uint8_t*)0x20000216 = 0;
  *(uint8_t*)0x20000217 = 0;
  *(uint8_t*)0x20000218 = 0;
  *(uint8_t*)0x20000219 = 0;
  *(uint8_t*)0x2000021a = 0;
  *(uint8_t*)0x2000021b = 0;
  *(uint8_t*)0x2000021c = 0;
  *(uint8_t*)0x2000021d = 0;
  *(uint8_t*)0x2000021e = 0;
  *(uint8_t*)0x2000021f = 0;
  *(uint8_t*)0x20000220 = 0;
  *(uint8_t*)0x20000221 = 0;
  *(uint8_t*)0x20000222 = 0;
  *(uint8_t*)0x20000223 = 0;
  *(uint8_t*)0x20000224 = 0;
  *(uint8_t*)0x20000225 = 0;
  *(uint8_t*)0x20000226 = 0;
  *(uint8_t*)0x20000227 = 0;
  syscall(__NR_ioctl, r[2], 0xc080aebe, 0x200001c0);
  syscall(__NR_socket, 0x10, 3, 0x10);
  memcpy((void*)0x20000300, "encrypted", 10);
  *(uint8_t*)0x20000340 = 0x73;
  *(uint8_t*)0x20000341 = 0x79;
  *(uint8_t*)0x20000342 = 0x7a;
  *(uint8_t*)0x20000343 = 0x23;
  *(uint8_t*)0x20000344 = 0;
  memcpy((void*)0x20000380, "/dev/vcs#", 10);
  syscall(__NR_request_key, 0x20000300, 0x20000340, 0x20000380, -1);
  memcpy((void*)0x200001c0, "/dev/vcs#", 10);
  syz_open_dev(0x200001c0, 0x10000008, 0x400000);
  syscall(__NR_mmap, 0x20011000, 0x3000, 1, 0x32, -1, 0);
  syscall(__NR_write, -1, 0x20011fd2, 0);
  memcpy((void*)0x20434ff8, "./file0", 8);
  memcpy((void*)0x20abf000, "./file0", 8);
  memcpy((void*)0x20f4c000, "jfs", 4);
  syscall(__NR_mount, 0x20434ff8, 0x20abf000, 0x20f4c000, 0x1023402, 0);
  syscall(__NR_dup2, -1, -1);
  syscall(__NR_ioctl, -1, 0xae41, 0);
  *(uint32_t*)0x20000800 = 0;
  *(uint16_t*)0x20000804 = 0;
  *(uint16_t*)0x20000806 = 0;
  *(uint32_t*)0x20000840 = 8;
  syscall(__NR_getsockopt, 0xffffff9c, 0x84, 0x77, 0x20000800, 0x20000840);
  syscall(__NR_mmap, 0x20011000, 0x3000, 1, 0x32, -1, 0);
  *(uint8_t*)0x20011fd2 = 0;
  *(uint8_t*)0x20011fd3 = 0;
  *(uint8_t*)0x20011fd4 = 0;
  *(uint8_t*)0x20011fd5 = 0;
  *(uint64_t*)0x20011fda = 0;
  *(uint64_t*)0x20011fe2 = 0;
  *(uint8_t*)0x20011fea = 0;
  *(uint8_t*)0x20011feb = 0;
  *(uint8_t*)0x20011fec = 0;
  *(uint8_t*)0x20011fed = 0;
  *(uint64_t*)0x20011ff2 = 0x77359400;
  *(uint64_t*)0x20011ffa = 0;
  syscall(__NR_write, -1, 0x20011fd2, 0x30);
  *(uint32_t*)0x20000300 = 0xe8;
  syscall(__NR_getsockopt, 0xffffff9c, 0x29, 0x23, 0x20000200, 0x20000300);
  *(uint32_t*)0x20000a80 = 0x14;
  syscall(__NR_getsockname, 0xffffff9c, 0x20000a40, 0x20000a80);
  *(uint32_t*)0x200010c0 = 0xe8;
  syscall(__NR_getsockopt, -1, 0x29, 0x23, 0x20000fc0, 0x200010c0);
  *(uint16_t*)0x20000040 = 0xa;
  *(uint16_t*)0x20000042 = htobe16(0x4e24);
  *(uint32_t*)0x20000044 = 0;
  *(uint8_t*)0x20000048 = 0;
  *(uint8_t*)0x20000049 = 0;
  *(uint8_t*)0x2000004a = 0;
  *(uint8_t*)0x2000004b = 0;
  *(uint8_t*)0x2000004c = 0;
  *(uint8_t*)0x2000004d = 0;
  *(uint8_t*)0x2000004e = 0;
  *(uint8_t*)0x2000004f = 0;
  *(uint8_t*)0x20000050 = 0;
  *(uint8_t*)0x20000051 = 0;
  *(uint8_t*)0x20000052 = 0;
  *(uint8_t*)0x20000053 = 0;
  *(uint8_t*)0x20000054 = 0;
  *(uint8_t*)0x20000055 = 0;
  *(uint8_t*)0x20000056 = 0;
  *(uint8_t*)0x20000057 = 0;
  *(uint32_t*)0x20000058 = 0;
  syscall(__NR_bind, -1, 0x20000040, 0x1c);
  return 0;
}
