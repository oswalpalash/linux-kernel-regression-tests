// https://syzkaller.appspot.com/bug?id=2b061d1fabd9760e98f92163543189b637c4af36
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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                                  \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)                      \
  if ((bf_off) == 0 && (bf_len) == 0) {                                        \
    *(type*)(addr) = (type)(val);                                              \
  } else {                                                                     \
    type new_val = *(type*)(addr);                                             \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));                     \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);          \
    *(type*)(addr) = new_val;                                                  \
  }

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

#ifndef __NR_bpf
#define __NR_bpf 321
#endif
#ifndef __NR_renameat2
#define __NR_renameat2 316
#endif

uint64_t r[5] = {0xffffffffffffffff, 0xffffffffffffffff, 0x0,
                 0xffffffffffffffff, 0x0};

int main(void)
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  long res = 0;
  memcpy((void*)0x20bee000, "./control", 10);
  memcpy((void*)0x2003a000, "./control", 10);
  syscall(__NR_renameat2, -1, 0x20bee000, -1, 0x2003a000, 2);
  memcpy((void*)0x20000000, "/dev/usbmon#", 13);
  res = syz_open_dev(0x20000000, 0, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000380, "/dev/bus/usb/00#/00#", 21);
  syz_open_dev(0x20000380, 0x202, 0);
  syscall(__NR_pread64, r[0], 0x200003c0, 0x1000, 0);
  *(uint32_t*)0x20000000 = 1;
  *(uint16_t*)0x20000008 = 0xa;
  *(uint16_t*)0x2000000a = htobe16(0);
  *(uint32_t*)0x2000000c = 0;
  *(uint8_t*)0x20000010 = -1;
  *(uint8_t*)0x20000011 = 2;
  *(uint8_t*)0x20000012 = 0;
  *(uint8_t*)0x20000013 = 0;
  *(uint8_t*)0x20000014 = 0;
  *(uint8_t*)0x20000015 = 0;
  *(uint8_t*)0x20000016 = 0;
  *(uint8_t*)0x20000017 = 0;
  *(uint8_t*)0x20000018 = 0;
  *(uint8_t*)0x20000019 = 0;
  *(uint8_t*)0x2000001a = 0;
  *(uint8_t*)0x2000001b = 0;
  *(uint8_t*)0x2000001c = 0;
  *(uint8_t*)0x2000001d = 0;
  *(uint8_t*)0x2000001e = 0;
  *(uint8_t*)0x2000001f = 1;
  *(uint32_t*)0x20000020 = 0;
  *(uint16_t*)0x20000088 = 0xa;
  *(uint16_t*)0x2000008a = htobe16(0);
  *(uint32_t*)0x2000008c = 0;
  *(uint8_t*)0x20000090 = 0;
  *(uint8_t*)0x20000091 = 0;
  *(uint8_t*)0x20000092 = 0;
  *(uint8_t*)0x20000093 = 0;
  *(uint8_t*)0x20000094 = 0;
  *(uint8_t*)0x20000095 = 0;
  *(uint8_t*)0x20000096 = 0;
  *(uint8_t*)0x20000097 = 0;
  *(uint8_t*)0x20000098 = 0;
  *(uint8_t*)0x20000099 = 0;
  *(uint8_t*)0x2000009a = -1;
  *(uint8_t*)0x2000009b = -1;
  *(uint32_t*)0x2000009c = htobe32(0);
  *(uint32_t*)0x200000a0 = 0;
  syscall(__NR_setsockopt, -1, 0x29, 0x2e, 0x20000000, 0x108);
  *(uint64_t*)0x20001380 = 0x20001200;
  *(uint64_t*)0x20001388 = 0x94;
  syscall(__NR_preadv, -1, 0x20001380, 1, 0);
  syscall(__NR_preadv, -1, 0x20000100, 0x12f, 0x10400003);
  memcpy((void*)0x20000400, "/dev/usbmon#", 13);
  res = syz_open_dev(0x20000400, 0, 0);
  if (res != -1)
    r[1] = res;
  syscall(__NR_mmap, 0x20e00000, 0x200000, 0xff, 0x8012, r[1], 0);
  memcpy((void*)0x20000300, "/dev/loop#", 11);
  syz_open_dev(0x20000300, 0, 0xeffbfffffffffffe);
  syscall(__NR_listen, -1, 0x20000003);
  *(uint32_t*)0x20047ffc = 0x80;
  syscall(__NR_accept4, -1, 0x204d4000, 0x20047ffc, 0);
  *(uint32_t*)0x20000500 = 0;
  *(uint16_t*)0x20000504 = 0x40;
  *(uint32_t*)0x20000540 = 8;
  res = syscall(__NR_getsockopt, -1, 0x84, 0x18, 0x20000500, 0x20000540);
  if (res != -1)
    r[2] = *(uint32_t*)0x20000500;
  *(uint32_t*)0x20000580 = r[2];
  *(uint16_t*)0x20000584 = 3;
  *(uint16_t*)0x20000586 = 0xfff7;
  *(uint32_t*)0x20000588 = 0xb2;
  *(uint32_t*)0x2000058c = 0xe22;
  *(uint32_t*)0x20000590 = 6;
  syscall(__NR_setsockopt, -1, 0x84, 1, 0x20000580, 0x14);
  *(uint64_t*)0x20000b00 = 0x200001c0;
  *(uint16_t*)0x200001c0 = 2;
  *(uint16_t*)0x200001c2 = htobe16(0x4e20);
  *(uint32_t*)0x200001c4 = htobe32(0xe0000002);
  *(uint8_t*)0x200001c8 = 0;
  *(uint8_t*)0x200001c9 = 0;
  *(uint8_t*)0x200001ca = 0;
  *(uint8_t*)0x200001cb = 0;
  *(uint8_t*)0x200001cc = 0;
  *(uint8_t*)0x200001cd = 0;
  *(uint8_t*)0x200001ce = 0;
  *(uint8_t*)0x200001cf = 0;
  *(uint32_t*)0x20000b08 = 0x10;
  *(uint64_t*)0x20000b10 = 0x20000700;
  *(uint64_t*)0x20000700 = 0x20000640;
  *(uint64_t*)0x20000708 = 0;
  *(uint64_t*)0x20000710 = 0x200006c0;
  *(uint64_t*)0x20000718 = 0;
  *(uint64_t*)0x20000b18 = 2;
  *(uint64_t*)0x20000b20 = 0x20000740;
  *(uint64_t*)0x20000740 = 0x20;
  *(uint32_t*)0x20000748 = 0x84;
  *(uint32_t*)0x2000074c = 8;
  *(uint8_t*)0x20000750 = -1;
  *(uint8_t*)0x20000751 = 2;
  *(uint8_t*)0x20000752 = 0;
  *(uint8_t*)0x20000753 = 0;
  *(uint8_t*)0x20000754 = 0;
  *(uint8_t*)0x20000755 = 0;
  *(uint8_t*)0x20000756 = 0;
  *(uint8_t*)0x20000757 = 0;
  *(uint8_t*)0x20000758 = 0;
  *(uint8_t*)0x20000759 = 0;
  *(uint8_t*)0x2000075a = 0;
  *(uint8_t*)0x2000075b = 0;
  *(uint8_t*)0x2000075c = 0;
  *(uint8_t*)0x2000075d = 0;
  *(uint8_t*)0x2000075e = 0;
  *(uint8_t*)0x2000075f = 1;
  *(uint64_t*)0x20000760 = 0x20;
  *(uint32_t*)0x20000768 = 0x84;
  *(uint32_t*)0x2000076c = 2;
  *(uint16_t*)0x20000770 = 0x3f;
  *(uint16_t*)0x20000772 = 1;
  *(uint32_t*)0x20000774 = 1;
  *(uint32_t*)0x20000778 = 1;
  *(uint32_t*)0x2000077c = r[2];
  *(uint64_t*)0x20000780 = 0x18;
  *(uint32_t*)0x20000788 = 0x84;
  *(uint32_t*)0x2000078c = 6;
  *(uint16_t*)0x20000790 = 0xd5c2;
  *(uint64_t*)0x20000798 = 0x18;
  *(uint32_t*)0x200007a0 = 0x84;
  *(uint32_t*)0x200007a4 = 6;
  *(uint16_t*)0x200007a8 = 9;
  *(uint64_t*)0x20000b28 = 0x70;
  *(uint32_t*)0x20000b30 = 0x4000000;
  *(uint64_t*)0x20000b38 = 0x200007c0;
  *(uint16_t*)0x200007c0 = 2;
  *(uint16_t*)0x200007c2 = htobe16(0x4e21);
  *(uint32_t*)0x200007c4 = htobe32(0xe0000001);
  *(uint8_t*)0x200007c8 = 0;
  *(uint8_t*)0x200007c9 = 0;
  *(uint8_t*)0x200007ca = 0;
  *(uint8_t*)0x200007cb = 0;
  *(uint8_t*)0x200007cc = 0;
  *(uint8_t*)0x200007cd = 0;
  *(uint8_t*)0x200007ce = 0;
  *(uint8_t*)0x200007cf = 0;
  *(uint32_t*)0x20000b40 = 0x10;
  *(uint64_t*)0x20000b48 = 0x20000a80;
  *(uint64_t*)0x20000a80 = 0x20000800;
  *(uint64_t*)0x20000a88 = 0;
  *(uint64_t*)0x20000a90 = 0x20000900;
  *(uint64_t*)0x20000a98 = 0;
  *(uint64_t*)0x20000aa0 = 0x20000a00;
  *(uint64_t*)0x20000aa8 = 0;
  *(uint64_t*)0x20000ab0 = 0x20000a40;
  *(uint64_t*)0x20000ab8 = 0;
  *(uint64_t*)0x20000b50 = 4;
  *(uint64_t*)0x20000b58 = 0x20000ac0;
  *(uint64_t*)0x20000ac0 = 0x20;
  *(uint32_t*)0x20000ac8 = 0x84;
  *(uint32_t*)0x20000acc = 8;
  *(uint8_t*)0x20000ad0 = 0xfe;
  *(uint8_t*)0x20000ad1 = 0x80;
  *(uint8_t*)0x20000ad2 = 0;
  *(uint8_t*)0x20000ad3 = 0;
  *(uint8_t*)0x20000ad4 = 0;
  *(uint8_t*)0x20000ad5 = 0;
  *(uint8_t*)0x20000ad6 = 0;
  *(uint8_t*)0x20000ad7 = 0;
  *(uint8_t*)0x20000ad8 = 0;
  *(uint8_t*)0x20000ad9 = 0;
  *(uint8_t*)0x20000ada = 0;
  *(uint8_t*)0x20000adb = 0;
  *(uint8_t*)0x20000adc = 0;
  *(uint8_t*)0x20000add = 0;
  *(uint8_t*)0x20000ade = 0;
  *(uint8_t*)0x20000adf = 0x1b;
  *(uint64_t*)0x20000b60 = 0x20;
  *(uint32_t*)0x20000b68 = 0;
  syscall(__NR_sendmmsg, -1, 0x20000b00, 2, 0x8001);
  *(uint32_t*)0x200002c0 = 2;
  *(uint32_t*)0x200002c4 = 0x70;
  *(uint8_t*)0x200002c8 = 0xe6;
  *(uint8_t*)0x200002c9 = 0;
  *(uint8_t*)0x200002ca = 0;
  *(uint8_t*)0x200002cb = 0;
  *(uint32_t*)0x200002cc = 0;
  *(uint64_t*)0x200002d0 = 0;
  *(uint64_t*)0x200002d8 = 0;
  *(uint64_t*)0x200002e0 = 0;
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 1, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 2, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 3, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 4, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0xffff7ffffffffffb, 5, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 6, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 7, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 8, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 9, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 10, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 11, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 13, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 14, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 15, 2);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 17, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 18, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 19, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 20, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 21, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 22, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 23, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 24, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 25, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 26, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 27, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 28, 1);
  STORE_BY_BITMASK(uint64_t, 0x200002e8, 0, 29, 35);
  *(uint32_t*)0x200002f0 = 0;
  *(uint32_t*)0x200002f4 = 0;
  *(uint64_t*)0x200002f8 = 0;
  *(uint64_t*)0x20000300 = 0;
  *(uint64_t*)0x20000308 = 0;
  *(uint64_t*)0x20000310 = 0;
  *(uint32_t*)0x20000318 = 0;
  *(uint32_t*)0x2000031c = 0;
  *(uint64_t*)0x20000320 = 0;
  *(uint32_t*)0x20000328 = 0;
  *(uint16_t*)0x2000032c = 0;
  *(uint16_t*)0x2000032e = 0;
  syscall(__NR_perf_event_open, 0x200002c0, 0, 0, -1, 0);
  memcpy((void*)0x20000100, "/dev/binder#", 13);
  res = syz_open_dev(0x20000100, 0, 0);
  if (res != -1)
    r[3] = res;
  syscall(__NR_mmap, 0x20001000, 0x3000, 0, 0x20011, r[3], 0);
  syscall(__NR_ioctl, r[3], 0x40046207, 0);
  syscall(__NR_ioctl, -1, 0x8912, 0x20000040);
  memcpy((void*)0x20000100,
         "\x76\x63\x61\x6e\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint32_t*)0x20000110 = 0;
  res = syscall(__NR_ioctl, -1, 0x8933, 0x20000100);
  if (res != -1)
    r[4] = *(uint32_t*)0x20000110;
  *(uint64_t*)0x200002c0 = 0x200001c0;
  *(uint16_t*)0x200001c0 = 0x10;
  *(uint16_t*)0x200001c2 = 0;
  *(uint32_t*)0x200001c4 = 0;
  *(uint32_t*)0x200001c8 = 0;
  *(uint32_t*)0x200002c8 = 0xc;
  *(uint64_t*)0x200002d0 = 0x20000180;
  *(uint64_t*)0x20000180 = 0x20000240;
  *(uint32_t*)0x20000240 = 0x34;
  *(uint16_t*)0x20000244 = 0x14;
  *(uint16_t*)0x20000246 = 0x11d;
  *(uint32_t*)0x20000248 = 0;
  *(uint32_t*)0x2000024c = 0;
  *(uint8_t*)0x20000250 = 2;
  *(uint8_t*)0x20000251 = 0;
  *(uint8_t*)0x20000252 = 0;
  *(uint8_t*)0x20000253 = 0;
  *(uint32_t*)0x20000254 = r[4];
  *(uint16_t*)0x20000258 = 8;
  *(uint16_t*)0x2000025a = 2;
  *(uint8_t*)0x2000025c = 0xac;
  *(uint8_t*)0x2000025d = 0x14;
  *(uint8_t*)0x2000025e = 0x14;
  *(uint8_t*)0x2000025f = 0;
  *(uint16_t*)0x20000260 = 0x14;
  *(uint16_t*)0x20000262 = 6;
  *(uint32_t*)0x20000264 = 0;
  *(uint32_t*)0x20000268 = 7;
  *(uint32_t*)0x2000026c = 0;
  *(uint32_t*)0x20000270 = 0;
  *(uint64_t*)0x20000188 = 0x34;
  *(uint64_t*)0x200002d8 = 1;
  *(uint64_t*)0x200002e0 = 0;
  *(uint64_t*)0x200002e8 = 0;
  *(uint32_t*)0x200002f0 = 0;
  syscall(__NR_sendmsg, -1, 0x200002c0, 0);
  syscall(__NR_fcntl, -1, 8, -1);
  *(uint64_t*)0x20003380 = 0;
  *(uint32_t*)0x20003388 = 0;
  *(uint64_t*)0x20003390 = 0x20000200;
  *(uint64_t*)0x20000200 = 0x200000c0;
  *(uint64_t*)0x20000208 = 0;
  *(uint64_t*)0x20000210 = 0x20000100;
  *(uint64_t*)0x20000218 = 0;
  *(uint64_t*)0x20000220 = 0x20000280;
  *(uint64_t*)0x20000228 = 0;
  *(uint64_t*)0x20000230 = 0x20000300;
  *(uint64_t*)0x20000238 = 0;
  *(uint64_t*)0x20003398 = 4;
  *(uint64_t*)0x200033a0 = 0x20000380;
  *(uint64_t*)0x20000380 = 0x18;
  *(uint32_t*)0x20000388 = 0x117;
  *(uint32_t*)0x2000038c = 2;
  *(uint32_t*)0x20000390 = 0;
  *(uint64_t*)0x20000398 = 0x18;
  *(uint32_t*)0x200003a0 = 0x117;
  *(uint32_t*)0x200003a4 = 3;
  *(uint32_t*)0x200003a8 = 1;
  *(uint64_t*)0x200003b0 = 0x18;
  *(uint32_t*)0x200003b8 = 0x117;
  *(uint32_t*)0x200003bc = 4;
  *(uint32_t*)0x200003c0 = 4;
  *(uint64_t*)0x200033a8 = 0x48;
  *(uint32_t*)0x200033b0 = 0;
  *(uint64_t*)0x200033b8 = 0;
  *(uint32_t*)0x200033c0 = 0;
  *(uint64_t*)0x200033c8 = 0x200007c0;
  *(uint64_t*)0x200007c0 = 0x20000440;
  *(uint64_t*)0x200007c8 = 0;
  *(uint64_t*)0x200007d0 = 0x20000480;
  *(uint64_t*)0x200007d8 = 0;
  *(uint64_t*)0x200007e0 = 0x20000580;
  *(uint64_t*)0x200007e8 = 0;
  *(uint64_t*)0x200007f0 = 0x20000640;
  *(uint64_t*)0x200007f8 = 0;
  *(uint64_t*)0x20000800 = 0x20000680;
  *(uint64_t*)0x20000808 = 0;
  *(uint64_t*)0x20000810 = 0x20000700;
  *(uint64_t*)0x20000818 = 0;
  *(uint64_t*)0x200033d0 = 6;
  *(uint64_t*)0x200033d8 = 0x20000840;
  *(uint64_t*)0x20000840 = 0x18;
  *(uint32_t*)0x20000848 = 0x117;
  *(uint32_t*)0x2000084c = 3;
  *(uint32_t*)0x20000850 = 1;
  *(uint64_t*)0x20000858 = 0x18;
  *(uint32_t*)0x20000860 = 0x117;
  *(uint32_t*)0x20000864 = 2;
  *(uint32_t*)0x20000868 = 0;
  *(uint64_t*)0x20000870 = 0x18;
  *(uint32_t*)0x20000878 = 0x117;
  *(uint32_t*)0x2000087c = 4;
  *(uint32_t*)0x20000880 = 1;
  *(uint64_t*)0x20000888 = 0x18;
  *(uint32_t*)0x20000890 = 0x117;
  *(uint32_t*)0x20000894 = 3;
  *(uint32_t*)0x20000898 = 1;
  *(uint64_t*)0x200008a0 = 0x18;
  *(uint32_t*)0x200008a8 = 0x117;
  *(uint32_t*)0x200008ac = 2;
  *(uint32_t*)0x200008b0 = 0;
  *(uint64_t*)0x200008b8 = 0x18;
  *(uint32_t*)0x200008c0 = 0x117;
  *(uint32_t*)0x200008c4 = 4;
  *(uint32_t*)0x200008c8 = 9;
  *(uint64_t*)0x200008d0 = 0x18;
  *(uint32_t*)0x200008d8 = 0x117;
  *(uint32_t*)0x200008dc = 2;
  *(uint32_t*)0x200008e0 = 0;
  *(uint64_t*)0x200033e0 = 0xa8;
  *(uint32_t*)0x200033e8 = 0;
  *(uint64_t*)0x200033f0 = 0;
  *(uint32_t*)0x200033f8 = 0;
  *(uint64_t*)0x20003400 = 0x20000cc0;
  *(uint64_t*)0x20000cc0 = 0x20000a40;
  *(uint64_t*)0x20000cc8 = 0;
  *(uint64_t*)0x20000cd0 = 0x20000a80;
  *(uint64_t*)0x20000cd8 = 0;
  *(uint64_t*)0x20000ce0 = 0x20000b80;
  *(uint64_t*)0x20000ce8 = 0;
  *(uint64_t*)0x20000cf0 = 0x20000bc0;
  *(uint64_t*)0x20000cf8 = 0;
  *(uint64_t*)0x20003408 = 4;
  *(uint64_t*)0x20003410 = 0x20000d00;
  *(uint64_t*)0x20000d00 = 0x18;
  *(uint32_t*)0x20000d08 = 0x117;
  *(uint32_t*)0x20000d0c = 4;
  *(uint32_t*)0x20000d10 = 0x1000;
  *(uint64_t*)0x20000d18 = 0x18;
  *(uint32_t*)0x20000d20 = 0x117;
  *(uint32_t*)0x20000d24 = 3;
  *(uint32_t*)0x20000d28 = 0;
  *(uint64_t*)0x20000d30 = 0x18;
  *(uint32_t*)0x20000d38 = 0x117;
  *(uint32_t*)0x20000d3c = 3;
  *(uint32_t*)0x20000d40 = 1;
  *(uint64_t*)0x20000d48 = 0x18;
  *(uint32_t*)0x20000d50 = 0x117;
  *(uint32_t*)0x20000d54 = 4;
  *(uint32_t*)0x20000d58 = 9;
  *(uint64_t*)0x20000d60 = 0x18;
  *(uint32_t*)0x20000d68 = 0x117;
  *(uint32_t*)0x20000d6c = 3;
  *(uint32_t*)0x20000d70 = 1;
  *(uint64_t*)0x20000d78 = 0x18;
  *(uint32_t*)0x20000d80 = 0x117;
  *(uint32_t*)0x20000d84 = 4;
  *(uint32_t*)0x20000d88 = 0x7ff;
  *(uint64_t*)0x20000d90 = 0x18;
  *(uint32_t*)0x20000d98 = 0x117;
  *(uint32_t*)0x20000d9c = 2;
  *(uint32_t*)0x20000da0 = 0;
  *(uint64_t*)0x20000da8 = 0x18;
  *(uint32_t*)0x20000db0 = 0x117;
  *(uint32_t*)0x20000db4 = 2;
  *(uint32_t*)0x20000db8 = 0;
  *(uint64_t*)0x20000dc0 = 0x18;
  *(uint32_t*)0x20000dc8 = 0x117;
  *(uint32_t*)0x20000dcc = 3;
  *(uint32_t*)0x20000dd0 = 1;
  *(uint64_t*)0x20000dd8 = 0x18;
  *(uint32_t*)0x20000de0 = 0x117;
  *(uint32_t*)0x20000de4 = 4;
  *(uint32_t*)0x20000de8 = 0xfff;
  *(uint64_t*)0x20003418 = 0xf0;
  *(uint32_t*)0x20003420 = 0x800;
  *(uint64_t*)0x20003428 = 0;
  *(uint32_t*)0x20003430 = 0;
  *(uint64_t*)0x20003438 = 0x20002080;
  *(uint64_t*)0x20002080 = 0x20001e40;
  *(uint64_t*)0x20002088 = 0;
  *(uint64_t*)0x20002090 = 0x20001ec0;
  *(uint64_t*)0x20002098 = 0;
  *(uint64_t*)0x200020a0 = 0x20001f80;
  *(uint64_t*)0x200020a8 = 0;
  *(uint64_t*)0x200020b0 = 0x20001fc0;
  *(uint64_t*)0x200020b8 = 0;
  *(uint64_t*)0x200020c0 = 0x20002040;
  *(uint64_t*)0x200020c8 = 0;
  *(uint64_t*)0x20003440 = 5;
  *(uint64_t*)0x20003448 = 0x20002100;
  *(uint64_t*)0x20002100 = 0x18;
  *(uint32_t*)0x20002108 = 0x117;
  *(uint32_t*)0x2000210c = 2;
  *(uint32_t*)0x20002110 = 0;
  *(uint64_t*)0x20002118 = 0x18;
  *(uint32_t*)0x20002120 = 0x117;
  *(uint32_t*)0x20002124 = 2;
  *(uint32_t*)0x20002128 = 0;
  *(uint64_t*)0x20002130 = 0x18;
  *(uint32_t*)0x20002138 = 0x117;
  *(uint32_t*)0x2000213c = 4;
  *(uint32_t*)0x20002140 = 6;
  *(uint64_t*)0x20002148 = 0x18;
  *(uint32_t*)0x20002150 = 0x117;
  *(uint32_t*)0x20002154 = 4;
  *(uint32_t*)0x20002158 = 9;
  *(uint64_t*)0x20002160 = 0x18;
  *(uint32_t*)0x20002168 = 0x117;
  *(uint32_t*)0x2000216c = 4;
  *(uint32_t*)0x20002170 = 6;
  *(uint64_t*)0x20002178 = 0x18;
  *(uint32_t*)0x20002180 = 0x117;
  *(uint32_t*)0x20002184 = 4;
  *(uint32_t*)0x20002188 = 8;
  *(uint64_t*)0x20002190 = 0x18;
  *(uint32_t*)0x20002198 = 0x117;
  *(uint32_t*)0x2000219c = 2;
  *(uint32_t*)0x200021a0 = 0;
  *(uint64_t*)0x200021a8 = 0x18;
  *(uint32_t*)0x200021b0 = 0x117;
  *(uint32_t*)0x200021b4 = 3;
  *(uint32_t*)0x200021b8 = 1;
  *(uint64_t*)0x200021c0 = 0x18;
  *(uint32_t*)0x200021c8 = 0x117;
  *(uint32_t*)0x200021cc = 2;
  *(uint32_t*)0x200021d0 = 0;
  *(uint64_t*)0x200021d8 = 0x18;
  *(uint32_t*)0x200021e0 = 0x117;
  *(uint32_t*)0x200021e4 = 3;
  *(uint32_t*)0x200021e8 = 1;
  *(uint64_t*)0x20003450 = 0xf0;
  *(uint32_t*)0x20003458 = 0x20000000;
  syscall(__NR_sendmmsg, -1, 0x20003380, 4, 0x20008000);
  *(uint16_t*)0x20001140 = 2;
  *(uint16_t*)0x20001142 = htobe16(0);
  *(uint32_t*)0x20001144 = htobe32(0);
  *(uint8_t*)0x20001148 = 0;
  *(uint8_t*)0x20001149 = 0;
  *(uint8_t*)0x2000114a = 0;
  *(uint8_t*)0x2000114b = 0;
  *(uint8_t*)0x2000114c = 0;
  *(uint8_t*)0x2000114d = 0;
  *(uint8_t*)0x2000114e = 0;
  *(uint8_t*)0x2000114f = 0;
  *(uint16_t*)0x20001150 = 0;
  *(uint8_t*)0x20001152 = 0xaa;
  *(uint8_t*)0x20001153 = 0xaa;
  *(uint8_t*)0x20001154 = 0xaa;
  *(uint8_t*)0x20001155 = 0xaa;
  *(uint8_t*)0x20001156 = 0xaa;
  *(uint8_t*)0x20001157 = 0;
  *(uint32_t*)0x20001160 = 0;
  *(uint16_t*)0x20001164 = 2;
  *(uint16_t*)0x20001166 = htobe16(0);
  *(uint32_t*)0x20001168 = htobe32(0);
  *(uint8_t*)0x2000116c = 0;
  *(uint8_t*)0x2000116d = 0;
  *(uint8_t*)0x2000116e = 0;
  *(uint8_t*)0x2000116f = 0;
  *(uint8_t*)0x20001170 = 0;
  *(uint8_t*)0x20001171 = 0;
  *(uint8_t*)0x20001172 = 0;
  *(uint8_t*)0x20001173 = 0;
  memcpy((void*)0x20001174,
         "\x74\x75\x6e\x6c\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  syscall(__NR_ioctl, -1, 0x8953, 0x20001140);
  syscall(__NR_getuid);
  syscall(__NR_ioctl, -1, 0x40042408, -1);
  *(uint32_t*)0x200ba000 = 1;
  *(uint32_t*)0x200ba004 = 0;
  *(uint64_t*)0x200ba008 = 0x20000200;
  *(uint64_t*)0x200ba010 = 0x20f6bffb;
  memcpy((void*)0x20f6bffb, "GPL", 4);
  *(uint32_t*)0x200ba018 = 0xfffffffc;
  *(uint32_t*)0x200ba01c = 0xfb;
  *(uint64_t*)0x200ba020 = 0x201a7f05;
  *(uint32_t*)0x200ba028 = 0;
  *(uint32_t*)0x200ba02c = 0;
  *(uint8_t*)0x200ba030 = 0;
  *(uint8_t*)0x200ba031 = 0;
  *(uint8_t*)0x200ba032 = 0;
  *(uint8_t*)0x200ba033 = 0;
  *(uint8_t*)0x200ba034 = 0;
  *(uint8_t*)0x200ba035 = 0;
  *(uint8_t*)0x200ba036 = 0;
  *(uint8_t*)0x200ba037 = 0;
  *(uint8_t*)0x200ba038 = 0;
  *(uint8_t*)0x200ba039 = 0;
  *(uint8_t*)0x200ba03a = 0;
  *(uint8_t*)0x200ba03b = 0;
  *(uint8_t*)0x200ba03c = 0;
  *(uint8_t*)0x200ba03d = 0;
  *(uint8_t*)0x200ba03e = 0;
  *(uint8_t*)0x200ba03f = 0;
  *(uint32_t*)0x200ba040 = 0;
  *(uint32_t*)0x200ba044 = 0;
  syscall(__NR_bpf, 5, 0x200ba000, 0x48);
  memcpy((void*)0x20000100, "pids.events", 12);
  syscall(__NR_openat, -1, 0x20000100, 0, 0);
  *(uint32_t*)0x20000640 = 0;
  *(uint32_t*)0x20000644 = 0;
  *(uint32_t*)0x200005c0 = 8;
  syscall(__NR_getsockopt, -1, 0x84, 0x1b, 0x20000640, 0x200005c0);
  syscall(__NR_setsockopt, -1, 0x107, 0xf, 0x200004c0, 0);
  *(uint16_t*)0x20001140 = 2;
  *(uint16_t*)0x20001142 = htobe16(0);
  *(uint32_t*)0x20001144 = htobe32(0);
  *(uint8_t*)0x20001148 = 0;
  *(uint8_t*)0x20001149 = 0;
  *(uint8_t*)0x2000114a = 0;
  *(uint8_t*)0x2000114b = 0;
  *(uint8_t*)0x2000114c = 0;
  *(uint8_t*)0x2000114d = 0;
  *(uint8_t*)0x2000114e = 0;
  *(uint8_t*)0x2000114f = 0;
  *(uint16_t*)0x20001150 = 0;
  *(uint8_t*)0x20001152 = 0xaa;
  *(uint8_t*)0x20001153 = 0xaa;
  *(uint8_t*)0x20001154 = 0xaa;
  *(uint8_t*)0x20001155 = 0xaa;
  *(uint8_t*)0x20001156 = 0xaa;
  *(uint8_t*)0x20001157 = 0;
  *(uint32_t*)0x20001160 = 0;
  *(uint16_t*)0x20001164 = 2;
  *(uint16_t*)0x20001166 = htobe16(0);
  *(uint32_t*)0x20001168 = htobe32(0);
  *(uint8_t*)0x2000116c = 0;
  *(uint8_t*)0x2000116d = 0;
  *(uint8_t*)0x2000116e = 0;
  *(uint8_t*)0x2000116f = 0;
  *(uint8_t*)0x20001170 = 0;
  *(uint8_t*)0x20001171 = 0;
  *(uint8_t*)0x20001172 = 0;
  *(uint8_t*)0x20001173 = 0;
  memcpy((void*)0x20001174,
         "\x74\x75\x6e\x6c\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  syscall(__NR_ioctl, -1, 0x8953, 0x20001140);
  syscall(__NR_getuid);
  memcpy((void*)0x20000000, "/dev/vcsa#", 11);
  syz_open_dev(0x20000000, 6, 0);
  *(uint8_t*)0x200000c0 = 0;
  *(uint8_t*)0x200000c1 = 0;
  *(uint8_t*)0x200000c2 = 0;
  *(uint8_t*)0x200000c3 = 0;
  *(uint8_t*)0x200000c4 = 0;
  *(uint8_t*)0x200000c5 = 0;
  *(uint8_t*)0x200000c6 = 0;
  *(uint8_t*)0x200000c7 = 0;
  *(uint8_t*)0x200000c8 = 0;
  *(uint8_t*)0x200000c9 = 0;
  *(uint8_t*)0x200000ca = 0;
  *(uint8_t*)0x200000cb = 0;
  *(uint8_t*)0x200000cc = 0;
  *(uint8_t*)0x200000cd = 0;
  *(uint8_t*)0x200000ce = 0;
  *(uint8_t*)0x200000cf = 0;
  *(uint32_t*)0x200000d0 = 0;
  syscall(__NR_setsockopt, -1, 0x29, 0x1d, 0x200000c0, 0x14);
  return 0;
}
