// https://syzkaller.appspot.com/bug?id=ddb673727990990c6ded3e7cc220e39abfc244ab
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

static uintptr_t syz_open_dev(uintptr_t a0, uintptr_t a1, uintptr_t a2)
{
  if (a0 == 0xc || a0 == 0xb) {
    char buf[128];
    sprintf(buf, "/dev/%s/%d:%d", a0 == 0xc ? "char" : "block",
            (uint8_t)a1, (uint8_t)a2);
    return open(buf, O_RDWR, 0);
  } else {
    char buf[1024];
    char* hash;
    strncpy(buf, (char*)a0, sizeof(buf));
    buf[sizeof(buf) - 1] = 0;
    while ((hash = strchr(buf, '#'))) {
      *hash = '0' + (char)(a1 % 10);
      a1 /= 10;
    }
    return open(buf, a2, 0);
  }
}

long r[22];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socket, 0x2ul, 0x1ul, 0x0ul);
  memcpy((void*)0x20001ff0, "\x69\x70\x5f\x76\x74\x69\x30\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00",
         16);
  r[3] = syscall(__NR_ioctl, r[1], 0x89a0ul, 0x20001ff0ul);
  memcpy((void*)0x20957ff3,
         "\x2f\x64\x65\x76\x2f\x6e\x65\x74\x2f\x74\x75\x6e\x00", 13);
  r[6] = syz_open_dev(0x20957ff3ul, 0x0ul, 0x40ul);
  *(uint32_t*)0x20fe9ffc = (uint32_t)0x4;
  r[8] = syscall(__NR_ioctl, r[6], 0x400454daul, 0x20fe9ffcul);
  memcpy((void*)0x20533000, "\x02\x00\x00\x00\x04\x00\x00\x00\x00\x04"
                            "\x00\x80\x00\xe9\xbc\x22",
         16);
  *(uint32_t*)0x20533010 = (uint32_t)0x10001;
  *(uint32_t*)0x20533014 = (uint32_t)0x0;
  *(uint64_t*)0x20533018 = (uint64_t)0x20012fe8;
  *(uint32_t*)0x20012fe8 = (uint32_t)0x0;
  *(uint32_t*)0x20012fec = (uint32_t)0x0;
  *(uint32_t*)0x20012ff0 = (uint32_t)0x0;
  *(uint32_t*)0x20012ff4 = (uint32_t)0xfffffffffffffffb;
  *(uint32_t*)0x20012ff8 = (uint32_t)0x81c7;
  *(uint16_t*)0x20012ffc = (uint16_t)0x5;
  *(uint16_t*)0x20012ffe = (uint16_t)0x26f;
  r[20] = syscall(__NR_ioctl, r[6], 0x400454caul, 0x20533000ul);
  r[21] = syscall(__NR_shmctl, 0x0ul, 0x3ul, 0x20b30f4eul);
}

int main()
{
  loop();
  return 0;
}
