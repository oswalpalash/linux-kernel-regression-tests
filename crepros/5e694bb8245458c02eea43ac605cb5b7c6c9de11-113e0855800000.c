// https://syzkaller.appspot.com/bug?id=5e694bb8245458c02eea43ac605cb5b7c6c9de11
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[1];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0x18000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 2, 0x80005, 0);
  memcpy((void*)0x20012000, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20012020 = 0xe;
  *(uint32_t*)0x20012024 = 4;
  *(uint32_t*)0x20012028 = 0x358;
  *(uint32_t*)0x2001202c = -1;
  *(uint32_t*)0x20012030 = 0;
  *(uint32_t*)0x20012034 = 0;
  *(uint32_t*)0x20012038 = 0;
  *(uint32_t*)0x2001203c = -1;
  *(uint32_t*)0x20012040 = -1;
  *(uint32_t*)0x20012044 = 0;
  *(uint32_t*)0x20012048 = 0;
  *(uint32_t*)0x2001204c = 0;
  *(uint32_t*)0x20012050 = -1;
  *(uint32_t*)0x20012054 = 4;
  *(uint64_t*)0x20012058 = 0x20001000;
  *(uint8_t*)0x20012060 = 0;
  *(uint8_t*)0x20012061 = 0;
  *(uint8_t*)0x20012062 = 0;
  *(uint8_t*)0x20012063 = 0;
  *(uint8_t*)0x20012064 = 0;
  *(uint8_t*)0x20012065 = 0;
  *(uint8_t*)0x20012066 = 0;
  *(uint8_t*)0x20012067 = 0;
  *(uint8_t*)0x20012068 = 0;
  *(uint8_t*)0x20012069 = 0;
  *(uint8_t*)0x2001206a = 0;
  *(uint8_t*)0x2001206b = 0;
  *(uint8_t*)0x2001206c = 0;
  *(uint8_t*)0x2001206d = 0;
  *(uint8_t*)0x2001206e = 0;
  *(uint8_t*)0x2001206f = 0;
  *(uint8_t*)0x20012070 = 0;
  *(uint8_t*)0x20012071 = 0;
  *(uint8_t*)0x20012072 = 0;
  *(uint8_t*)0x20012073 = 0;
  *(uint8_t*)0x20012074 = 0;
  *(uint8_t*)0x20012075 = 0;
  *(uint8_t*)0x20012076 = 0;
  *(uint8_t*)0x20012077 = 0;
  *(uint8_t*)0x20012078 = 0;
  *(uint8_t*)0x20012079 = 0;
  *(uint8_t*)0x2001207a = 0;
  *(uint8_t*)0x2001207b = 0;
  *(uint8_t*)0x2001207c = 0;
  *(uint8_t*)0x2001207d = 0;
  *(uint8_t*)0x2001207e = 0;
  *(uint8_t*)0x2001207f = 0;
  *(uint8_t*)0x20012080 = 0;
  *(uint8_t*)0x20012081 = 0;
  *(uint8_t*)0x20012082 = 0;
  *(uint8_t*)0x20012083 = 0;
  *(uint8_t*)0x20012084 = 0;
  *(uint8_t*)0x20012085 = 0;
  *(uint8_t*)0x20012086 = 0;
  *(uint8_t*)0x20012087 = 0;
  *(uint8_t*)0x20012088 = 0;
  *(uint8_t*)0x20012089 = 0;
  *(uint8_t*)0x2001208a = 0;
  *(uint8_t*)0x2001208b = 0;
  *(uint8_t*)0x2001208c = 0;
  *(uint8_t*)0x2001208d = 0;
  *(uint8_t*)0x2001208e = 0;
  *(uint8_t*)0x2001208f = 0;
  *(uint8_t*)0x20012090 = 0;
  *(uint8_t*)0x20012091 = 0;
  *(uint8_t*)0x20012092 = 0;
  *(uint8_t*)0x20012093 = 0;
  *(uint8_t*)0x20012094 = 0;
  *(uint8_t*)0x20012095 = 0;
  *(uint8_t*)0x20012096 = 0;
  *(uint8_t*)0x20012097 = 0;
  *(uint8_t*)0x20012098 = 0;
  *(uint8_t*)0x20012099 = 0;
  *(uint8_t*)0x2001209a = 0;
  *(uint8_t*)0x2001209b = 0;
  *(uint8_t*)0x2001209c = 0;
  *(uint8_t*)0x2001209d = 0;
  *(uint8_t*)0x2001209e = 0;
  *(uint8_t*)0x2001209f = 0;
  *(uint8_t*)0x200120a0 = 0;
  *(uint8_t*)0x200120a1 = 0;
  *(uint8_t*)0x200120a2 = 0;
  *(uint8_t*)0x200120a3 = 0;
  *(uint8_t*)0x200120a4 = 0;
  *(uint8_t*)0x200120a5 = 0;
  *(uint8_t*)0x200120a6 = 0;
  *(uint8_t*)0x200120a7 = 0;
  *(uint8_t*)0x200120a8 = 0;
  *(uint8_t*)0x200120a9 = 0;
  *(uint8_t*)0x200120aa = 0;
  *(uint8_t*)0x200120ab = 0;
  *(uint8_t*)0x200120ac = 0;
  *(uint8_t*)0x200120ad = 0;
  *(uint8_t*)0x200120ae = 0;
  *(uint8_t*)0x200120af = 0;
  *(uint8_t*)0x200120b0 = 0;
  *(uint8_t*)0x200120b1 = 0;
  *(uint8_t*)0x200120b2 = 0;
  *(uint8_t*)0x200120b3 = 0;
  *(uint32_t*)0x200120b4 = 0;
  *(uint16_t*)0x200120b8 = 0x70;
  *(uint16_t*)0x200120ba = 0x98;
  *(uint32_t*)0x200120bc = 0;
  *(uint64_t*)0x200120c0 = 0;
  *(uint64_t*)0x200120c8 = 0;
  *(uint16_t*)0x200120d0 = 0x28;
  memcpy((void*)0x200120d2, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x200120ef = 0;
  *(uint32_t*)0x200120f0 = 0xfffffffe;
  *(uint32_t*)0x200120f8 = htobe32(-1);
  *(uint8_t*)0x200120fc = 0xac;
  *(uint8_t*)0x200120fd = 0x14;
  *(uint8_t*)0x200120fe = 0;
  *(uint8_t*)0x200120ff = 0xbb;
  *(uint32_t*)0x20012100 = htobe32(0);
  *(uint32_t*)0x20012104 = htobe32(0);
  memcpy((void*)0x20012108,
         "\x62\x63\x73\x68\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x20012118 = 0x73;
  *(uint8_t*)0x20012119 = 0x79;
  *(uint8_t*)0x2001211a = 0x7a;
  *(uint8_t*)0x2001211b = 0;
  *(uint8_t*)0x2001211c = 0;
  *(uint8_t*)0x20012128 = 0;
  *(uint8_t*)0x20012129 = 0;
  *(uint8_t*)0x2001212a = 0;
  *(uint8_t*)0x2001212b = 0;
  *(uint8_t*)0x2001212c = 0;
  *(uint8_t*)0x2001212d = 0;
  *(uint8_t*)0x2001212e = 0;
  *(uint8_t*)0x2001212f = 0;
  *(uint8_t*)0x20012130 = 0;
  *(uint8_t*)0x20012131 = 0;
  *(uint8_t*)0x20012132 = 0;
  *(uint8_t*)0x20012133 = 0;
  *(uint8_t*)0x20012134 = 0;
  *(uint8_t*)0x20012135 = 0;
  *(uint8_t*)0x20012136 = 0;
  *(uint8_t*)0x20012137 = 0;
  *(uint8_t*)0x20012138 = 0;
  *(uint8_t*)0x20012139 = 0;
  *(uint8_t*)0x2001213a = 0;
  *(uint8_t*)0x2001213b = 0;
  *(uint8_t*)0x2001213c = 0;
  *(uint8_t*)0x2001213d = 0;
  *(uint8_t*)0x2001213e = 0;
  *(uint8_t*)0x2001213f = 0;
  *(uint8_t*)0x20012140 = 0;
  *(uint8_t*)0x20012141 = 0;
  *(uint8_t*)0x20012142 = 0;
  *(uint8_t*)0x20012143 = 0;
  *(uint8_t*)0x20012144 = 0;
  *(uint8_t*)0x20012145 = 0;
  *(uint8_t*)0x20012146 = 0;
  *(uint8_t*)0x20012147 = 0;
  *(uint16_t*)0x20012148 = 0;
  *(uint8_t*)0x2001214a = 0;
  *(uint8_t*)0x2001214b = 0;
  *(uint32_t*)0x2001214c = 0;
  *(uint16_t*)0x20012150 = 0x70;
  *(uint16_t*)0x20012152 = 0x98;
  *(uint32_t*)0x20012154 = 0;
  *(uint64_t*)0x20012158 = 0;
  *(uint64_t*)0x20012160 = 0;
  *(uint16_t*)0x20012168 = 0x28;
  memcpy((void*)0x2001216a, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20012187 = 0;
  *(uint32_t*)0x20012188 = 0;
  *(uint8_t*)0x20012190 = 0;
  *(uint8_t*)0x20012191 = 0;
  *(uint8_t*)0x20012192 = 0;
  *(uint8_t*)0x20012193 = 0;
  *(uint8_t*)0x20012194 = 0;
  *(uint8_t*)0x20012195 = 0;
  *(uint8_t*)0x20012196 = 0;
  *(uint8_t*)0x20012197 = 0;
  *(uint8_t*)0x20012198 = 0;
  *(uint8_t*)0x20012199 = 0;
  *(uint8_t*)0x2001219a = 0;
  *(uint8_t*)0x2001219b = 0;
  *(uint8_t*)0x2001219c = 0;
  *(uint8_t*)0x2001219d = 0;
  *(uint8_t*)0x2001219e = 0;
  *(uint8_t*)0x2001219f = 0;
  *(uint8_t*)0x200121a0 = 0;
  *(uint8_t*)0x200121a1 = 0;
  *(uint8_t*)0x200121a2 = 0;
  *(uint8_t*)0x200121a3 = 0;
  *(uint8_t*)0x200121a4 = 0;
  *(uint8_t*)0x200121a5 = 0;
  *(uint8_t*)0x200121a6 = 0;
  *(uint8_t*)0x200121a7 = 0;
  *(uint8_t*)0x200121a8 = 0;
  *(uint8_t*)0x200121a9 = 0;
  *(uint8_t*)0x200121aa = 0;
  *(uint8_t*)0x200121ab = 0;
  *(uint8_t*)0x200121ac = 0;
  *(uint8_t*)0x200121ad = 0;
  *(uint8_t*)0x200121ae = 0;
  *(uint8_t*)0x200121af = 0;
  *(uint8_t*)0x200121b0 = 0;
  *(uint8_t*)0x200121b1 = 0;
  *(uint8_t*)0x200121b2 = 0;
  *(uint8_t*)0x200121b3 = 0;
  *(uint8_t*)0x200121b4 = 0;
  *(uint8_t*)0x200121b5 = 0;
  *(uint8_t*)0x200121b6 = 0;
  *(uint8_t*)0x200121b7 = 0;
  *(uint8_t*)0x200121b8 = 0;
  *(uint8_t*)0x200121b9 = 0;
  *(uint8_t*)0x200121ba = 0;
  *(uint8_t*)0x200121bb = 0;
  *(uint8_t*)0x200121bc = 0;
  *(uint8_t*)0x200121bd = 0;
  *(uint8_t*)0x200121be = 0;
  *(uint8_t*)0x200121bf = 0;
  *(uint8_t*)0x200121c0 = 0;
  *(uint8_t*)0x200121c1 = 0;
  *(uint8_t*)0x200121c2 = 0;
  *(uint8_t*)0x200121c3 = 0;
  *(uint8_t*)0x200121c4 = 0;
  *(uint8_t*)0x200121c5 = 0;
  *(uint8_t*)0x200121c6 = 0;
  *(uint8_t*)0x200121c7 = 0;
  *(uint8_t*)0x200121c8 = 0;
  *(uint8_t*)0x200121c9 = 0;
  *(uint8_t*)0x200121ca = 0;
  *(uint8_t*)0x200121cb = 0;
  *(uint8_t*)0x200121cc = 0;
  *(uint8_t*)0x200121cd = 0;
  *(uint8_t*)0x200121ce = 0;
  *(uint8_t*)0x200121cf = 0;
  *(uint8_t*)0x200121d0 = 0;
  *(uint8_t*)0x200121d1 = 0;
  *(uint8_t*)0x200121d2 = 0;
  *(uint8_t*)0x200121d3 = 0;
  *(uint8_t*)0x200121d4 = 0;
  *(uint8_t*)0x200121d5 = 0;
  *(uint8_t*)0x200121d6 = 0;
  *(uint8_t*)0x200121d7 = 0;
  *(uint8_t*)0x200121d8 = 0;
  *(uint8_t*)0x200121d9 = 0;
  *(uint8_t*)0x200121da = 0;
  *(uint8_t*)0x200121db = 0;
  *(uint8_t*)0x200121dc = 0;
  *(uint8_t*)0x200121dd = 0;
  *(uint8_t*)0x200121de = 0;
  *(uint8_t*)0x200121df = 0;
  *(uint8_t*)0x200121e0 = 0;
  *(uint8_t*)0x200121e1 = 0;
  *(uint8_t*)0x200121e2 = 0;
  *(uint8_t*)0x200121e3 = 0;
  *(uint32_t*)0x200121e4 = 0;
  *(uint16_t*)0x200121e8 = 0x168;
  *(uint16_t*)0x200121ea = 0x190;
  *(uint32_t*)0x200121ec = 0;
  *(uint64_t*)0x200121f0 = 0;
  *(uint64_t*)0x200121f8 = 0;
  *(uint16_t*)0x20012200 = 0xf8;
  memcpy((void*)0x20012202, "\x72\x65\x63\x65\x6e\x74\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2001221f = 0;
  *(uint32_t*)0x20012220 = 0;
  *(uint32_t*)0x20012224 = 0;
  *(uint8_t*)0x20012228 = 4;
  *(uint8_t*)0x20012229 = 0;
  memcpy((void*)0x2001222a,
         "\x73\x79\x7a\x2f\xf5\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x82\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         200);
  *(uint8_t*)0x200122f2 = 0;
  *(uint16_t*)0x200122f8 = 0x28;
  memcpy((void*)0x200122fa, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20012317 = 0;
  *(uint32_t*)0x20012318 = 0;
  *(uint8_t*)0x20012320 = 0;
  *(uint8_t*)0x20012321 = 0;
  *(uint8_t*)0x20012322 = 0;
  *(uint8_t*)0x20012323 = 0;
  *(uint8_t*)0x20012324 = 0;
  *(uint8_t*)0x20012325 = 0;
  *(uint8_t*)0x20012326 = 0;
  *(uint8_t*)0x20012327 = 0;
  *(uint8_t*)0x20012328 = 0;
  *(uint8_t*)0x20012329 = 0;
  *(uint8_t*)0x2001232a = 0;
  *(uint8_t*)0x2001232b = 0;
  *(uint8_t*)0x2001232c = 0;
  *(uint8_t*)0x2001232d = 0;
  *(uint8_t*)0x2001232e = 0;
  *(uint8_t*)0x2001232f = 0;
  *(uint8_t*)0x20012330 = 0;
  *(uint8_t*)0x20012331 = 0;
  *(uint8_t*)0x20012332 = 0;
  *(uint8_t*)0x20012333 = 0;
  *(uint8_t*)0x20012334 = 0;
  *(uint8_t*)0x20012335 = 0;
  *(uint8_t*)0x20012336 = 0;
  *(uint8_t*)0x20012337 = 0;
  *(uint8_t*)0x20012338 = 0;
  *(uint8_t*)0x20012339 = 0;
  *(uint8_t*)0x2001233a = 0;
  *(uint8_t*)0x2001233b = 0;
  *(uint8_t*)0x2001233c = 0;
  *(uint8_t*)0x2001233d = 0;
  *(uint8_t*)0x2001233e = 0;
  *(uint8_t*)0x2001233f = 0;
  *(uint8_t*)0x20012340 = 0;
  *(uint8_t*)0x20012341 = 0;
  *(uint8_t*)0x20012342 = 0;
  *(uint8_t*)0x20012343 = 0;
  *(uint8_t*)0x20012344 = 0;
  *(uint8_t*)0x20012345 = 0;
  *(uint8_t*)0x20012346 = 0;
  *(uint8_t*)0x20012347 = 0;
  *(uint8_t*)0x20012348 = 0;
  *(uint8_t*)0x20012349 = 0;
  *(uint8_t*)0x2001234a = 0;
  *(uint8_t*)0x2001234b = 0;
  *(uint8_t*)0x2001234c = 0;
  *(uint8_t*)0x2001234d = 0;
  *(uint8_t*)0x2001234e = 0;
  *(uint8_t*)0x2001234f = 0;
  *(uint8_t*)0x20012350 = 0;
  *(uint8_t*)0x20012351 = 0;
  *(uint8_t*)0x20012352 = 0;
  *(uint8_t*)0x20012353 = 0;
  *(uint8_t*)0x20012354 = 0;
  *(uint8_t*)0x20012355 = 0;
  *(uint8_t*)0x20012356 = 0;
  *(uint8_t*)0x20012357 = 0;
  *(uint8_t*)0x20012358 = 0;
  *(uint8_t*)0x20012359 = 0;
  *(uint8_t*)0x2001235a = 0;
  *(uint8_t*)0x2001235b = 0;
  *(uint8_t*)0x2001235c = 0;
  *(uint8_t*)0x2001235d = 0;
  *(uint8_t*)0x2001235e = 0;
  *(uint8_t*)0x2001235f = 0;
  *(uint8_t*)0x20012360 = 0;
  *(uint8_t*)0x20012361 = 0;
  *(uint8_t*)0x20012362 = 0;
  *(uint8_t*)0x20012363 = 0;
  *(uint8_t*)0x20012364 = 0;
  *(uint8_t*)0x20012365 = 0;
  *(uint8_t*)0x20012366 = 0;
  *(uint8_t*)0x20012367 = 0;
  *(uint8_t*)0x20012368 = 0;
  *(uint8_t*)0x20012369 = 0;
  *(uint8_t*)0x2001236a = 0;
  *(uint8_t*)0x2001236b = 0;
  *(uint8_t*)0x2001236c = 0;
  *(uint8_t*)0x2001236d = 0;
  *(uint8_t*)0x2001236e = 0;
  *(uint8_t*)0x2001236f = 0;
  *(uint8_t*)0x20012370 = 0;
  *(uint8_t*)0x20012371 = 0;
  *(uint8_t*)0x20012372 = 0;
  *(uint8_t*)0x20012373 = 0;
  *(uint32_t*)0x20012374 = 0;
  *(uint16_t*)0x20012378 = 0x70;
  *(uint16_t*)0x2001237a = 0x98;
  *(uint32_t*)0x2001237c = 0;
  *(uint64_t*)0x20012380 = 0;
  *(uint64_t*)0x20012388 = 0;
  *(uint16_t*)0x20012390 = 0x28;
  memcpy((void*)0x20012392, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x200123af = 0;
  *(uint32_t*)0x200123b0 = 0;
  *(uint64_t*)0x20001000 = 0;
  *(uint64_t*)0x20001008 = 0;
  *(uint64_t*)0x20001010 = 0;
  *(uint64_t*)0x20001018 = 0;
  *(uint64_t*)0x20001020 = 0;
  *(uint64_t*)0x20001028 = 0;
  *(uint64_t*)0x20001030 = 0;
  *(uint64_t*)0x20001038 = 0;
  syscall(__NR_setsockopt, r[0], 0, 0x40, 0x20012000, 0x3b8);
}

int main()
{
  loop();
  return 0;
}
