// https://syzkaller.appspot.com/bug?id=d0d8879a66b36bd60a0d815b7755e558a9afc82f
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

uint64_t r[1] = {0xffffffffffffffff};

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
  intptr_t res = 0;
  memcpy((void*)0x20000000, "/dev/sequencer2\000", 16);
  syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000000ul,
          /*flags=*/0ul, /*mode=*/0ul);
  memcpy((void*)0x20000100, "/dev/audio\000", 11);
  res =
      syscall(__NR_openat, /*fd=*/0xffffffffffffff9cul, /*file=*/0x20000100ul,
              /*flags=O_SYNC|O_NONBLOCK|O_LARGEFILE|O_CREAT|O_RDWR*/ 0x109842ul,
              /*mode=*/0ul);
  if (res != -1)
    r[0] = res;
  *(uint32_t*)0x20000000 = 7;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc0045005, /*arg=*/0x20000000ul);
  syscall(__NR_mmap, /*addr=*/0x20ffd000ul, /*len=*/0x2000ul,
          /*prot=PROT_GROWSUP|PROT_WRITE*/ 0x2000002ul,
          /*flags=MAP_POPULATE|MAP_FIXED|MAP_PRIVATE*/ 0x8012ul, /*fd=*/r[0],
          /*offset=*/0ul);
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0x5008, /*arg=*/0ul);
  return 0;
}
