// https://syzkaller.appspot.com/bug?id=de6519e18a472f06a6b530c84c3be8a29c554900
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>

#ifndef __NR_bpf
#define __NR_bpf 321
#endif

long r[128];
void loop()
{
  memset(r, -1, sizeof(r));
  r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                 0xfffffffffffffffful, 0x0ul);
  r[1] = syscall(__NR_socketpair, 0x1ul, 0x5ul, 0x0ul, 0x2000dff8ul);
  if (r[1] != -1)
    r[2] = *(uint32_t*)0x2000dff8;
  if (r[1] != -1)
    r[3] = *(uint32_t*)0x2000dffc;
  *(uint32_t*)0x20b4cfd0 = (uint32_t)0x1;
  *(uint32_t*)0x20b4cfd4 = (uint32_t)0x2;
  *(uint64_t*)0x20b4cfd8 = (uint64_t)0x20ef3000;
  *(uint64_t*)0x20b4cfe0 = (uint64_t)0x20ccdffa;
  *(uint32_t*)0x20b4cfe8 = (uint32_t)0x1;
  *(uint32_t*)0x20b4cfec = (uint32_t)0x80;
  *(uint64_t*)0x20b4cff0 = (uint64_t)0x2000a000;
  *(uint32_t*)0x20b4cff8 = (uint32_t)0x0;
  *(uint32_t*)0x20b4cffc = (uint32_t)0x0;
  *(uint8_t*)0x20ef3000 = (uint8_t)0x8db7;
  *(uint8_t*)0x20ef3001 = (uint8_t)0x0;
  *(uint16_t*)0x20ef3002 = (uint16_t)0x0;
  *(uint32_t*)0x20ef3004 = (uint32_t)0x0;
  *(uint8_t*)0x20ef3008 = (uint8_t)0xd395;
  *(uint8_t*)0x20ef3009 = (uint8_t)0x0;
  *(uint16_t*)0x20ef300a = (uint16_t)0x0;
  *(uint32_t*)0x20ef300c = (uint32_t)0x0;
  memcpy((void*)0x20ccdffa, "\x73\x79\x73\x65\x4f\x00", 6);
  r[22] = syscall(__NR_bpf, 0x5ul, 0x20b4cfd0ul, 0x30ul);
  r[23] = syscall(__NR_socket, 0x29ul, 0x5ul, 0x0ul);
  *(uint32_t*)0x2081cff8 = r[2];
  *(uint32_t*)0x2081cffc = r[22];
  r[26] = syscall(__NR_ioctl, r[23], 0x89e0ul, 0x2081cff8ul);
  *(uint64_t*)0x204eff4c = (uint64_t)0x201c0000;
  *(uint32_t*)0x204eff54 = (uint32_t)0x60;
  *(uint64_t*)0x204eff5c = (uint64_t)0x20966ff0;
  *(uint64_t*)0x204eff64 = (uint64_t)0x1;
  *(uint64_t*)0x204eff6c = (uint64_t)0x20cf7000;
  *(uint64_t*)0x204eff74 = (uint64_t)0x9;
  *(uint32_t*)0x204eff7c = (uint32_t)0x200040c4;
  *(uint32_t*)0x204eff84 = (uint32_t)0x40;
  *(uint64_t*)0x204eff88 = (uint64_t)0x205a9ff4;
  *(uint32_t*)0x204eff90 = (uint32_t)0xc;
  *(uint64_t*)0x204eff98 = (uint64_t)0x20203ff0;
  *(uint64_t*)0x204effa0 = (uint64_t)0x1;
  *(uint64_t*)0x204effa8 = (uint64_t)0x206eae98;
  *(uint64_t*)0x204effb0 = (uint64_t)0x3;
  *(uint32_t*)0x204effb8 = (uint32_t)0x40040;
  *(uint32_t*)0x204effc0 = (uint32_t)0x4;
  *(uint64_t*)0x204effc4 = (uint64_t)0x20000000;
  *(uint32_t*)0x204effcc = (uint32_t)0x0;
  *(uint64_t*)0x204effd4 = (uint64_t)0x2075a000;
  *(uint64_t*)0x204effdc = (uint64_t)0x6;
  *(uint64_t*)0x204effe4 = (uint64_t)0x206a28d7;
  *(uint64_t*)0x204effec = (uint64_t)0x4;
  *(uint32_t*)0x204efff4 = (uint32_t)0x800;
  *(uint32_t*)0x204efffc = (uint32_t)0xec;
  *(uint16_t*)0x201c0000 = (uint16_t)0x27;
  *(uint32_t*)0x201c0004 = (uint32_t)0x80000001;
  *(uint32_t*)0x201c0008 = (uint32_t)0x7;
  *(uint32_t*)0x201c000c = (uint32_t)0x3;
  *(uint8_t*)0x201c0010 = (uint8_t)0x3;
  *(uint8_t*)0x201c0011 = (uint8_t)0x3;
  memcpy((void*)0x201c0012,
         "\xdf\xf9\x79\xe5\x43\x5f\x52\xee\x46\xf6\xb7\x26\x2b\x54\x3a"
         "\x5e\x7f\xbc\x2b\x0e\xd1\x3a\x42\x5f\xcc\xb3\xa5\x20\x03\xbe"
         "\xb9\x9f\x94\x7d\x2a\xde\xd1\x30\xcb\xb1\x66\x71\x79\xd7\xc3"
         "\x05\xa7\x2d\x49\xc2\x42\xf3\xb4\x45\x73\x45\x45\xe1\x43\x4a"
         "\x85\x8f\x15",
         63);
  *(uint64_t*)0x201c0058 = (uint64_t)0x262;
  *(uint64_t*)0x20966ff0 = (uint64_t)0x20da3f54;
  *(uint64_t*)0x20966ff8 = (uint64_t)0x0;
  *(uint64_t*)0x20cf7000 = (uint64_t)0x10;
  *(uint32_t*)0x20cf7008 = (uint32_t)0x13f;
  *(uint32_t*)0x20cf700c = (uint32_t)0x101;
  *(uint64_t*)0x20cf7010 = (uint64_t)0x10;
  *(uint32_t*)0x20cf7018 = (uint32_t)0x7;
  *(uint32_t*)0x20cf701c = (uint32_t)0x4;
  *(uint64_t*)0x20cf7020 = (uint64_t)0x10;
  *(uint32_t*)0x20cf7028 = (uint32_t)0x10a;
  *(uint32_t*)0x20cf702c = (uint32_t)0x7f;
  *(uint64_t*)0x20cf7030 = (uint64_t)0x10;
  *(uint32_t*)0x20cf7038 = (uint32_t)0x1;
  *(uint32_t*)0x20cf703c = (uint32_t)0x7;
  *(uint64_t*)0x20cf7040 = (uint64_t)0x10;
  *(uint32_t*)0x20cf7048 = (uint32_t)0x11f;
  *(uint32_t*)0x20cf704c = (uint32_t)0x3;
  *(uint64_t*)0x20cf7050 = (uint64_t)0x10;
  *(uint32_t*)0x20cf7058 = (uint32_t)0x10d;
  *(uint32_t*)0x20cf705c = (uint32_t)0xfb;
  *(uint64_t*)0x20cf7060 = (uint64_t)0x10;
  *(uint32_t*)0x20cf7068 = (uint32_t)0x24459b4b5bdf74ef;
  *(uint32_t*)0x20cf706c = (uint32_t)0x5;
  *(uint64_t*)0x20cf7070 = (uint64_t)0x10;
  *(uint32_t*)0x20cf7078 = (uint32_t)0x103;
  *(uint32_t*)0x20cf707c = (uint32_t)0x2;
  *(uint64_t*)0x20cf7080 = (uint64_t)0x10;
  *(uint32_t*)0x20cf7088 = (uint32_t)0xff;
  *(uint32_t*)0x20cf708c = (uint32_t)0x40;
  *(uint16_t*)0x205a9ff4 = (uint16_t)0x10;
  *(uint16_t*)0x205a9ff6 = (uint16_t)0x0;
  *(uint32_t*)0x205a9ff8 = (uint32_t)0x3;
  *(uint32_t*)0x205a9ffc = (uint32_t)0x2;
  *(uint64_t*)0x20203ff0 = (uint64_t)0x20ef7fe0;
  *(uint64_t*)0x20203ff8 = (uint64_t)0x0;
  *(uint64_t*)0x206eae98 = (uint64_t)0x10;
  *(uint32_t*)0x206eaea0 = (uint32_t)0x84;
  *(uint32_t*)0x206eaea4 = (uint32_t)0x400;
  *(uint64_t*)0x206eaea8 = (uint64_t)0x10;
  *(uint32_t*)0x206eaeb0 = (uint32_t)0x29;
  *(uint32_t*)0x206eaeb4 = (uint32_t)0x4;
  *(uint64_t*)0x206eaeb8 = (uint64_t)0x10;
  *(uint32_t*)0x206eaec0 = (uint32_t)0x11;
  *(uint32_t*)0x206eaec4 = (uint32_t)0xfffffffffffffff7;
  *(uint64_t*)0x2075a000 = (uint64_t)0x20f6cf5e;
  *(uint64_t*)0x2075a008 = (uint64_t)0x0;
  *(uint64_t*)0x2075a010 = (uint64_t)0x20f74f56;
  *(uint64_t*)0x2075a018 = (uint64_t)0x0;
  *(uint64_t*)0x2075a020 = (uint64_t)0x200ecf60;
  *(uint64_t*)0x2075a028 = (uint64_t)0x0;
  *(uint64_t*)0x2075a030 = (uint64_t)0x20705f8c;
  *(uint64_t*)0x2075a038 = (uint64_t)0x0;
  *(uint64_t*)0x2075a040 = (uint64_t)0x20f4f000;
  *(uint64_t*)0x2075a048 = (uint64_t)0x0;
  *(uint64_t*)0x2075a050 = (uint64_t)0x20e9f000;
  *(uint64_t*)0x2075a058 = (uint64_t)0x0;
  *(uint64_t*)0x206a28d7 = (uint64_t)0x10;
  *(uint32_t*)0x206a28df = (uint32_t)0x11f;
  *(uint32_t*)0x206a28e3 = (uint32_t)0x401;
  *(uint64_t*)0x206a28e7 = (uint64_t)0x10;
  *(uint32_t*)0x206a28ef = (uint32_t)0x11f;
  *(uint32_t*)0x206a28f3 = (uint32_t)0x40;
  *(uint64_t*)0x206a28f7 = (uint64_t)0x10;
  *(uint32_t*)0x206a28ff = (uint32_t)0x1ff;
  *(uint32_t*)0x206a2903 = (uint32_t)0x8;
  *(uint64_t*)0x206a2907 = (uint64_t)0x10;
  *(uint32_t*)0x206a290f = (uint32_t)0xf1e3f6cfee039a14;
  *(uint32_t*)0x206a2913 = (uint32_t)0x3;
  r[127] = syscall(__NR_sendmmsg, r[3], 0x204eff4cul, 0x3ul, 0x0ul);
}

int main()
{
  loop();
  return 0;
}
