// https://syzkaller.appspot.com/bug?id=a84d6ad70b281bfc5632f272f745104fb43d219d
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

uint64_t r[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
void loop()
{
  long res = 0;
  memcpy((void*)0x20000340, "keyring", 8);
  *(uint8_t*)0x20000380 = 0x73;
  *(uint8_t*)0x20000381 = 0x79;
  *(uint8_t*)0x20000382 = 0x7a;
  *(uint8_t*)0x20000383 = 0;
  *(uint8_t*)0x20000384 = 0;
  res = syscall(__NR_add_key, 0x20000340, 0x20000380, 0, 0, 0xfffffffe);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000400, "keyring", 8);
  *(uint8_t*)0x200003c0 = 0x73;
  *(uint8_t*)0x200003c1 = 0x79;
  *(uint8_t*)0x200003c2 = 0x7a;
  *(uint8_t*)0x200003c3 = 0;
  *(uint8_t*)0x200003c4 = 0;
  res = syscall(__NR_add_key, 0x20000400, 0x200003c0, 0, 0, r[0]);
  if (res != -1)
    r[1] = res;
  memcpy((void*)0x20000040, "user", 5);
  *(uint8_t*)0x20000080 = 0x73;
  *(uint8_t*)0x20000081 = 0x79;
  *(uint8_t*)0x20000082 = 0x7a;
  *(uint8_t*)0x20000083 = 0;
  *(uint8_t*)0x20000084 = 0;
  memcpy((void*)0x200001c0, "\xda", 1);
  res = syscall(__NR_add_key, 0x20000040, 0x20000080, 0x200001c0, 1, r[1]);
  if (res != -1)
    r[2] = res;
  memcpy((void*)0x200000c0, "keyring", 8);
  *(uint8_t*)0x20000180 = 0x73;
  *(uint8_t*)0x20000181 = 0x79;
  *(uint8_t*)0x20000182 = 0x7a;
  *(uint8_t*)0x20000183 = 0;
  *(uint8_t*)0x20000184 = 0;
  res = syscall(__NR_add_key, 0x200000c0, 0x20000180, 0, 0, r[1]);
  if (res != -1)
    r[3] = res;
  memcpy((void*)0x203bd000, "user", 5);
  *(uint8_t*)0x20000280 = 0x73;
  *(uint8_t*)0x20000281 = 0x79;
  *(uint8_t*)0x20000282 = 0x7a;
  *(uint8_t*)0x20000283 = 0;
  *(uint8_t*)0x20000284 = 0;
  memcpy((void*)0x20000440, "", 1);
  res = syscall(__NR_add_key, 0x203bd000, 0x20000280, 0x20000440, 1, r[3]);
  if (res != -1)
    r[4] = res;
  memcpy((void*)0x20fc0ffb, "user", 5);
  *(uint8_t*)0x20752ffb = 0x73;
  *(uint8_t*)0x20752ffc = 0x79;
  *(uint8_t*)0x20752ffd = 0x7a;
  *(uint8_t*)0x20752ffe = 0x23;
  *(uint8_t*)0x20752fff = 0;
  memcpy((void*)0x203eb000, "\xb3", 1);
  res = syscall(__NR_add_key, 0x20fc0ffb, 0x20752ffb, 0x203eb000, 1, r[3]);
  if (res != -1)
    r[5] = res;
  *(uint32_t*)0x200006c0 = r[4];
  *(uint32_t*)0x200006c4 = r[5];
  *(uint32_t*)0x200006c8 = r[2];
  *(uint64_t*)0x20c61fc8 = 0x20a3dffa;
  memcpy((void*)0x20a3dffa,
         "\x73\x68\x61\x35\x31\x32\x2d\x67\x65\x6e\x65\x72\x69\x63\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         64);
  *(uint64_t*)0x20c61fd0 = 0;
  *(uint32_t*)0x20c61fd8 = 0;
  *(uint32_t*)0x20c61fdc = 0;
  *(uint32_t*)0x20c61fe0 = 0;
  *(uint32_t*)0x20c61fe4 = 0;
  *(uint32_t*)0x20c61fe8 = 0;
  *(uint32_t*)0x20c61fec = 0;
  *(uint32_t*)0x20c61ff0 = 0;
  *(uint32_t*)0x20c61ff4 = 0;
  *(uint32_t*)0x20c61ff8 = 0;
  syscall(__NR_keyctl, 0x17, 0x200006c0, 0x20000540, 5, 0x20c61fc8);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
