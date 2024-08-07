// https://syzkaller.appspot.com/bug?id=6e96f5bd56bfecdaa0bfeaf2695a25cc80bfbd9a
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 3ul, 0x32ul, -1, 0);
  use_temporary_dir();
  intptr_t res = 0;
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20001080 = 0;
  *(uint32_t*)0x20001088 = 0;
  *(uint64_t*)0x20001090 = 0x20000080;
  *(uint64_t*)0x20000080 = 0x20000f80;
  memcpy((void*)0x20000f80,
         "\x54\x00\x00\x00\x02\x06\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x0d\x00\x03\x00\x68\x61\x73\x68\x3a\x6e\x65\x74\x00\x00"
         "\x00\x00\x05\x00\x04\x00\x00\x00\x00\x00\x09\x00\x02\x00\x73\x79\x7a"
         "\x31\x00\x00\x00\x00\x0c\x00\x07\x80\x08\x00\x08\x40\x00\x00\x00\xf0"
         "\x05\x00\x05\x00\x02\x00\x00\x00\x05\x00\x01\x00\x07\x00\x00\x00",
         84);
  *(uint64_t*)0x20000088 = 0x54;
  *(uint64_t*)0x20001098 = 1;
  *(uint64_t*)0x200010a0 = 0;
  *(uint64_t*)0x200010a8 = 0;
  *(uint32_t*)0x200010b0 = 0;
  syscall(__NR_sendmsg, r[0], 0x20001080ul, 0ul);
  res = syscall(__NR_socket, 0x10ul, 3ul, 0xcul);
  if (res != -1)
    r[1] = res;
  *(uint64_t*)0x20000d00 = 0;
  *(uint32_t*)0x20000d08 = 0;
  *(uint64_t*)0x20000d10 = 0x20000cc0;
  *(uint64_t*)0x20000cc0 = 0x20000080;
  memcpy((void*)0x20000080,
         "\x44\x00\x00\x00\x09\x06\x01\x08\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x05\x00\x01\x00\x07\x00\x00\x00\x1c\x00\x07\x80\x0c\x00"
         "\x02\x80\x08\x00\x01\x40\x00\x22\x00\x00\x0c\x00\x01\x80\x08\x00\x01"
         "\x40\xac\x14\x14\xaa\x09\x00\x02\x00\x73\x79\x7a\x31\x00\x00\x00\x00"
         "\xc5\x7e\x56\x70\xd2\x32\xc8\x74\xf6\x57\x9e\x37",
         80);
  *(uint64_t*)0x20000cc8 = 0x44;
  *(uint64_t*)0x20000d18 = 1;
  *(uint64_t*)0x20000d20 = 0;
  *(uint64_t*)0x20000d28 = 0;
  *(uint32_t*)0x20000d30 = 0;
  syscall(__NR_sendmsg, r[1], 0x20000d00ul, 0ul);
  return 0;
}
