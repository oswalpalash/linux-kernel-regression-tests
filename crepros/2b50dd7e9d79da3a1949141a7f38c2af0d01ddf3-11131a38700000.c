// https://syzkaller.appspot.com/bug?id=2b50dd7e9d79da3a1949141a7f38c2af0d01ddf3
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

uint64_t r[2] = {0xffffffffffffffff, 0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x20000000ul, 0x1000000ul, 7ul, 0x32ul, -1, 0ul);
  syscall(__NR_mmap, 0x21000000ul, 0x1000ul, 0ul, 0x32ul, -1, 0ul);
  intptr_t res = 0;
  syscall(__NR_socket, 0x11ul, 3ul, 0x300);
  res = syscall(__NR_pipe, 0x20000180ul);
  if (res != -1)
    r[0] = *(uint32_t*)0x20000184;
  res = syscall(__NR_socket, 0xaul, 1ul, 0);
  if (res != -1)
    r[1] = res;
  *(uint16_t*)0x20000040 = 0xa;
  *(uint16_t*)0x20000042 = htobe16(2);
  *(uint32_t*)0x20000044 = htobe32(0);
  *(uint64_t*)0x20000048 = htobe64(0);
  *(uint64_t*)0x20000050 = htobe64(1);
  *(uint32_t*)0x20000058 = 0;
  syscall(__NR_bind, r[1], 0x20000040ul, 0x1cul);
  *(uint16_t*)0x20b63fe4 = 0xa;
  *(uint16_t*)0x20b63fe6 = htobe16(2);
  *(uint32_t*)0x20b63fe8 = htobe32(0);
  *(uint64_t*)0x20b63fec = htobe64(0);
  *(uint64_t*)0x20b63ff4 = htobe64(1);
  *(uint32_t*)0x20b63ffc = 0;
  syscall(__NR_sendto, r[1], 0ul, 0ul, 0x22004001ul, 0x20b63fe4ul, 0x1cul);
  syscall(__NR_write, r[1], 0x200004c0ul, 0x78ul);
  memcpy(
      (void*)0x200003c0,
      "\x44\xf9\xb1\x28\xb1\xcd\xc8\x85\xc9\xc5\x33\xb2\x1f\x47\xb8\x5d\x9c\xdb"
      "\xf1\xdf\x1e\x2d\xa7\x1e\x57\x8d\xc6\xb9\x1d\x09\xf7\xab\x15\x37\x85\x71"
      "\xd8\xe2\x75\x46\x09\x00\x00\x00\x6e\x75\x43\x69\x14\xab\x71\x75\x28\xee"
      "\x4b\x7a\x9b\xea\xf9\x08\xd1\x11\x37\xc1\x19\x03\x06\x4e\x83\xb4\x95\x1f"
      "\x4d\x43\x3a\x54\x04\x97\x0c\x85\xd9\x2d\x70\x83\xfd\x38\x84\x4c\xbb\x0c"
      "\x6c\x5e\xb5\x08\xdd\xc2\xdc\x7a\x59\x0a\xa7\x94\x1b\x1e\x9e\xeb\x5a\xff"
      "\xff\xff\xff\x00\x00\x00\x00\xbf\xa7\x84\xcb\xf5\x50\xbf\x30\x74\xfb\x0d"
      "\x77\x5d\xa4\xdf\x5a\x3f\x48\xbb\xdf\x45\x2e\xeb\x6b\x92\x3d\xa9\xd0\xe2"
      "\x5b\x80\xf7\x6a\x87\x36\x64\xb5\x75\x34\x44\x6c\x3c\xd6\xaf\x10\xf0\xcd"
      "\x57\x1e\xa4\xe1\x4f\x98\x69\x53\xbe\x01\x8f\x0c\x6f\x57\xf9\x26\xac\x95"
      "\x9a\x56\x28\xc6\x50\x88\xfb\xe0\xc8\x7f\xbe\x6c\xbc\xda\x46\x62\xd2\xa1"
      "\x2f\x6d\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
      215);
  syscall(__NR_sendto, r[1], 0x200003c0ul, 0xd7ul, 1ul, 0ul, 0ul);
  syscall(__NR_splice, r[1], 0ul, r[0], 0ul, 0x406f40aul, 0ul);
  return 0;
}
