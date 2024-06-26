// https://syzkaller.appspot.com/bug?id=874bf0a9464205c21d77d8a3e7bc9dff824329a5
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

  *(uint32_t*)0x200019c0 = 0x10;
  *(uint32_t*)0x200019c4 = 4;
  *(uint32_t*)0x200019c8 = 7;
  *(uint32_t*)0x200019cc = 0x3f;
  *(uint32_t*)0x200019d0 = 9;
  *(uint32_t*)0x200019d4 = 0x10001;
  syscall(__NR_open_by_handle_at, /*mountdirfd=*/-1, /*handle=*/0x200019c0ul,
          /*flags=FASYNC*/ 0x2000ul);
  return 0;
}
