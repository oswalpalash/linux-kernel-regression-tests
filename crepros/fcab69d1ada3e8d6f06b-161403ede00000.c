// https://syzkaller.appspot.com/bug?id=e5054b48470ac0510465d011d8bb2bb72669e41a
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
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  intptr_t res = 0;
  res = syscall(__NR_pipe, 0x20000080ul);
  if (res != -1) {
    r[0] = *(uint32_t*)0x20000080;
    r[1] = *(uint32_t*)0x20000084;
  }
  *(uint64_t*)0x200000c0 = 0x20000180;
  memcpy((void*)0x20000180, "\x77\x69\x0a\xdd\xcf\xbe\x1f\xbb\x66\xec", 10);
  *(uint64_t*)0x200000c8 = 0xff3b;
  syscall(__NR_vmsplice, r[1], 0x200000c0ul, 1ul, 1ul);
  syscall(__NR_close, r[1]);
  memcpy((void*)0x20000000, "/sys/kernel/debug/bluetooth/6lowpan_enable\000",
         43);
  res = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20000000ul, 2ul, 0ul);
  if (res != -1)
    r[2] = res;
  syscall(__NR_splice, r[0], 0ul, r[2], 0ul, 0x10001ul, 0xaul);
  syscall(__NR_read, r[1], 0ul, 0ul);
  return 0;
}
