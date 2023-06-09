// https://syzkaller.appspot.com/bug?id=8f063539d4ecf1faf3132624b57a641e923ee25a
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

uint64_t r[3] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  syscall(__NR_ioctl, -1, 0x8912, 0x20000280);
  memcpy((void*)0x20000040, "/dev/loop0", 11);
  memcpy((void*)0x200000c0, "./file0", 8);
  memcpy((void*)0x20000100, "fuseblk", 8);
  memcpy((void*)0x20000300, "fd", 2);
  *(uint8_t*)0x20000302 = 0x3d;
  sprintf((char*)0x20000303, "0x%016llx", (long long)-1);
  *(uint8_t*)0x20000315 = 0x2c;
  memcpy((void*)0x20000316, "rootmode", 8);
  *(uint8_t*)0x2000031e = 0x3d;
  sprintf((char*)0x2000031f, "%023llo", (long long)0x1000);
  *(uint8_t*)0x20000336 = 0x2c;
  memcpy((void*)0x20000337, "user_id", 7);
  *(uint8_t*)0x2000033e = 0x3d;
  sprintf((char*)0x2000033f, "%020llu", (long long)0);
  *(uint8_t*)0x20000353 = 0x2c;
  memcpy((void*)0x20000354, "group_id", 8);
  *(uint8_t*)0x2000035c = 0x3d;
  sprintf((char*)0x2000035d, "%020llu", (long long)0);
  *(uint8_t*)0x20000371 = 0x2c;
  memcpy((void*)0x20000372, "allow_other", 11);
  *(uint8_t*)0x2000037d = 0x2c;
  *(uint8_t*)0x2000037e = 0;
  syscall(__NR_mount, 0x20000040, 0x200000c0, 0x20000100, 5, 0x20000300);
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
  *(uint32_t*)0x200001c0 = 2;
  *(uint32_t*)0x200001c4 = 0;
  *(uint64_t*)0x200001c8 = 0x488;
  *(uint64_t*)0x200001d0 = 0;
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
  syscall(__NR_ioctl, r[2], 0x4080aebf, 0x200001c0);
  syscall(__NR_socketpair, 0xa, 2, 0x88, 0x20000040);
  *(uint32_t*)0x20001f00 = 0;
  *(uint32_t*)0x20001f04 = 0;
  *(uint32_t*)0x20001f08 = 0;
  *(uint32_t*)0x20001f0c = 0;
  *(uint32_t*)0x20001f10 = 0;
  *(uint64_t*)0x20001f18 = 0;
  syscall(__NR_ioctl, -1, 0x801c581f, 0x20001f00);
  *(uint32_t*)0x20000240 = 0;
  *(uint64_t*)0x20000248 = 0x77359400;
  *(uint64_t*)0x20000250 = 0;
  *(uint8_t*)0x20000258 = 0;
  *(uint8_t*)0x20000259 = 0;
  *(uint8_t*)0x2000025a = 0;
  *(uint8_t*)0x2000025b = 0;
  *(uint32_t*)0x2000025c = 1;
  *(uint8_t*)0x20000260 = 0;
  *(uint32_t*)0x20000264 = 0;
  *(uint32_t*)0x20000268 = 0;
  *(uint32_t*)0x2000026c = 0;
  *(uint32_t*)0x20000270 = 0;
  *(uint32_t*)0x20000274 = 0;
  *(uint32_t*)0x20000278 = 0;
  *(uint32_t*)0x2000027c = 0;
  *(uint32_t*)0x20000280 = 0;
  *(uint32_t*)0x20000284 = 0;
  *(uint32_t*)0x20000288 = 0;
  syscall(__NR_ioctl, -1, 0x80045300, 0x20000240);
  *(uint32_t*)0x200007c0 = 2;
  syscall(__NR_setsockopt, -1, 0x114, 8, 0x200007c0, 4);
  syscall(__NR_ioctl, -1, 0x8912, 0x400200);
  *(uint32_t*)0x20001f00 = 0;
  *(uint32_t*)0x20001f04 = 0;
  *(uint32_t*)0x20001f08 = 0;
  *(uint32_t*)0x20001f0c = 0;
  *(uint32_t*)0x20001f10 = 0;
  *(uint64_t*)0x20001f18 = 0;
  syscall(__NR_ioctl, -1, 0x801c581f, 0x20001f00);
  return 0;
}
