// https://syzkaller.appspot.com/bug?id=bae12042b390b8a893dc00d72d37e7a667632f66
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
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

struct csum_inet {
  uint32_t acc;
};

static void csum_inet_init(struct csum_inet* csum)
{
  csum->acc = 0;
}

static void csum_inet_update(struct csum_inet* csum, const uint8_t* data,
                             size_t length)
{
  if (length == 0)
    return;

  size_t i;
  for (i = 0; i < length - 1; i += 2)
    csum->acc += *(uint16_t*)&data[i];

  if (length & 1)
    csum->acc += (uint16_t)data[length - 1];

  while (csum->acc > 0xffff)
    csum->acc = (csum->acc & 0xffff) + (csum->acc >> 16);
}

static uint16_t csum_inet_digest(struct csum_inet* csum)
{
  return ~csum->acc;
}

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res = 0;
  res = syscall(__NR_socket, 0x11, 0x800000003, 0x81);
  if (res != -1)
    r[0] = res;
  *(uint16_t*)0x205a2000 = 0x11;
  memcpy((void*)0x205a2002,
         "\x00\x00\x01\x00\x00\x00\x00\x00\x08\x00\x44\x94\x4e\xeb\xa7\x1a\x49"
         "\x76\xe2\x52\x92\x2c\xb1\x8f\x6e\x2e\x2a\xba\x00\x00\x00\x01\x2e\x0b"
         "\x38\x36\x00\x54\x04\xb0\xe0\x30\x1a\x4c\xe8\x75\xf2\xe3\xff\x5f\x16"
         "\x3e\xe3\x40\xb7\x67\x95\x00\x80\x00\x00\x00\x00\x00\x00\x01\x01\x01"
         "\x3c\x58\x11\x03\x9e\x15\x77\x50\x27\xec\xce\x66\xfd\x79\x2b\xbf\x0e"
         "\x5b\xf5\xff\x1b\x08\x16\xf3\xf6\xdb\x1c\x00\x01\x00\x00\x00\x00\x00"
         "\x00\x00\x49\x74\x00\x00\x00\x00\x00\x00\x00\x06\xad\x8e\x5e\xcc\x32"
         "\x6d\x3a\x09\xff\xc2\xc6\x54",
         126);
  syscall(__NR_bind, r[0], 0x205a2000, 0x80);
  *(uint64_t*)0x20000080 = 0x20001100;
  memcpy(
      (void*)0x20001100,
      "\x47\xbb\x40\xaa\xc7\x2d\x29\x4d\xb9\x76\xf2\xd0\x44\x89\xab\xb5\x72\xfe"
      "\xd9\x1e\xb1\x8e\x2b\x00\x21\x2f\x2b\x9f\x3a\xa9\x09\x20\x9a\x54\xda\x79"
      "\xd9\x8e\xb5\xfe\xe9\xf6\xf7\xcc\x5a\xec\xe4\xff\x7e\xef\x77\xba\x49\x63"
      "\x5d\x2b\x49\xa8\xf7\xb6\x4c\x1a\x14\x18\x71\x80\xbf\x2e\x4e\x68\x76\x1a"
      "\xf0\xec\xb4\x96\x0d\x7b\x77\x77\xf1\x55\x18\x32\x51\x09\xec\xa3\x6d\x7a"
      "\x20\x56\xe0\x61\x22\xa1\x5f\x49\x79\xcb\xe4\x16\x2e\x17\xec\x4e\x0b\x5c"
      "\xfa\xd3\x20\xc7\x3a\x24\x6e\x08\xc0\xcd\xc0\xad\xfd\x18\x5b\x9b\xbc\x32"
      "\x1b\x8b\xc0\x78\xdd\x39\x56\x81\xf1\xc6\x2b\xa2\xff\xe2\x54\x43\x77\x69"
      "\x57\x52\xfc\xc0\x50\xb4\xe9\xc2\x47\xcb\xfb\xc2\x65\x40\x5b\xe0\x5a\x97"
      "\x92\xb7\xc2\x6a\xfd\x9b\x2a\xc7\xd4\x7f\x49\xc4\xae\x47\xb6\x74\x89\xb7"
      "\x27\xad\xa7\x0a\xe4\xb5\x8d\x1d\xb1\x03\x9c\x01\xa4\x7a\xd5\xc5\x9a\x30"
      "\x8c\xaa\x07\xac\xc5\xbe\xd2\x4b\x86\x1e\x47\xe8\x2e\x9f\x13\x2b\x5e\x69"
      "\xd0\xb8\x93\x18\x7d\x2d\x91\xe5\x9f\x5c\x62\x66\x20\xc2\x79\x49\x31\x97"
      "\x52\xb5\xfe\x5b\xa8\x37\xbc\xa2\x32\xff\x87\xde\x0f\x15\xc9\xd0\x5e\x66"
      "\x5d\x01\x4c\x3d\xd5\xba\x5d\xa9\x9d\x7d\x1f\x62\xff\x47\x13\xad\x7f\xcf"
      "\x17\x97\x06\x62\xbf\xa0\xc8\x97\x63\x6d\x78\x7f\xbf\x70\x2a\x88\x85\x93"
      "\xf9\xd2\x13\xd6\x59\x47\x37\xdd\x21\x2e\x79\x0c\xfd\xac\x92\xb2\x9e\x55"
      "\xd3\x1a\xb8\x61\x8f\x08\xbd\x94\x7b\xba\xb2\x47\x3c\xb5\xdb\xe0\xdf\x9d"
      "\x8e\x33\xbb\x8a\x21\xf1\x06\xf1\xbe\xc6\x41\x8d\xcc\xb0\x28\x2e\x75\x3a"
      "\xea\x3c\xf9\xab\xb2\x03\x90\x43\xf1\x08\xc6\x21\x05\xf5\xfa\x25\x18\x70"
      "\xc4\xb3\xea\xd3\xad\x6b\x56\x2c\x4b\xa2\x1a\xaa\x38\x2b\xa3\x4e\x1c\xcc"
      "\x33\x68\x95\xdf\x62\x32\x9a\x6c\x27\xf6\x37\x1d\xc8\xe6\x3d\x89\x40\x9a"
      "\xdb\x35\xd8\x8e\x66\x08\x59\x91\x21\x2d\x3d\xa1\x4b\x94\x53\x0c\x18\xf0"
      "\x83\x6a\x75\x9f\x47\x6a\x91\x1a\xf1\x03\xa7\x2a\xd8\x6c\x39\xdd\xca\xa8"
      "\xf6\xee\x22\x0c\xd3\x52\xda\x8a\x71\xd7\x7d\xb1\x90\x01\x09\xf1\x40\xbf"
      "\xf0\x0f\x53\xac\x42\x49\x1e\x0b\xab\xc3\x78\xaf\xe5\xdf\x93\x06\x0b\x78"
      "\x99\x00\x20\x34\x93\xca\x8c\xc7\xa2\x00\x7e\x0c\x72\x23\x7c\x63\x0a\x37"
      "\xf0\x24\xb8\xca\x98\x53\xdf\x97\xf6\x50\xfb\x2f\x7f\x31\x63\x44\x06\x0b"
      "\x39\x5f\xb7\xa8\x38\xfe\x9d\x7c\x52\x72\x51\xc0\xef\x24\x57\xfb\xfc\x37"
      "\xb8\xb7\x67\x33\x7e\x80\xdd\x09\xe4\xda\x8b\x11\x44\x9a\x2f\x7a\x22\x87"
      "\xa7\xe7\xd7\xae\x93\xe2\x57\x6b\x88\xd2\x3d\x47\xf6\x53\x5e\xe6\x1e\x0f"
      "\x98\x83\x69\xa7\xa9\x89\x78\xce\xe2\x18\x15\xac\x37\x3d\x8e\x5d\x41\x61"
      "\x0c\xb1\xc3\x83\xb7\xbd\x23\x2d\xb0\x4f\xfc\xf7\x8a\x39\x3e\x4f\x11\x4a"
      "\x01\x0f\x5c\xa0\x9c\x92\x30\x7e\xa3\x49\xa2\xa9\xb6\x89\x49\x0d\x5a\xf0"
      "\x5b\x9f\x88\xa3\xb2\xec\x7c\xf7\x8c\x3c\xf8\x64\x07\xa0\x40\xb6\x83\x26"
      "\x25\x39\x9f\xef\x52\x24\x7c\xb6\x8b\xf1\x81\x25\x1b\xf4\x39\xee\x29\x8c"
      "\x44\x14\x98\x8d\x54\x79\xde\xe1\x21\x12\x26\x71\x59\x95\x49\x11\x8e\x35"
      "\x3f\x8d\xb5\xf0\x4d\x4a\x0f\x25\x37\xa8\xfa\x98\x47\x2d\x8e\x00\x2b\x61"
      "\x88\x8f\x67\x74\xc0\x8d\xd2\x98\x73\x7e\xbd\x5f\x7e\xc8\x81\xb8\xcd\x5b"
      "\x7e\x29\xf2\x6f\xb9\x07\x23\x01\x69\xc7\xd8\x7c\x34\x77\x85\xb1\xc9\x43"
      "\x15\xae\xc7\xb1\xdc\x13\x5a\x40\x62\x79\xa6\x68\xf5\x94\x3a\xa5\x57\x71"
      "\x40\x62\xfa\x8b\xc6\x4c\xd6\x93\xad\xe0\x0b\x1a\x6a\xd5\x5f\x6f\xe3\xcd"
      "\x18\xf4\xf9\x37\xef\xb3\x8c\xfe\x94\x3a\x78\x51\xe7\x8e\xe6\x42\x9b\x61"
      "\xe0\x30\xd2\x0e\xde\x01\x6c\xe6\x45\x53\xf1\xdb\x29\x78\x6d\x96\x2e\x3c"
      "\xb1\x8a\x55\x6a\xba\x03\x34\x23\xfa\xb2\xce\x7e\x0a\x17\xbb\x4c\x46\xf5"
      "\x39\xf5\xd2\x2d\x05\x1f\x96\xc2\x81\x6a\x96\xe7\x3c\x0e\xff\x89\x6b\xdc"
      "\x13\xb0\xf5\x37\xe0\xcb\x0d\xc9\x4b\xf2\x17\x12\xb0\x67\x61\x35\x99\x37"
      "\x94\xfe\x04\x3e\xb0\x4b\x62\xc7\xd2\xec\xfe\x0e\xb6\x59\xf8\xd6\x77\x13"
      "\x09\x6e\xf4\x9f\x8c\x7c\x03\x9c\xd9\x44\x2b\x22\x19\xba\x41\xbc\xeb\x08"
      "\xad\x22\x87\xbf\x0c\x00\x22\x72\x5c\xa6\x7f\x12\xd0\xf1\x37\x1a\x76\x99"
      "\xfd\xbb\x04\x47\x68\x19\xfb\x50\xd0\x0f\x69\x2a\x46\x54\x40\xd3\x56\xc9"
      "\xf6\xe7\x21\xc0\xcf\x4b\xd5\x91\x2f\xba\xc2\x00\xaf\xb5\xa4\x15\x1f\x6a"
      "\x02\x94\x26\xfd\x53\xf7\x7e\x19\xfa\x37\x54\xac\xe2\xe4\xd4\x41\xd9\x40"
      "\x03\xbb\xa5\x79\x97\x2e\x96\xc5\x83\xcc\xbe\x8c\xee\x81\x12\x68\x9b\x2e"
      "\x24\x24\x88\xca\xcc\x0d\x1a\xf2\x32\xc6\x50\x86\x93\x0b\xe5\xc9\x57\xe8"
      "\xe3\x63\xb0\x3d\xa5\xff\xcf\xa5\xfa\x98\xc9\xa2\x14\x82\x9d\xce\x23\xa1"
      "\xae\xb1\x02\x62\x90\x38\x2c\x12\xe0\x2a\x9f\xc5\xd1\x8e\x85\xdf\x75\xa6"
      "\xda\x75\xe7\x0a\xf7\xa1\x62\xc7\x1f\xc5\xdc\x5e\xe2\x6e\x3f\x0a\x9b\xba"
      "\x94\x75\xeb\x51\x08\x50\xc0\x6f\x02\x76\xd8\x86\x13\xe2\xeb\x7c\x25\xb0"
      "\xe0\x4d\x47\x5d\x4d\xce\x1a\xa0\xe3\xc8\x29\x06\x2b\x68\x7e\x2b\x4c\x4f"
      "\x16\xd2\xbc\xdc\xf5\x50\x3a\x77\x42\x13\x26\x8c\xa2\x87\x0b\xf4\xd8\x98"
      "\xdc\x4a\x73\x0a\x00\xf1\x66\x60\xfd\x71\x0e\x51\xfd\x51\x71\x68\x0f\x99"
      "\x33\x37\xb4\xe7\x70\x55\xff\x9f\x6f\xba\xc2\xce\x24\xb7\xc5\x1e\x8d\x77"
      "\x57\x54\x1c\xed\x48\x80\xb0\xd0\x16\x33\x17\xfc\xcc\xd3\x0a\xf0\x68\xd4"
      "\x9f\x43\x74\x35\xf5\xc4\x06\x02\x54\x81\x58\x7b\xb5\x17\x96\x39\x99\xe0"
      "\xa3\xef\x67\xde\xd4\x19\xf3\x9f\x57\x8a\xad\x12\x71\x76\xdb\x0a\xd8\xf8"
      "\x91\x79\xf7\xa7\x37\x1d\xb7\x59\x85\x13\xe1\x40\x04\xc4\x0f\x0c\x55\x41"
      "\xa7\x50\xe8\xa5\xde\xc9\xa7\x09\x80\xc7\x26\x55\x27\xaf\x4b\x6a\x88\x87"
      "\x2e\x55\x38\x90\xf8\x9e\xdd\x9e\xce\xd6\x7c\x07\xf3\xe4\xc5\x8e\x70\x65"
      "\x8c\x18\xe5\xb0\x15\x09\xf0\xe7\x02\x53\xe0\x5c\x53\x95\x9b\x2b\x99\x39"
      "\x43\x36\xca\x04\x75\xd6\xae\xff\x94\x18\xb6\x88\xcd\x09\x2f\xca\x72\xeb"
      "\x18\x32\xc6\xe5\x59\xfd\xfb\xd8\x66\xba\x12\x0b\x8c\x27\x54\x39\x03\x45"
      "\x6d\x47\x3f\x4e\xf3\xa5\x69\x75\x69\xe3\x80\xed\x21\x5b\x88\xd5\xde\xf9"
      "\x11\xbe\xbb\xfe\x3c\xc4\xce\x3c\xc8\x45\x21\xcb\xb9\xed\x34\xdb\x93\xc7"
      "\x90\x80\xf0\x42\xc7\xee\xc5\x43\x62\x54\x45\x3a\x39\x6a\x69\x25\xf4\x45"
      "\x77\x23\xe2\xc3\xb3\xc7\xf6\x60\x26\x99\xe7\x26\xe0\x3f\x80\x9c\xbf\xfa"
      "\xa8\x64\x96\x70\x85\xec\x5c\xe7\x7f\x4c\xa2\x98\xd2\xee\x82\xd0\x5f\x50"
      "\x69\x7c\x80\x79\x8a\xa5\xc3\x99\xce\x6c\x36\x9e\xf6\x74\x59\x79\x22\x80"
      "\x21\xc1\x9a\x51\x07\xeb\x6c\x53\x97\x6c\x1c\x5c\x28\x50\xf1\x9f\x01\x0f"
      "\x10\xfe\x97\x20\xae\x0a\x1c\x31\xd2\x06\x8c\xc0\x5e\x6c\x2b\xab\x4e\x76"
      "\x05\xa6\xaa\xef\xe1\x7f\xf0\x84\x80\x55\xd4\xf0\xc2\x42\x10\x75\xb4\x8c"
      "\x55\x3a\xbf\x63\x65\xf5\xb5\xed\x76\xb0\x30\xfc\x27\xea\xaf\xf9\x63\x9c"
      "\x88\x59\xde\x80\x94\x14\xc8\xf7\x83\xe8\xc6\x6c\xcf\xd4\x78\xae\x6f\x58"
      "\x6b\xab\xf9\x8b\xfb\xb7\xa3\xc8\x90\x15\xb9\x45\x70\x79\xc2\xf7\x2b\x2e"
      "\xa7\x6c\xb5\x5d\x28\x4f\xda\x5c\x5e\x30\xbb\x58\x1d\xd7\x43\x74\x55\x6a"
      "\x77\x6f\xcb\x73\xf0\x8c\x04\x25\x8c\x53\x14\xb4\x82\xb9\x1d\xf4\x07\x82"
      "\xc1\xf0\x04\x13\x69\x1b\x91\x3b\xe1\xd0\x1a\x3f\xfb\x16\xc7\x31\xa1\xec"
      "\x67\x99\x0b\xf0\x14\xfd\xda\xe2\x26\x7b\xde\x05\x12\xa4\x11\xa9\x25\x54"
      "\xe2\xf8\xeb\x43\x84\x0c\x85\xcf\xd0\xb3\x05\x1f\x2d\x04\x2e\x69\x2a\xda"
      "\xda\x63\x6d\x8a\x18\xba\xe4\xdb\x70\x63\x12\x85\xf6\xf3\xeb\x3c\xcc\xf9"
      "\x5e\x21\x57\x58\x4d\x82\x88\x84\x55\x7b\x3f\x9b\x40\xc1\x9c\xc8\x49\x78"
      "\x3d\x21\xc1\xc8\x2a\x72\xcf\x68\x5a\x9b\xf1\x1a\x07\x37\x0e\xbd\x40\x8a"
      "\x20\x1f\xc0\x6c\xf2\x58\xff\x9e\x88\xae\x16\xf4\x1c\x16\xeb\x3e\xd1\xa7"
      "\x47\x3c\x20\x05\x89\xc4\x58\x33\x54\xea\x7f\xe4\xbb\x33\x2f\xe4\x2d\xe7"
      "\xef\x3b\x87\x53\x11\xbd\xad\x50\x92\x27\x26\xab\x9e\x68\x0b\xd2\x77\x2b"
      "\x9c\xb5\xd6\x02\x18\xb9\x41\xdf\xd0\x62\xed\x90\xef\x08\x27\x76\x29\x81"
      "\xca\xb3\x2c\xc5\xf4\x82\x32\x8a\xad\x7b\x0b\x03\x5a\x26\x30\xdb\x27\x87"
      "\x51\x44\xd8\xf7\x79\x14\x6d\x9f\x8c\xb5\x7c\x60\xe8\xd3\xe8\xeb\xbd\x65"
      "\xaf\x43\x09\x6d\x73\x75\x5b\xbf\x2c\x52\x06\xb1\xcf\x39\x01\x29\xf5\x5d"
      "\x34\xd7\x58\x96\x10\x1f\xcd\xfe\xf7\x62\x70\x99\xc5\xb6\x66\x36\x28\x4c"
      "\xd1\x14\x7e\x7b\x91\x0c\xc4\xfc\x82\xac\xb3\xf5\x50\x14\x31\xe2\x95\xbb"
      "\x04\x6b\x5e\xfc\xbd\xae\x8c\x41\xb7\x2b\x68\x91\xcb\xdd\xa5\x6a\x57\x48"
      "\x94\x33\xa6\xbf\xcf\x7f\x8b\x35\x6f\x87\xf7\x07\xe2\xa0\x71\x97\xe1\x6a"
      "\x1d\xf7\x55\x27\xb3\x15\x3c\xe4\x4c\xcc\xa0\xd7\xa1\x20\xfb\x6a\xed\x49"
      "\x55\xf7\x09\xa4\x40\xd9\xd9\x6f\x7b\x23\x13\xe6\xc3\x14\xa5\xe3\x4e\x2a"
      "\xb7\xbd\xd9\x44\x39\xc8\xd1\xc4\xdb\x6a\x64\xd9\xa9\xdd\xbe\x58\xc1\xac"
      "\xe2\x80\x0f\x8f\x6d\xb2\xee\xa7\x04\x42\xb6\x4c\xcd\x57\x3d\x38\x95\x02"
      "\xd9\xc2\x35\x4b\x82\x0e\xc6\xe9\x65\xe1\x37\xac\x46\x34\x79\x1b\x76\xa7"
      "\x74\x0c\x59\xb3\x2d\x96\xea\xa9\x91\x66\x3f\xc3\x2f\xc1\x59\x49\x30\xf7"
      "\x07\x9e\xf7\xe3\xac\x23\x47\x16\xd5\x6f\x7a\xf1\xfc\x2e\xed\x51\x22\xcf"
      "\x0f\x35\xd9\x90\xbe\xd3\x13\x1c\x02\x79\x9b\xea\xa7\xc3\xb0\x2f\xb2\x56"
      "\x0d\x6f\x82\xb2\x1c\xec\x1a\x99\x20\x63\x63\xeb\xf6\x8c\x03\xcf\xfb\x2d"
      "\xc5\xc1\xd9\x59\x30\x44\x4b\x11\x80\x8a\xfc\xb5\x46\x6e\x00\xe9\xe2\xf8"
      "\xea\x54\x86\x5d\x4e\x26\x9f\x45\x7b\xe7\x7f\x8b\xbf\x31\x5c\x1e\xb0\x07"
      "\xd0\xa0\x4c\x16\x56\x48\x1b\x91\x75\xa2\xef\xb9\xa5\x80\x0e\x7a\x43\xbc"
      "\x70\xe7\xb0\x37\x0a\x34\x69\x08\xbc\xf4\xf9\xa9\x97\x62\xee\xb3\xb0\xba"
      "\x3b\x73\x3b\x99\x80\x27\xf4\x41\x97\x52\xae\x87\xff\xe1\x3f\x3a\x36\x11"
      "\x15\x51\xd3\x8f\xeb\x4a\x88\x5f\x94\x44\x7a\xdf\x01\xba\xfe\x0d\xce\x19"
      "\x04\x96\xc7\x45\x47\x30\x83\x11\x45\xf7\x3c\x69\xa4\xe4\x4c\xf7\x8a\x73"
      "\xd9\xb8\xbc\x45\x84\x0e\xc9\xcc\x2d\xed\x23\xfe\xdc\x31\xda\xcb\x97\x66"
      "\xd3\x52\x3d\x21\xd8\x6b\x80\x1b\x98\xeb\x03\xd7\x80\x2c\x0b\x4a\xca\x25"
      "\xf1\xcc\x9d\xf6\x7e\xe9\x72\x94\x2e\xcc\xac\x16\xe7\xc8\xf5\xf2\xac\x20"
      "\x31\x81\x46\x5d\xcf\xbb\x1b\x52\xc1\xcc\x08\x93\xe6\x66\x16\x71\x0e\x54"
      "\x7e\xe3\x31\x8d\xc6\x3e\x8e\x9e\xf9\xfd\x2f\x87\x5a\x2a\x00\xfc\xba\xb7"
      "\x6a\x87\x4f\x55\x20\x80\x68\x01\x9f\x87\xb1\x16\x30\x16\xb9\x46\xa6\x2a"
      "\x72\xe4\x98\x3a\xa7\xa8\xf8\x18\x6f\x05\x37\x09\xb0\xa9\xa8\xe8\x4e\xd1"
      "\x4f\x72\xc3\x00\xc8\x81\x50\xa4\x9b\x31\x8d\x3d\x79\x4f\x2a\xc7\x33\x4b"
      "\x0b\x1c\x33\x4f\x86\x31\x8c\x84\x2f\xca\xf6\x3c\xbd\x2b\x9a\x73\x55\xef"
      "\x25\x6a\x0c\xfb\x12\x33\xae\xe1\x4e\xc6\xf0\x19\x4f\x8a\x5d\x36\x5d\xf8"
      "\xd3\x86\x09\xe8\x59\x8d\xf5\x26\xab\x08\x21\x64\xab\x17\x30\xcf\x01\xd9"
      "\x2e\x70\x81\x65\x6a\xe3\xc1\xb7\xaf\x75\x0c\xe4\x7c\x4b\xed\x70\x5a\x68"
      "\xc8\x1e\x59\xc8\x69\xb8\xfa\x68\xfa\xdc\x2c\x54\xb8\x5e\xcb\x08\x4f\xb2"
      "\xaf\x91\xd0\x2e\xa4\xd9\x40\xbb\x83\x8a\x88\x8a\xa1\xaf\xb7\x59\x50\x0f"
      "\x2d\x9b\x18\xfc\xf8\x37\xde\x0d\xe6\xe9\x47\x6e\x44\x0e\x7d\x23\x5f\xdd"
      "\x04\x4f\x6f\x5b\x65\x70\x3a\xd4\x22\xfb\xaf\xc6\xa9\x28\x69\x3b\x2d\x98"
      "\x7a\x23\xa8\x8b\x7d\xa6\xb2\xa3\x49\x25\x85\xef\x34\x97\x0b\x27\xec\xda"
      "\xc9\x55\x5b\x61\x54\x94\xfe\x60\x89\xe3\x2b\x0f\x36\xca\x0e\x3c\xef\xa8"
      "\x08\xe8\x04\x7d\x4d\xe6\x35\x71\xdb\x16\xbc\xae\x15\x5e\x8a\xef\x0b\x57"
      "\x4a\xe8\xc3\xfa\x07\x45\x73\x8a\x52\x34\x81\x48\x27\xe6\x44\x33\x6f\x3a"
      "\xc4\x8b\x7b\x46\xf1\xb4\x2f\xc9\x5d\x26\x5a\x61\x33\x94\x96\x3f\xa6\x27"
      "\xf8\x95\xd2\x76\x26\x4e\x86\x75\x00\x36\x51\x04\x08\x10\xcc\xe5\xf4\x2b"
      "\xf6\x15\xba\xb4\x54\xfa\xad\x0f\xb6\xca\xc0\x11\xd8\x02\x49\xd3\x02\x57"
      "\x5e\x1e\xf5\x9c\x82\x72\x13\xb5\xf2\xe5\xcd\x5f\xc1\xfe\xda\xb4\x0f\x46"
      "\x97\x9d\x06\xfb\xd9\xc6\xed\x0e\x62\x9b\x32\x71\x96\x00\x4e\x29\x1b\x1c"
      "\x09\x70\x14\xd5\xff\xa4\xd3\x01\x51\xa6\x56\x5d\x5f\x2f\xd4\x2d\xca\x4c"
      "\xbb\x1c\x72\x4e\xd3\xe4\x50\x60\x6f\xc5\x87\xab\xa2\xd1\xa6\x55\xa4\x1e"
      "\xb5\xa5\x7a\x49\xa8\x55\x7e\xaf\x26\xfa\x52\xdd\xd5\x16\xa9\x3e\xbc\x19"
      "\x2f\xef\xf8\x2b\x78\x4f\xcd\x63\x0a\x9c\xd9\x33\x91\x58\x2a\xee\xc5\x0e"
      "\xa2\x6c\xa0\xa2\x3d\x07\xb6\x9e\xb8\xed\x87\x2a\xa5\xb7\x56\xf5\x46\x87"
      "\x57\xc8\xe5\xc1\x43\x69\x05\xfc\xd3\x29\x88\xd3\xb5\xea\x36\xe5\xc1\xe9"
      "\x63\x21\xd9\x12\x76\x57\x12\x7c\x87\xf2\xdd\x7f\x36\x9a\x41\x07\x26\xf7"
      "\x71\x74\x3a\x5e\x9a\xd6\x41\x55\x4e\xfd\x6e\x57\xfc\x30\xa6\xfe\x0f\x55"
      "\x6c\x72\xb9\x4d\xe0\x41\xe6\xf1\xca\x81\x68\x66\x8a\x96\x67\xd1\xa4\x0f"
      "\xfd\xd5\x24\x77\xd6\xe7\xf8\xe4\x77\x4e\x56\xea\x9a\x54\x9b\x65\x08\x1d"
      "\x9e\xef\x55\x5c\x77\x7e\x1d\x72\xe0\x3b\x14\x27\x4b\x4c\x2d\xd1\x8a\x14"
      "\x29\x18\xbb\x27\x61\x7e\x8d\x0d\x16\x7c\x68\xac\x56\x82\x85\x4e\x6d\x9b"
      "\xa1\x10\x7d\xb7\xe1\xc2\x47\x3a\x61\xc3\x80\xc5\x1e\x3e\xf4\x0a\x8f\x10"
      "\x11\x91\xf5\x58\x1b\x24\x24\xdc\x9d\x13\xb1\xad\x9c\x37\xe7\x10\x53\xfb"
      "\x35\x77\x48\x36\x24\xb7\x11\x0a\xdc\xa7\x54\x09\xc6\xa5\x55\x2e\x3b\xf6"
      "\xd2\x13\x35\x78\x76\x4c\xdf\xaf\x4e\x7b\xe9\xa0\x77\xa5\xd0\x22\x2f\x9d"
      "\xaf\x1b\xf1\x56\x4d\x00\x8b\x4d\x34\x7a\x28\xa2\x10\x92\xc7\xd5\x21\x2c"
      "\x87\x31\xa2\xe5\x70\x70\x86\x66\x81\x18\x22\x7c\xea\x77\x48\x21\x12\xbb"
      "\x72\x6a\xfa\x7e\xb5\xe9\x45\xc8\xda\x2b\x99\x68\x95\xac\xba\x9a\x41\x83"
      "\x7b\xbb\x35\x4d\x75\xc2\x06\xe8\x1f\xe6\x6d\xba\xc6\xb6\x9a\x27\x1c\xc0"
      "\xd8\x07\x3f\x07\x3d\xf1\xa5\xc0\xb4\xf3\x6c\x2d\x92\x50\xf1\x0e\xb3\xa0"
      "\x9d\xbb\x52\x5e\xfe\x06\x66\x2e\xb5\xdd\x85\xd6\xa5\x8d\x10\x07\xdd\xe5"
      "\xe0\x67\xb2\xf4\xfe\xc2\xe8\x3c\x85\xf9\xaf\x9f\x0c\x9b\xf3\x14\x2c\x9e"
      "\x36\xee\x63\x53\x5c\x01\xbb\x25\x50\x64\x14\x2a\x5e\x6c\xb5\xf9\xf1\xcc"
      "\x41\xdf\xe3\x20\xd0\xd3\x3a\x7f\x10\x61\x1f\xa5\xe7\xd1\x0d\x43\xdf\x18"
      "\x0c\xbb\xc0\xe7\x15\xdb\x4c\xab\x15\x29\x99\xcc\x15\xde\x7d\xa7\xae\x82"
      "\x65\x88\x75\xb2\xfc\xe7\x37\x98\x48\x9b\x49\x05\x25\x70\x95\x5a\x00\xd3"
      "\x35\xa3\x0c\x7d\x27\x3a\x66\xf6\x21\xd2\x29\x6c\x1a\xcb\xae\xe2\x38\xfb"
      "\x4f\x03\x9b\x64\xc5\x76\xa0\x42\x0a\x69\x0f\x0d\x4e\x75\x39\x17\x7e\xd9"
      "\xc4\x83\xc5\x92\x9d\xa2\x98\x58\x4b\xa8\x49\x83\xce\x4d\x9e\x98\x1b\x7b"
      "\x7d\xc2\x6c\xd3\x49\x86\x7d\x7e\x5c\xab\x95\x43\xa7\xe0\xb5\x31\x59\x24"
      "\x1d\xdc\xac\x65\xe5\xb1\x68\x25\x02\xd5\x20\x37\xd2\x87\xc0\x05\x2f\x49"
      "\x48\x34\xe2\x7a\x16\x70\x45\x31\x6e\x61\x1b\x90\x5e\xb9\x19\xf0\x73\x22"
      "\xe7\xff\xc9\xd5\x06\x2d\xef\x4c\xe9\x93\xf1\xf5\x47\xa3\xd8\xaa\x82\xba"
      "\x3f\x14\x85\xcd\x61\x8b\xab\xd4\xf1\xdb\x16\xea\x33\x29\xd7\x39\x04\x6d"
      "\x0b\x6d\x21\x48\xcc\xc6\xb2\xe0\xc3\x4b\x15\x59\x11\x22\xd3\x0c\x9a\x9a"
      "\x64\x31\x4f\xc4\xb5\xa4\x3b\xd4\x1d\x0f\x6d\x71\xf7\xb5\xc5\x9e\xb8\x88"
      "\x57\xaa\x49\xeb\x03\x8b\x28\x8f\x1b\x12\x2a\x4b\x07\x52\x0b\xac\x3d\x31"
      "\x1c\xe3\x3d\xef\xdc\xea\x7d\x02\xb8\x11\x1a\x96\xeb\x6c\xf2\x44\x2d\xf9"
      "\x63\xf7\xc8\x14\x08\x6b\xf3\x85\xbe\x71\x64\xd1\x4b\x83\xc7\xc1\xe3\x83"
      "\x9a\xf2\x95\x94\xad\x9e\x66\x4c\x76\x8d\xb2\x3c\x20\x0b\x41\x4f\x3c\x17"
      "\x7d\xef\xd7\xb4\x86\xe3\x0a\xf8\x17\xdd\x97\x58\xf1\xb5\x78\xdf\xbe\x75"
      "\x9c\xe7\xc5\x00\x24\x75\xe4\xb1\xd0\xcd\xc8\xc5\xe1\x27\x41\x3f\x43\x84"
      "\xca\x31\xe0\xc7\x52\xf2\x4e\x60\xcb\x3b\xa8\x13\x86\xae\xfc\xa4\xd2\xb0"
      "\x96\x9b\x4b\xa5\xbe\xd9\xba\xe9\xee\xc6\x29\xd6\xbd\x72\x9b\x89\x75\xef"
      "\x16\xa9\x88\x93\x3f\xd5\x7f\x9f\x09\x02\xcc\xc9\x47\x37\x53\x24\x98\x96"
      "\xf4\xa0\x5e\x7e\xa0\x2d\x2b\x04\xf0\x5c\x30\x92\xe6\x31\xc8\xc8\xdf\xdb"
      "\x29\x54\x26\xe5\x47\x93\x27\xb2\xa8\x2c\x42\xc2\x5c\x50\x07\xa6\xf0\x3d"
      "\xa8\x84\x2c\xc4\xa7\x1e\xde\xcd\xd2\x44\x7b\x25\x8a\xc3\x09\x7c\x1b\x4e"
      "\xf6\x62\x6a\x8d\x90\x6f\x6d\xd8\x30\x7b\xf7\x21\x78\xc5\x05\x24\x80\x25"
      "\x3a\x90\xba\x40\xe8\x4a\xeb\xdd\x59\x7a\xb6\xe4\x78\x9d\x8a\xb4\x07\x43"
      "\x7c\x20\x01\xe5\x49\x72\x40\xa0\x1c\x84\xf5\x2f\xca\xdf\xdc\xf2\x33\x7b"
      "\x87\xfd\x50\x32\xd5\xe3\x54\x99\x38\xe2\x2a\x5c\x96\xa6\x06\x3c\x9f\x81"
      "\x6f\x44\x19\x3c\xc9\xaa\xaa\xa3\xa0\x17\x29\xd5\xd2\xf3\x9e\x2b\x73\x30"
      "\x35\xd3\xb9\x17\xea\x35\x42\x9b\xbc\xe5\x94\x71\x11\x02\x8d\x9e\xe1\xc5"
      "\xc8\x49\xb2\x50\xcf\x1b\x6b\x53\x2c\x77\x54\x53\x73\x93\xa3\xd3\xff\x3f"
      "\x12\x24\x54\x03\xe9\x87\x54\x92\x7f\xfd\x0d\x44\x7c\xce\x47\x54\x09\xd5"
      "\xe0\x7a\x9b\xec\x4b\xda\x2a\x83\xe1\x9a\xaa\xc2\x99\x0a\xa4\xc4\x47\x11"
      "\x12\x7d\x03\xda\x93\xff\x6c\x3a\xc3\x5b\xa3\xab\x54\xb5\xce\x47\x96\x12"
      "\x7f\x1e\x5c\xab\xc5\xe5\x71\xd4\xb7\x38\x91\x9d\x48\x8f\x5f\x32\x25\x97"
      "\x70\xad\x98\x5f\x44\xd2\x99\x64\x46\x15\xfa\x09\xd6\x02\x6c\x54\xb4\x7a"
      "\x5f\xfe\x8d\x15\x72\x88\xeb\xbb\xf8\xd9\xe6\x29\x58\x7f\x95\x82\x95\x8d"
      "\x3e\x80\x1e\x34\xbd\x35\xf3\x95\x09\x60\xca\x26\x05\x07\x46\x21\xe3\xa5"
      "\x49\x88\x95\xf3\x2c\x75\x47\x75\x6c\x7c\x66\xee\xc0\xf8\xe7\xf7\x97\x53"
      "\x03\x99\xcd\x52\x0d\xff\xd0\x37\x4f\x22\x0a\x8a\xa0\x15\xfe\x24\x2b\xeb"
      "\xd6\xe8\x4a\x3c\x66\xda\x77\xbb\xd1\xab\xe2\x88\xc3\xfe\x18\x5a\x17\xa0"
      "\x25\x4c\x75\xeb\x81\x1c\x75\xa8\x2b\x5a\xab\x8f\xa6\xdd\x2f\x94\xb5\xe3"
      "\x1f\x23\xc8\x71\x91\x42\x27\x1d\x86\x2c\x43\xbb\x9d\x69\x71\x1c\x73\x68"
      "\xc8\x4b\x55\x60\x1f\x30\x26\xfd\xd3\x81\xdd\xa5\x85\x35\x43\xf3\x3c\x6a"
      "\xf1\x26\x10\xf9\x1f\xcf\x71\xfb\x57\x15\xa7\x56\x80\xf8\xa3\x61\xc6\x43"
      "\x43\x1d\x9a\xe5\x59\x1f\xc2\xc6\xd1\x31\xf1\xe4\xff\xab\xfe\xbd\x7f\xa7"
      "\x5f\x28\xcf\x2d\x37\x7e\x4e\xf0\xd9\x60\x3f\xbd\x2f\x2a\x27\xb9\xbd\x23"
      "\x3f\x86\xd7\x99\x8c\xb5\x83\x37\x46\x1d\xc8\x0f\x93\x0e\xb9\x62\x1e\xad"
      "\x30\x3b\x97\xd0\xef\x68\xac\xdf\x83\x33\xfb\xa6\xdf\x3e\x9d\xea\x49\x7c"
      "\xe5\xfd\x5e\xf6\xac\x0f\xe8\xe6\xf0\xb3\x23\xf4\x4f\x0e\x8d\x1f\x44\xe5"
      "\x16\x30\x90\x88\x8b\x30\xb8\x36\x88\x11\x44\x04\x8b\x96\xf0\x9b\x02\x91"
      "\xc9\x98\x2b\x2f\xcc\x55\xda\x54\xb0\x2d\xfa\x3b\x5d\xb1\x5a\x38\x47\xe8"
      "\x74\xe4\xed\x0a\xbc\xc7\x47\x43\xe4\xef\x39\xdb\x10\x60\xc2\x51\xac\x05"
      "\xfa\x9d\x8f\x38\x62\xbf\xca\xe9\xe8\xe3\xff\xe2\x7d\x7d\x97\x48\x8b\x27"
      "\xcc\x16\xf2\x0c\x9c\x99\x1c\x55\x37\x23\x5c\xe7\xb9\xfe\x2a\xff\x94\x9e"
      "\x51\x57\xef\xc6\x45\x7a\xcf\x4c\x0c\x3f\x83\xb1\x87\x0c\xdc\xe9\xe4\x60"
      "\xed\x3e\x53\xcd\x79\xb7\xad\xdd\x6d\x7d\x4f\x1c\x97\x38\x14\xd2\xa2\xd6"
      "\xb3\x93\xb0\xf2\x67\x43\x97\x1c\x06\x31\x99\xa2",
      4080);
  *(uint64_t*)0x20000088 = 0xff0;
  syscall(__NR_writev, r[0], 0x20000080, 1);
  *(uint8_t*)0x20000080 = 0xaa;
  *(uint8_t*)0x20000081 = 0xaa;
  *(uint8_t*)0x20000082 = 0xaa;
  *(uint8_t*)0x20000083 = 0xaa;
  *(uint8_t*)0x20000084 = 0xaa;
  *(uint8_t*)0x20000085 = 0xaa;
  *(uint8_t*)0x20000086 = -1;
  *(uint8_t*)0x20000087 = -1;
  *(uint8_t*)0x20000088 = -1;
  *(uint8_t*)0x20000089 = -1;
  *(uint8_t*)0x2000008a = -1;
  *(uint8_t*)0x2000008b = -1;
  *(uint16_t*)0x2000008c = htobe16(0x800);
  STORE_BY_BITMASK(uint8_t, 0x2000008e, 5, 0, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000008e, 4, 4, 4);
  STORE_BY_BITMASK(uint8_t, 0x2000008f, 0, 0, 2);
  STORE_BY_BITMASK(uint8_t, 0x2000008f, 0, 2, 6);
  *(uint16_t*)0x20000090 = htobe16(0x1c);
  *(uint16_t*)0x20000092 = htobe16(0);
  *(uint16_t*)0x20000094 = htobe16(0);
  *(uint8_t*)0x20000096 = 0;
  *(uint8_t*)0x20000097 = 0;
  *(uint16_t*)0x20000098 = 0;
  *(uint8_t*)0x2000009a = 0xac;
  *(uint8_t*)0x2000009b = 0x14;
  *(uint8_t*)0x2000009c = 0x14;
  *(uint8_t*)0x2000009d = 0;
  *(uint8_t*)0x2000009e = 0xac;
  *(uint8_t*)0x2000009f = 0x23;
  *(uint8_t*)0x200000a0 = 0x14;
  *(uint8_t*)0x200000a1 = 0xbb;
  *(uint16_t*)0x200000a2 = htobe16(0);
  *(uint16_t*)0x200000a4 = htobe16(0);
  *(uint16_t*)0x200000a6 = htobe16(8);
  *(uint16_t*)0x200000a8 = 0;
  *(uint32_t*)0x20000000 = 0;
  *(uint32_t*)0x20000004 = 0;
  *(uint32_t*)0x20000008 = 0;
  *(uint32_t*)0x2000000c = 0;
  *(uint32_t*)0x20000010 = 0;
  *(uint32_t*)0x20000014 = 0;
  struct csum_inet csum_1;
  csum_inet_init(&csum_1);
  csum_inet_update(&csum_1, (const uint8_t*)0x2000009a, 4);
  csum_inet_update(&csum_1, (const uint8_t*)0x2000009e, 4);
  uint16_t csum_1_chunk_2 = 0x1100;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_2, 2);
  uint16_t csum_1_chunk_3 = 0x800;
  csum_inet_update(&csum_1, (const uint8_t*)&csum_1_chunk_3, 2);
  csum_inet_update(&csum_1, (const uint8_t*)0x200000a2, 8);
  *(uint16_t*)0x200000a8 = csum_inet_digest(&csum_1);
  struct csum_inet csum_2;
  csum_inet_init(&csum_2);
  csum_inet_update(&csum_2, (const uint8_t*)0x2000008e, 20);
  *(uint16_t*)0x20000098 = csum_inet_digest(&csum_2);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  loop();
  return 0;
}
