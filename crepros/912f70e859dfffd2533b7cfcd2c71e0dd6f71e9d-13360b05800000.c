// https://syzkaller.appspot.com/bug?id=912f70e859dfffd2533b7cfcd2c71e0dd6f71e9d
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

long r[2];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0xf82000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 2, 1, 0);
  *(uint32_t*)0x20000000 = htobe32(0xe0000002);
  *(uint32_t*)0x20000004 = htobe32(0x7f000001);
  *(uint32_t*)0x20000008 = 0;
  syscall(__NR_setsockopt, r[0], 0, 0x27, 0x20000000, 0xc);
  r[1] = syscall(__NR_socket, 2, 0x80005, 0);
  memcpy((void*)0x20019000, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20019020 = 0xe;
  *(uint32_t*)0x20019024 = 4;
  *(uint32_t*)0x20019028 = 0x2a8;
  *(uint32_t*)0x2001902c = -1;
  *(uint32_t*)0x20019030 = 0;
  *(uint32_t*)0x20019034 = 0;
  *(uint32_t*)0x20019038 = 0;
  *(uint32_t*)0x2001903c = -1;
  *(uint32_t*)0x20019040 = -1;
  *(uint32_t*)0x20019044 = 0;
  *(uint32_t*)0x20019048 = 0;
  *(uint32_t*)0x2001904c = 0;
  *(uint32_t*)0x20019050 = -1;
  *(uint32_t*)0x20019054 = 4;
  *(uint64_t*)0x20019058 = 0x20001000;
  *(uint8_t*)0x20019060 = 0;
  *(uint8_t*)0x20019061 = 0;
  *(uint8_t*)0x20019062 = 0;
  *(uint8_t*)0x20019063 = 0;
  *(uint8_t*)0x20019064 = 0;
  *(uint8_t*)0x20019065 = 0;
  *(uint8_t*)0x20019066 = 0;
  *(uint8_t*)0x20019067 = 0;
  *(uint8_t*)0x20019068 = 0;
  *(uint8_t*)0x20019069 = 0;
  *(uint8_t*)0x2001906a = 0;
  *(uint8_t*)0x2001906b = 0;
  *(uint8_t*)0x2001906c = 0;
  *(uint8_t*)0x2001906d = 0;
  *(uint8_t*)0x2001906e = 0;
  *(uint8_t*)0x2001906f = 0;
  *(uint8_t*)0x20019070 = 0;
  *(uint8_t*)0x20019071 = 0;
  *(uint8_t*)0x20019072 = 0;
  *(uint8_t*)0x20019073 = 0;
  *(uint8_t*)0x20019074 = 0;
  *(uint8_t*)0x20019075 = 0;
  *(uint8_t*)0x20019076 = 0;
  *(uint8_t*)0x20019077 = 0;
  *(uint8_t*)0x20019078 = 0;
  *(uint8_t*)0x20019079 = 0;
  *(uint8_t*)0x2001907a = 0;
  *(uint8_t*)0x2001907b = 0;
  *(uint8_t*)0x2001907c = 0;
  *(uint8_t*)0x2001907d = 0;
  *(uint8_t*)0x2001907e = 0;
  *(uint8_t*)0x2001907f = 0;
  *(uint8_t*)0x20019080 = 0;
  *(uint8_t*)0x20019081 = 0;
  *(uint8_t*)0x20019082 = 0;
  *(uint8_t*)0x20019083 = 0;
  *(uint8_t*)0x20019084 = 0;
  *(uint8_t*)0x20019085 = 0;
  *(uint8_t*)0x20019086 = 0;
  *(uint8_t*)0x20019087 = 0;
  *(uint8_t*)0x20019088 = 0;
  *(uint8_t*)0x20019089 = 0;
  *(uint8_t*)0x2001908a = 0;
  *(uint8_t*)0x2001908b = 0;
  *(uint8_t*)0x2001908c = 0;
  *(uint8_t*)0x2001908d = 0;
  *(uint8_t*)0x2001908e = 0;
  *(uint8_t*)0x2001908f = 0;
  *(uint8_t*)0x20019090 = 0;
  *(uint8_t*)0x20019091 = 0;
  *(uint8_t*)0x20019092 = 0;
  *(uint8_t*)0x20019093 = 0;
  *(uint8_t*)0x20019094 = 0;
  *(uint8_t*)0x20019095 = 0;
  *(uint8_t*)0x20019096 = 0;
  *(uint8_t*)0x20019097 = 0;
  *(uint8_t*)0x20019098 = 0;
  *(uint8_t*)0x20019099 = 0;
  *(uint8_t*)0x2001909a = 0;
  *(uint8_t*)0x2001909b = 0;
  *(uint8_t*)0x2001909c = 0;
  *(uint8_t*)0x2001909d = 0;
  *(uint8_t*)0x2001909e = 0;
  *(uint8_t*)0x2001909f = 0;
  *(uint8_t*)0x200190a0 = 0;
  *(uint8_t*)0x200190a1 = 0;
  *(uint8_t*)0x200190a2 = 0;
  *(uint8_t*)0x200190a3 = 0;
  *(uint8_t*)0x200190a4 = 0;
  *(uint8_t*)0x200190a5 = 0;
  *(uint8_t*)0x200190a6 = 0;
  *(uint8_t*)0x200190a7 = 0;
  *(uint8_t*)0x200190a8 = 0;
  *(uint8_t*)0x200190a9 = 0;
  *(uint8_t*)0x200190aa = 0;
  *(uint8_t*)0x200190ab = 0;
  *(uint8_t*)0x200190ac = 0;
  *(uint8_t*)0x200190ad = 0;
  *(uint8_t*)0x200190ae = 0;
  *(uint8_t*)0x200190af = 0;
  *(uint8_t*)0x200190b0 = 0;
  *(uint8_t*)0x200190b1 = 0;
  *(uint8_t*)0x200190b2 = 0;
  *(uint8_t*)0x200190b3 = 0;
  *(uint32_t*)0x200190b4 = 0;
  *(uint16_t*)0x200190b8 = 0x70;
  *(uint16_t*)0x200190ba = 0x98;
  *(uint32_t*)0x200190bc = 0;
  *(uint64_t*)0x200190c0 = 0;
  *(uint64_t*)0x200190c8 = 0;
  *(uint16_t*)0x200190d0 = 0x28;
  memcpy((void*)0x200190d2, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x200190ef = 0;
  *(uint32_t*)0x200190f0 = 0xfffffffe;
  *(uint32_t*)0x200190f8 = htobe32(0x40);
  *(uint8_t*)0x200190fc = 0xac;
  *(uint8_t*)0x200190fd = 0x14;
  *(uint8_t*)0x200190fe = 0;
  *(uint8_t*)0x200190ff = 0xbb;
  *(uint32_t*)0x20019100 = htobe32(0);
  *(uint32_t*)0x20019104 = htobe32(0);
  memcpy((void*)0x20019108,
         "\x62\x63\x73\x68\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  memcpy((void*)0x20019118,
         "\x69\x70\x5f\x76\x74\x69\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x20019128 = 0;
  *(uint8_t*)0x20019129 = 0;
  *(uint8_t*)0x2001912a = 0;
  *(uint8_t*)0x2001912b = 0;
  *(uint8_t*)0x2001912c = 0;
  *(uint8_t*)0x2001912d = 0;
  *(uint8_t*)0x2001912e = 0;
  *(uint8_t*)0x2001912f = 0;
  *(uint8_t*)0x20019130 = 0;
  *(uint8_t*)0x20019131 = 0;
  *(uint8_t*)0x20019132 = 0;
  *(uint8_t*)0x20019133 = 0;
  *(uint8_t*)0x20019134 = 0;
  *(uint8_t*)0x20019135 = 0;
  *(uint8_t*)0x20019136 = 0;
  *(uint8_t*)0x20019137 = 0;
  *(uint8_t*)0x20019138 = 0;
  *(uint8_t*)0x20019139 = 0;
  *(uint8_t*)0x2001913a = 0;
  *(uint8_t*)0x2001913b = 0;
  *(uint8_t*)0x2001913c = 0;
  *(uint8_t*)0x2001913d = 0;
  *(uint8_t*)0x2001913e = 0;
  *(uint8_t*)0x2001913f = 0;
  *(uint8_t*)0x20019140 = 0;
  *(uint8_t*)0x20019141 = 0;
  *(uint8_t*)0x20019142 = 0;
  *(uint8_t*)0x20019143 = 0;
  *(uint8_t*)0x20019144 = 0;
  *(uint8_t*)0x20019145 = 0;
  *(uint8_t*)0x20019146 = 0;
  *(uint8_t*)0x20019147 = 0;
  *(uint16_t*)0x20019148 = 0;
  *(uint8_t*)0x2001914a = 2;
  *(uint8_t*)0x2001914b = 0;
  *(uint32_t*)0x2001914c = 0;
  *(uint16_t*)0x20019150 = 0x70;
  *(uint16_t*)0x20019152 = 0x98;
  *(uint32_t*)0x20019154 = 0;
  *(uint64_t*)0x20019158 = 0;
  *(uint64_t*)0x20019160 = 0;
  *(uint16_t*)0x20019168 = 0x28;
  memcpy((void*)0x2001916a, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20019187 = 0;
  *(uint32_t*)0x20019188 = 0;
  *(uint8_t*)0x20019190 = 0;
  *(uint8_t*)0x20019191 = 0;
  *(uint8_t*)0x20019192 = 0;
  *(uint8_t*)0x20019193 = 0;
  *(uint8_t*)0x20019194 = 0;
  *(uint8_t*)0x20019195 = 0;
  *(uint8_t*)0x20019196 = 0;
  *(uint8_t*)0x20019197 = 0;
  *(uint8_t*)0x20019198 = 0;
  *(uint8_t*)0x20019199 = 0;
  *(uint8_t*)0x2001919a = 0;
  *(uint8_t*)0x2001919b = 0;
  *(uint8_t*)0x2001919c = 0;
  *(uint8_t*)0x2001919d = 0;
  *(uint8_t*)0x2001919e = 0;
  *(uint8_t*)0x2001919f = 0;
  *(uint8_t*)0x200191a0 = 0;
  *(uint8_t*)0x200191a1 = 0;
  *(uint8_t*)0x200191a2 = 0;
  *(uint8_t*)0x200191a3 = 0;
  *(uint8_t*)0x200191a4 = 0;
  *(uint8_t*)0x200191a5 = 0;
  *(uint8_t*)0x200191a6 = 0;
  *(uint8_t*)0x200191a7 = 0;
  *(uint8_t*)0x200191a8 = 0;
  *(uint8_t*)0x200191a9 = 0;
  *(uint8_t*)0x200191aa = 0;
  *(uint8_t*)0x200191ab = 0;
  *(uint8_t*)0x200191ac = 0;
  *(uint8_t*)0x200191ad = 0;
  *(uint8_t*)0x200191ae = 0;
  *(uint8_t*)0x200191af = 0;
  *(uint8_t*)0x200191b0 = 0;
  *(uint8_t*)0x200191b1 = 0;
  *(uint8_t*)0x200191b2 = 0;
  *(uint8_t*)0x200191b3 = 0;
  *(uint8_t*)0x200191b4 = 0;
  *(uint8_t*)0x200191b5 = 0;
  *(uint8_t*)0x200191b6 = 0;
  *(uint8_t*)0x200191b7 = 0;
  *(uint8_t*)0x200191b8 = 0;
  *(uint8_t*)0x200191b9 = 0;
  *(uint8_t*)0x200191ba = 0;
  *(uint8_t*)0x200191bb = 0;
  *(uint8_t*)0x200191bc = 0;
  *(uint8_t*)0x200191bd = 0;
  *(uint8_t*)0x200191be = 0;
  *(uint8_t*)0x200191bf = 0;
  *(uint8_t*)0x200191c0 = 0;
  *(uint8_t*)0x200191c1 = 0;
  *(uint8_t*)0x200191c2 = 0;
  *(uint8_t*)0x200191c3 = 0;
  *(uint8_t*)0x200191c4 = 0;
  *(uint8_t*)0x200191c5 = 0;
  *(uint8_t*)0x200191c6 = 0;
  *(uint8_t*)0x200191c7 = 0;
  *(uint8_t*)0x200191c8 = 0;
  *(uint8_t*)0x200191c9 = 0;
  *(uint8_t*)0x200191ca = 0;
  *(uint8_t*)0x200191cb = 0;
  *(uint8_t*)0x200191cc = 0;
  *(uint8_t*)0x200191cd = 0;
  *(uint8_t*)0x200191ce = 0;
  *(uint8_t*)0x200191cf = 0;
  *(uint8_t*)0x200191d0 = 0;
  *(uint8_t*)0x200191d1 = 0;
  *(uint8_t*)0x200191d2 = 0;
  *(uint8_t*)0x200191d3 = 0;
  *(uint8_t*)0x200191d4 = 0;
  *(uint8_t*)0x200191d5 = 0;
  *(uint8_t*)0x200191d6 = 0;
  *(uint8_t*)0x200191d7 = 0;
  *(uint8_t*)0x200191d8 = 0;
  *(uint8_t*)0x200191d9 = 0;
  *(uint8_t*)0x200191da = 0;
  *(uint8_t*)0x200191db = 0;
  *(uint8_t*)0x200191dc = 0;
  *(uint8_t*)0x200191dd = 0;
  *(uint8_t*)0x200191de = 0;
  *(uint8_t*)0x200191df = 0;
  *(uint8_t*)0x200191e0 = 0;
  *(uint8_t*)0x200191e1 = 0;
  *(uint8_t*)0x200191e2 = 0;
  *(uint8_t*)0x200191e3 = 0;
  *(uint32_t*)0x200191e4 = 0;
  *(uint16_t*)0x200191e8 = 0x98;
  *(uint16_t*)0x200191ea = 0xe0;
  *(uint32_t*)0x200191ec = 0;
  *(uint64_t*)0x200191f0 = 0;
  *(uint64_t*)0x200191f8 = 0;
  *(uint16_t*)0x20019200 = 0x28;
  memcpy((void*)0x20019202, "\x73\x74\x61\x74\x65\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2001921f = 0;
  *(uint32_t*)0x20019220 = 0;
  *(uint16_t*)0x20019228 = 0x48;
  memcpy((void*)0x2001922a, "\x54\x45\x45\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20019247 = 1;
  *(uint32_t*)0x20019248 = htobe32(8);
  memcpy((void*)0x20019258,
         "\x62\x70\x71\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint64_t*)0x20019268 = 4;
  *(uint8_t*)0x20019270 = 0;
  *(uint8_t*)0x20019271 = 0;
  *(uint8_t*)0x20019272 = 0;
  *(uint8_t*)0x20019273 = 0;
  *(uint8_t*)0x20019274 = 0;
  *(uint8_t*)0x20019275 = 0;
  *(uint8_t*)0x20019276 = 0;
  *(uint8_t*)0x20019277 = 0;
  *(uint8_t*)0x20019278 = 0;
  *(uint8_t*)0x20019279 = 0;
  *(uint8_t*)0x2001927a = 0;
  *(uint8_t*)0x2001927b = 0;
  *(uint8_t*)0x2001927c = 0;
  *(uint8_t*)0x2001927d = 0;
  *(uint8_t*)0x2001927e = 0;
  *(uint8_t*)0x2001927f = 0;
  *(uint8_t*)0x20019280 = 0;
  *(uint8_t*)0x20019281 = 0;
  *(uint8_t*)0x20019282 = 0;
  *(uint8_t*)0x20019283 = 0;
  *(uint8_t*)0x20019284 = 0;
  *(uint8_t*)0x20019285 = 0;
  *(uint8_t*)0x20019286 = 0;
  *(uint8_t*)0x20019287 = 0;
  *(uint8_t*)0x20019288 = 0;
  *(uint8_t*)0x20019289 = 0;
  *(uint8_t*)0x2001928a = 0;
  *(uint8_t*)0x2001928b = 0;
  *(uint8_t*)0x2001928c = 0;
  *(uint8_t*)0x2001928d = 0;
  *(uint8_t*)0x2001928e = 0;
  *(uint8_t*)0x2001928f = 0;
  *(uint8_t*)0x20019290 = 0;
  *(uint8_t*)0x20019291 = 0;
  *(uint8_t*)0x20019292 = 0;
  *(uint8_t*)0x20019293 = 0;
  *(uint8_t*)0x20019294 = 0;
  *(uint8_t*)0x20019295 = 0;
  *(uint8_t*)0x20019296 = 0;
  *(uint8_t*)0x20019297 = 0;
  *(uint8_t*)0x20019298 = 0;
  *(uint8_t*)0x20019299 = 0;
  *(uint8_t*)0x2001929a = 0;
  *(uint8_t*)0x2001929b = 0;
  *(uint8_t*)0x2001929c = 0;
  *(uint8_t*)0x2001929d = 0;
  *(uint8_t*)0x2001929e = 0;
  *(uint8_t*)0x2001929f = 0;
  *(uint8_t*)0x200192a0 = 0;
  *(uint8_t*)0x200192a1 = 0;
  *(uint8_t*)0x200192a2 = 0;
  *(uint8_t*)0x200192a3 = 0;
  *(uint8_t*)0x200192a4 = 0;
  *(uint8_t*)0x200192a5 = 0;
  *(uint8_t*)0x200192a6 = 0;
  *(uint8_t*)0x200192a7 = 0;
  *(uint8_t*)0x200192a8 = 0;
  *(uint8_t*)0x200192a9 = 0;
  *(uint8_t*)0x200192aa = 0;
  *(uint8_t*)0x200192ab = 0;
  *(uint8_t*)0x200192ac = 0;
  *(uint8_t*)0x200192ad = 0;
  *(uint8_t*)0x200192ae = 0;
  *(uint8_t*)0x200192af = 0;
  *(uint8_t*)0x200192b0 = 0;
  *(uint8_t*)0x200192b1 = 0;
  *(uint8_t*)0x200192b2 = 0;
  *(uint8_t*)0x200192b3 = 0;
  *(uint8_t*)0x200192b4 = 0;
  *(uint8_t*)0x200192b5 = 0;
  *(uint8_t*)0x200192b6 = 0;
  *(uint8_t*)0x200192b7 = 0;
  *(uint8_t*)0x200192b8 = 0;
  *(uint8_t*)0x200192b9 = 0;
  *(uint8_t*)0x200192ba = 0;
  *(uint8_t*)0x200192bb = 0;
  *(uint8_t*)0x200192bc = 0;
  *(uint8_t*)0x200192bd = 0;
  *(uint8_t*)0x200192be = 0;
  *(uint8_t*)0x200192bf = 0;
  *(uint8_t*)0x200192c0 = 0;
  *(uint8_t*)0x200192c1 = 0;
  *(uint8_t*)0x200192c2 = 0;
  *(uint8_t*)0x200192c3 = 0;
  *(uint32_t*)0x200192c4 = 0;
  *(uint16_t*)0x200192c8 = 0x70;
  *(uint16_t*)0x200192ca = 0x98;
  *(uint32_t*)0x200192cc = 0;
  *(uint64_t*)0x200192d0 = 0;
  *(uint64_t*)0x200192d8 = 0;
  *(uint16_t*)0x200192e0 = 0x28;
  memcpy((void*)0x200192e2, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x200192ff = 0;
  *(uint32_t*)0x20019300 = 0;
  *(uint64_t*)0x20001000 = 0;
  *(uint64_t*)0x20001008 = 0;
  *(uint64_t*)0x20001010 = 0;
  *(uint64_t*)0x20001018 = 0;
  *(uint64_t*)0x20001020 = 0;
  *(uint64_t*)0x20001028 = 0;
  *(uint64_t*)0x20001030 = 0;
  *(uint64_t*)0x20001038 = 0;
  syscall(__NR_setsockopt, r[1], 0, 0x40, 0x20019000, 0x308);
}

int main()
{
  loop();
  return 0;
}
