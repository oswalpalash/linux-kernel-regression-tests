// https://syzkaller.appspot.com/bug?id=f7b8f028e05bc12dff977860c6bf28f79ced86d7
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <stdint.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[1];
void test()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000, 0x1d000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 2, 0);
  memcpy((void*)0x20019c48, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20019c68 = 0xe;
  *(uint32_t*)0x20019c6c = 4;
  *(uint32_t*)0x20019c70 = 0x358;
  *(uint32_t*)0x20019c74 = -1;
  *(uint32_t*)0x20019c78 = 0;
  *(uint32_t*)0x20019c7c = 0;
  *(uint32_t*)0x20019c80 = 0;
  *(uint32_t*)0x20019c84 = -1;
  *(uint32_t*)0x20019c88 = -1;
  *(uint32_t*)0x20019c8c = 0;
  *(uint32_t*)0x20019c90 = 0;
  *(uint32_t*)0x20019c94 = 0;
  *(uint32_t*)0x20019c98 = -1;
  *(uint32_t*)0x20019c9c = 4;
  *(uint64_t*)0x20019ca0 = 0x20012000;
  *(uint8_t*)0x20019ca8 = 0;
  *(uint8_t*)0x20019ca9 = 0;
  *(uint8_t*)0x20019caa = 0;
  *(uint8_t*)0x20019cab = 0;
  *(uint8_t*)0x20019cac = 0;
  *(uint8_t*)0x20019cad = 0;
  *(uint8_t*)0x20019cae = 0;
  *(uint8_t*)0x20019caf = 0;
  *(uint8_t*)0x20019cb0 = 0;
  *(uint8_t*)0x20019cb1 = 0;
  *(uint8_t*)0x20019cb2 = 0;
  *(uint8_t*)0x20019cb3 = 0;
  *(uint8_t*)0x20019cb4 = 0;
  *(uint8_t*)0x20019cb5 = 0;
  *(uint8_t*)0x20019cb6 = 0;
  *(uint8_t*)0x20019cb7 = 0;
  *(uint8_t*)0x20019cb8 = 0;
  *(uint8_t*)0x20019cb9 = 0;
  *(uint8_t*)0x20019cba = 0;
  *(uint8_t*)0x20019cbb = 0;
  *(uint8_t*)0x20019cbc = 0;
  *(uint8_t*)0x20019cbd = 0;
  *(uint8_t*)0x20019cbe = 0;
  *(uint8_t*)0x20019cbf = 0;
  *(uint8_t*)0x20019cc0 = 0;
  *(uint8_t*)0x20019cc1 = 0;
  *(uint8_t*)0x20019cc2 = 0;
  *(uint8_t*)0x20019cc3 = 0;
  *(uint8_t*)0x20019cc4 = 0;
  *(uint8_t*)0x20019cc5 = 0;
  *(uint8_t*)0x20019cc6 = 0;
  *(uint8_t*)0x20019cc7 = 0;
  *(uint8_t*)0x20019cc8 = 0;
  *(uint8_t*)0x20019cc9 = 0;
  *(uint8_t*)0x20019cca = 0;
  *(uint8_t*)0x20019ccb = 0;
  *(uint8_t*)0x20019ccc = 0;
  *(uint8_t*)0x20019ccd = 0;
  *(uint8_t*)0x20019cce = 0;
  *(uint8_t*)0x20019ccf = 0;
  *(uint8_t*)0x20019cd0 = 0;
  *(uint8_t*)0x20019cd1 = 0;
  *(uint8_t*)0x20019cd2 = 0;
  *(uint8_t*)0x20019cd3 = 0;
  *(uint8_t*)0x20019cd4 = 0;
  *(uint8_t*)0x20019cd5 = 0;
  *(uint8_t*)0x20019cd6 = 0;
  *(uint8_t*)0x20019cd7 = 0;
  *(uint8_t*)0x20019cd8 = 0;
  *(uint8_t*)0x20019cd9 = 0;
  *(uint8_t*)0x20019cda = 0;
  *(uint8_t*)0x20019cdb = 0;
  *(uint8_t*)0x20019cdc = 0;
  *(uint8_t*)0x20019cdd = 0;
  *(uint8_t*)0x20019cde = 0;
  *(uint8_t*)0x20019cdf = 0;
  *(uint8_t*)0x20019ce0 = 0;
  *(uint8_t*)0x20019ce1 = 0;
  *(uint8_t*)0x20019ce2 = 0;
  *(uint8_t*)0x20019ce3 = 0;
  *(uint8_t*)0x20019ce4 = 0;
  *(uint8_t*)0x20019ce5 = 0;
  *(uint8_t*)0x20019ce6 = 0;
  *(uint8_t*)0x20019ce7 = 0;
  *(uint8_t*)0x20019ce8 = 0;
  *(uint8_t*)0x20019ce9 = 0;
  *(uint8_t*)0x20019cea = 0;
  *(uint8_t*)0x20019ceb = 0;
  *(uint8_t*)0x20019cec = 0;
  *(uint8_t*)0x20019ced = 0;
  *(uint8_t*)0x20019cee = 0;
  *(uint8_t*)0x20019cef = 0;
  *(uint8_t*)0x20019cf0 = 0;
  *(uint8_t*)0x20019cf1 = 0;
  *(uint8_t*)0x20019cf2 = 0;
  *(uint8_t*)0x20019cf3 = 0;
  *(uint8_t*)0x20019cf4 = 0;
  *(uint8_t*)0x20019cf5 = 0;
  *(uint8_t*)0x20019cf6 = 0;
  *(uint8_t*)0x20019cf7 = 0;
  *(uint8_t*)0x20019cf8 = 0;
  *(uint8_t*)0x20019cf9 = 0;
  *(uint8_t*)0x20019cfa = 0;
  *(uint8_t*)0x20019cfb = 0;
  *(uint8_t*)0x20019cfc = 0;
  *(uint8_t*)0x20019cfd = 0;
  *(uint8_t*)0x20019cfe = 0;
  *(uint8_t*)0x20019cff = 0;
  *(uint8_t*)0x20019d00 = 0;
  *(uint8_t*)0x20019d01 = 0;
  *(uint8_t*)0x20019d02 = 0;
  *(uint8_t*)0x20019d03 = 0;
  *(uint8_t*)0x20019d04 = 0;
  *(uint8_t*)0x20019d05 = 0;
  *(uint8_t*)0x20019d06 = 0;
  *(uint8_t*)0x20019d07 = 0;
  *(uint8_t*)0x20019d08 = 0;
  *(uint8_t*)0x20019d09 = 0;
  *(uint8_t*)0x20019d0a = 0;
  *(uint8_t*)0x20019d0b = 0;
  *(uint8_t*)0x20019d0c = 0;
  *(uint8_t*)0x20019d0d = 0;
  *(uint8_t*)0x20019d0e = 0;
  *(uint8_t*)0x20019d0f = 0;
  *(uint8_t*)0x20019d10 = 0;
  *(uint8_t*)0x20019d11 = 0;
  *(uint8_t*)0x20019d12 = 0;
  *(uint8_t*)0x20019d13 = 0;
  *(uint8_t*)0x20019d14 = 0;
  *(uint8_t*)0x20019d15 = 0;
  *(uint8_t*)0x20019d16 = 0;
  *(uint8_t*)0x20019d17 = 0;
  *(uint8_t*)0x20019d18 = 0;
  *(uint8_t*)0x20019d19 = 0;
  *(uint8_t*)0x20019d1a = 0;
  *(uint8_t*)0x20019d1b = 0;
  *(uint8_t*)0x20019d1c = 0;
  *(uint8_t*)0x20019d1d = 0;
  *(uint8_t*)0x20019d1e = 0;
  *(uint8_t*)0x20019d1f = 0;
  *(uint8_t*)0x20019d20 = 0;
  *(uint8_t*)0x20019d21 = 0;
  *(uint8_t*)0x20019d22 = 0;
  *(uint8_t*)0x20019d23 = 0;
  *(uint8_t*)0x20019d24 = 0;
  *(uint8_t*)0x20019d25 = 0;
  *(uint8_t*)0x20019d26 = 0;
  *(uint8_t*)0x20019d27 = 0;
  *(uint8_t*)0x20019d28 = 0;
  *(uint8_t*)0x20019d29 = 0;
  *(uint8_t*)0x20019d2a = 0;
  *(uint8_t*)0x20019d2b = 0;
  *(uint8_t*)0x20019d2c = 0;
  *(uint8_t*)0x20019d2d = 0;
  *(uint8_t*)0x20019d2e = 0;
  *(uint8_t*)0x20019d2f = 0;
  *(uint32_t*)0x20019d30 = 0;
  *(uint16_t*)0x20019d34 = 0xa8;
  *(uint16_t*)0x20019d36 = 0xd0;
  *(uint32_t*)0x20019d38 = 0;
  *(uint64_t*)0x20019d40 = 0;
  *(uint64_t*)0x20019d48 = 0;
  *(uint16_t*)0x20019d50 = 0x28;
  memcpy((void*)0x20019d52, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20019d6f = 0;
  *(uint32_t*)0x20019d70 = 0xfffffffe;
  *(uint8_t*)0x20019d78 = 0;
  *(uint8_t*)0x20019d79 = 0;
  *(uint8_t*)0x20019d7a = 0;
  *(uint8_t*)0x20019d7b = 0;
  *(uint8_t*)0x20019d7c = 0;
  *(uint8_t*)0x20019d7d = 0;
  *(uint8_t*)0x20019d7e = 0;
  *(uint8_t*)0x20019d7f = 0;
  *(uint8_t*)0x20019d80 = 0;
  *(uint8_t*)0x20019d81 = 0;
  *(uint8_t*)0x20019d82 = 0;
  *(uint8_t*)0x20019d83 = 0;
  *(uint8_t*)0x20019d84 = 0;
  *(uint8_t*)0x20019d85 = 0;
  *(uint8_t*)0x20019d86 = 0;
  *(uint8_t*)0x20019d87 = 0;
  *(uint8_t*)0x20019d88 = 0;
  *(uint8_t*)0x20019d89 = 0;
  *(uint8_t*)0x20019d8a = 0;
  *(uint8_t*)0x20019d8b = 0;
  *(uint8_t*)0x20019d8c = 0;
  *(uint8_t*)0x20019d8d = 0;
  *(uint8_t*)0x20019d8e = 0;
  *(uint8_t*)0x20019d8f = 0;
  *(uint8_t*)0x20019d90 = 0;
  *(uint8_t*)0x20019d91 = 0;
  *(uint8_t*)0x20019d92 = 0;
  *(uint8_t*)0x20019d93 = 0;
  *(uint8_t*)0x20019d94 = 0;
  *(uint8_t*)0x20019d95 = 0;
  *(uint8_t*)0x20019d96 = 0;
  *(uint8_t*)0x20019d97 = 0;
  *(uint8_t*)0x20019d98 = 0;
  *(uint8_t*)0x20019d99 = 0;
  *(uint8_t*)0x20019d9a = 0;
  *(uint8_t*)0x20019d9b = 0;
  *(uint8_t*)0x20019d9c = 0;
  *(uint8_t*)0x20019d9d = 0;
  *(uint8_t*)0x20019d9e = 0;
  *(uint8_t*)0x20019d9f = 0;
  *(uint8_t*)0x20019da0 = 0;
  *(uint8_t*)0x20019da1 = 0;
  *(uint8_t*)0x20019da2 = 0;
  *(uint8_t*)0x20019da3 = 0;
  *(uint8_t*)0x20019da4 = 0;
  *(uint8_t*)0x20019da5 = 0;
  *(uint8_t*)0x20019da6 = 0;
  *(uint8_t*)0x20019da7 = 0;
  *(uint8_t*)0x20019da8 = 0;
  *(uint8_t*)0x20019da9 = 0;
  *(uint8_t*)0x20019daa = 0;
  *(uint8_t*)0x20019dab = 0;
  *(uint8_t*)0x20019dac = 0;
  *(uint8_t*)0x20019dad = 0;
  *(uint8_t*)0x20019dae = 0;
  *(uint8_t*)0x20019daf = 0;
  *(uint8_t*)0x20019db0 = 0;
  *(uint8_t*)0x20019db1 = 0;
  *(uint8_t*)0x20019db2 = 0;
  *(uint8_t*)0x20019db3 = 0;
  *(uint8_t*)0x20019db4 = 0;
  *(uint8_t*)0x20019db5 = 0;
  *(uint8_t*)0x20019db6 = 0;
  *(uint8_t*)0x20019db7 = 0;
  *(uint8_t*)0x20019db8 = 0;
  *(uint8_t*)0x20019db9 = 0;
  *(uint8_t*)0x20019dba = 0;
  *(uint8_t*)0x20019dbb = 0;
  *(uint8_t*)0x20019dbc = 0;
  *(uint8_t*)0x20019dbd = 0;
  *(uint8_t*)0x20019dbe = 0;
  *(uint8_t*)0x20019dbf = 0;
  *(uint8_t*)0x20019dc0 = 0;
  *(uint8_t*)0x20019dc1 = 0;
  *(uint8_t*)0x20019dc2 = 0;
  *(uint8_t*)0x20019dc3 = 0;
  *(uint8_t*)0x20019dc4 = 0xe4;
  *(uint8_t*)0x20019dc5 = 0;
  *(uint8_t*)0x20019dc6 = 0;
  *(uint8_t*)0x20019dc7 = 0;
  *(uint8_t*)0x20019dc8 = 0;
  *(uint8_t*)0x20019dc9 = 0;
  *(uint8_t*)0x20019dca = 0;
  *(uint8_t*)0x20019dcb = 0;
  *(uint8_t*)0x20019dcc = 0;
  *(uint8_t*)0x20019dcd = 0;
  *(uint8_t*)0x20019dce = 0;
  *(uint8_t*)0x20019dcf = 0;
  *(uint8_t*)0x20019dd0 = 0;
  *(uint8_t*)0x20019dd1 = 0;
  *(uint8_t*)0x20019dd2 = 0;
  *(uint8_t*)0x20019dd3 = 0;
  *(uint8_t*)0x20019dd4 = 0;
  *(uint8_t*)0x20019dd5 = 0;
  *(uint8_t*)0x20019dd6 = 0;
  *(uint8_t*)0x20019dd7 = 0;
  *(uint8_t*)0x20019dd8 = 0;
  *(uint8_t*)0x20019dd9 = 0;
  *(uint8_t*)0x20019dda = 0;
  *(uint8_t*)0x20019ddb = 0;
  *(uint8_t*)0x20019ddc = 0;
  *(uint8_t*)0x20019ddd = 0;
  *(uint8_t*)0x20019dde = 0;
  *(uint8_t*)0x20019ddf = 0;
  *(uint8_t*)0x20019de0 = 0;
  *(uint8_t*)0x20019de1 = 0;
  *(uint8_t*)0x20019de2 = 0;
  *(uint8_t*)0x20019de3 = 0;
  *(uint8_t*)0x20019de4 = 0;
  *(uint8_t*)0x20019de5 = 0;
  *(uint8_t*)0x20019de6 = 0;
  *(uint8_t*)0x20019de7 = 0;
  *(uint8_t*)0x20019de8 = 0;
  *(uint8_t*)0x20019de9 = 0;
  *(uint8_t*)0x20019dea = 0;
  *(uint8_t*)0x20019deb = 0;
  *(uint8_t*)0x20019dec = 0;
  *(uint8_t*)0x20019ded = 0;
  *(uint8_t*)0x20019dee = 0;
  *(uint8_t*)0x20019def = 0;
  *(uint8_t*)0x20019df0 = 0;
  *(uint8_t*)0x20019df1 = 0;
  *(uint8_t*)0x20019df2 = 0;
  *(uint8_t*)0x20019df3 = 0;
  *(uint8_t*)0x20019df4 = 0;
  *(uint8_t*)0x20019df5 = 0;
  *(uint8_t*)0x20019df6 = 0;
  *(uint8_t*)0x20019df7 = 0;
  *(uint8_t*)0x20019df8 = 0;
  *(uint8_t*)0x20019df9 = 0;
  *(uint8_t*)0x20019dfa = 0;
  *(uint8_t*)0x20019dfb = 0;
  *(uint8_t*)0x20019dfc = 0;
  *(uint8_t*)0x20019dfd = 0;
  *(uint8_t*)0x20019dfe = 0;
  *(uint8_t*)0x20019dff = 0;
  *(uint32_t*)0x20019e00 = 0;
  *(uint16_t*)0x20019e04 = 0xa8;
  *(uint16_t*)0x20019e06 = 0xd0;
  *(uint32_t*)0x20019e08 = 0;
  *(uint64_t*)0x20019e0c = 0;
  *(uint64_t*)0x20019e14 = 0;
  *(uint16_t*)0x20019e20 = 0x28;
  memcpy((void*)0x20019e22, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20019e3f = 0;
  *(uint32_t*)0x20019e40 = 0;
  *(uint8_t*)0x20019e48 = 0xfe;
  *(uint8_t*)0x20019e49 = 0x80;
  *(uint8_t*)0x20019e4a = 0;
  *(uint8_t*)0x20019e4b = 0;
  *(uint8_t*)0x20019e4c = 0;
  *(uint8_t*)0x20019e4d = 0;
  *(uint8_t*)0x20019e4e = 0;
  *(uint8_t*)0x20019e4f = 0;
  *(uint8_t*)0x20019e50 = 0;
  *(uint8_t*)0x20019e51 = 0;
  *(uint8_t*)0x20019e52 = 0;
  *(uint8_t*)0x20019e53 = 0;
  *(uint8_t*)0x20019e54 = 0;
  *(uint8_t*)0x20019e55 = 0;
  *(uint8_t*)0x20019e56 = 0;
  *(uint8_t*)0x20019e57 = 0xbb;
  *(uint8_t*)0x20019e58 = 0;
  *(uint8_t*)0x20019e59 = 0;
  *(uint8_t*)0x20019e5a = 0;
  *(uint8_t*)0x20019e5b = 0;
  *(uint8_t*)0x20019e5c = 0;
  *(uint8_t*)0x20019e5d = 0;
  *(uint8_t*)0x20019e5e = 0;
  *(uint8_t*)0x20019e5f = 0;
  *(uint8_t*)0x20019e60 = 0;
  *(uint8_t*)0x20019e61 = 0;
  *(uint8_t*)0x20019e62 = 0;
  *(uint8_t*)0x20019e63 = 0;
  *(uint8_t*)0x20019e64 = 0;
  *(uint8_t*)0x20019e65 = 0;
  *(uint8_t*)0x20019e66 = 0;
  *(uint8_t*)0x20019e67 = 0;
  *(uint32_t*)0x20019e68 = htobe32(0);
  *(uint32_t*)0x20019e6c = htobe32(0);
  *(uint32_t*)0x20019e70 = htobe32(0);
  *(uint32_t*)0x20019e74 = htobe32(0);
  *(uint32_t*)0x20019e78 = htobe32(0);
  *(uint32_t*)0x20019e7c = htobe32(0);
  *(uint32_t*)0x20019e80 = htobe32(0);
  *(uint32_t*)0x20019e84 = htobe32(0);
  memcpy((void*)0x20019e88,
         "\x28\x44\x63\x31\x60\x5b\xfe\xfb\x40\xec\x55\xd8\x9c\xce\xfc\xf6",
         16);
  memcpy((void*)0x20019e98,
         "\x21\xbc\xf3\x5d\x5f\x3f\x02\xf3\x97\xf4\x12\x52\x77\xfa\xaf\x31",
         16);
  *(uint8_t*)0x20019ea8 = 0;
  *(uint8_t*)0x20019ea9 = 0;
  *(uint8_t*)0x20019eaa = 0;
  *(uint8_t*)0x20019eab = 0;
  *(uint8_t*)0x20019eac = 0;
  *(uint8_t*)0x20019ead = 0;
  *(uint8_t*)0x20019eae = 0;
  *(uint8_t*)0x20019eaf = 0;
  *(uint8_t*)0x20019eb0 = 0;
  *(uint8_t*)0x20019eb1 = 0;
  *(uint8_t*)0x20019eb2 = 0;
  *(uint8_t*)0x20019eb3 = 0;
  *(uint8_t*)0x20019eb4 = 0;
  *(uint8_t*)0x20019eb5 = 0;
  *(uint8_t*)0x20019eb6 = 0;
  *(uint8_t*)0x20019eb7 = 0;
  *(uint8_t*)0x20019eb8 = 0;
  *(uint8_t*)0x20019eb9 = 0;
  *(uint8_t*)0x20019eba = 0;
  *(uint8_t*)0x20019ebb = 0;
  *(uint8_t*)0x20019ebc = 0;
  *(uint8_t*)0x20019ebd = 0;
  *(uint8_t*)0x20019ebe = 0;
  *(uint8_t*)0x20019ebf = 0;
  *(uint8_t*)0x20019ec0 = 0;
  *(uint8_t*)0x20019ec1 = 0;
  *(uint8_t*)0x20019ec2 = 0;
  *(uint8_t*)0x20019ec3 = 0;
  *(uint8_t*)0x20019ec4 = 0;
  *(uint8_t*)0x20019ec5 = 0;
  *(uint8_t*)0x20019ec6 = 0;
  *(uint8_t*)0x20019ec7 = 0;
  *(uint16_t*)0x20019ec8 = 0;
  *(uint8_t*)0x20019eca = 0;
  *(uint8_t*)0x20019ecb = 0;
  *(uint8_t*)0x20019ecc = 0;
  *(uint32_t*)0x20019ed0 = 0;
  *(uint16_t*)0x20019ed4 = 0xa8;
  *(uint16_t*)0x20019ed6 = 0xe8;
  *(uint32_t*)0x20019ed8 = 0;
  *(uint64_t*)0x20019edc = 0;
  *(uint64_t*)0x20019ee4 = 0;
  *(uint16_t*)0x20019ef0 = 0x40;
  memcpy((void*)0x20019ef2, "\x52\x41\x54\x45\x45\x53\x54\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20019f0f = 0;
  memcpy((void*)0x20019f10,
         "\x73\x79\x7a\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x20019f20 = 0;
  *(uint8_t*)0x20019f21 = 0x80;
  *(uint64_t*)0x20019f28 = 0;
  *(uint8_t*)0x20019f30 = 0;
  *(uint8_t*)0x20019f31 = 0;
  *(uint8_t*)0x20019f32 = 0;
  *(uint8_t*)0x20019f33 = 0;
  *(uint8_t*)0x20019f34 = 0;
  *(uint8_t*)0x20019f35 = 0;
  *(uint8_t*)0x20019f36 = 0;
  *(uint8_t*)0x20019f37 = 0;
  *(uint8_t*)0x20019f38 = 0;
  *(uint8_t*)0x20019f39 = 0;
  *(uint8_t*)0x20019f3a = 0;
  *(uint8_t*)0x20019f3b = 0;
  *(uint8_t*)0x20019f3c = 0;
  *(uint8_t*)0x20019f3d = 0;
  *(uint8_t*)0x20019f3e = 0;
  *(uint8_t*)0x20019f3f = 0;
  *(uint8_t*)0x20019f40 = 0;
  *(uint8_t*)0x20019f41 = 0;
  *(uint8_t*)0x20019f42 = 0;
  *(uint8_t*)0x20019f43 = 0;
  *(uint8_t*)0x20019f44 = 0;
  *(uint8_t*)0x20019f45 = 0;
  *(uint8_t*)0x20019f46 = 0;
  *(uint8_t*)0x20019f47 = 0;
  *(uint8_t*)0x20019f48 = 0;
  *(uint8_t*)0x20019f49 = 0;
  *(uint8_t*)0x20019f4a = 0;
  *(uint8_t*)0x20019f4b = 0;
  *(uint8_t*)0x20019f4c = 0;
  *(uint8_t*)0x20019f4d = 0;
  *(uint8_t*)0x20019f4e = 0;
  *(uint8_t*)0x20019f4f = 0;
  *(uint8_t*)0x20019f50 = 0;
  *(uint8_t*)0x20019f51 = 0;
  *(uint8_t*)0x20019f52 = 0;
  *(uint8_t*)0x20019f53 = 0;
  *(uint8_t*)0x20019f54 = 0;
  *(uint8_t*)0x20019f55 = 0;
  *(uint8_t*)0x20019f56 = 0;
  *(uint8_t*)0x20019f57 = 0;
  *(uint8_t*)0x20019f58 = 0;
  *(uint8_t*)0x20019f59 = 0;
  *(uint8_t*)0x20019f5a = 0;
  *(uint8_t*)0x20019f5b = 0;
  *(uint8_t*)0x20019f5c = 0;
  *(uint8_t*)0x20019f5d = 0;
  *(uint8_t*)0x20019f5e = 0;
  *(uint8_t*)0x20019f5f = 0;
  *(uint8_t*)0x20019f60 = 0;
  *(uint8_t*)0x20019f61 = 0;
  *(uint8_t*)0x20019f62 = 0;
  *(uint8_t*)0x20019f63 = 0;
  *(uint8_t*)0x20019f64 = 0;
  *(uint8_t*)0x20019f65 = 0;
  *(uint8_t*)0x20019f66 = 0;
  *(uint8_t*)0x20019f67 = 0;
  *(uint8_t*)0x20019f68 = 0;
  *(uint8_t*)0x20019f69 = 0;
  *(uint8_t*)0x20019f6a = 0;
  *(uint8_t*)0x20019f6b = 0;
  *(uint8_t*)0x20019f6c = 0;
  *(uint8_t*)0x20019f6d = 0;
  *(uint8_t*)0x20019f6e = 0;
  *(uint8_t*)0x20019f6f = 0;
  *(uint8_t*)0x20019f70 = 0;
  *(uint8_t*)0x20019f71 = 0;
  *(uint8_t*)0x20019f72 = 0;
  *(uint8_t*)0x20019f73 = 0;
  *(uint8_t*)0x20019f74 = 0;
  *(uint8_t*)0x20019f75 = 0;
  *(uint8_t*)0x20019f76 = 0;
  *(uint8_t*)0x20019f77 = 0;
  *(uint8_t*)0x20019f78 = 0;
  *(uint8_t*)0x20019f79 = 0;
  *(uint8_t*)0x20019f7a = 0;
  *(uint8_t*)0x20019f7b = 0;
  *(uint8_t*)0x20019f7c = 0;
  *(uint8_t*)0x20019f7d = 0;
  *(uint8_t*)0x20019f7e = 0;
  *(uint8_t*)0x20019f7f = 0;
  *(uint8_t*)0x20019f80 = 0;
  *(uint8_t*)0x20019f81 = 0;
  *(uint8_t*)0x20019f82 = 0;
  *(uint8_t*)0x20019f83 = 0;
  *(uint8_t*)0x20019f84 = 0;
  *(uint8_t*)0x20019f85 = 0;
  *(uint8_t*)0x20019f86 = 0;
  *(uint8_t*)0x20019f87 = 0;
  *(uint8_t*)0x20019f88 = 0;
  *(uint8_t*)0x20019f89 = 0;
  *(uint8_t*)0x20019f8a = 0;
  *(uint8_t*)0x20019f8b = 0;
  *(uint8_t*)0x20019f8c = 0;
  *(uint8_t*)0x20019f8d = 0;
  *(uint8_t*)0x20019f8e = 0;
  *(uint8_t*)0x20019f8f = 0;
  *(uint8_t*)0x20019f90 = 0;
  *(uint8_t*)0x20019f91 = 0;
  *(uint8_t*)0x20019f92 = 0;
  *(uint8_t*)0x20019f93 = 0;
  *(uint8_t*)0x20019f94 = 0;
  *(uint8_t*)0x20019f95 = 0;
  *(uint8_t*)0x20019f96 = 0;
  *(uint8_t*)0x20019f97 = 0;
  *(uint8_t*)0x20019f98 = 0;
  *(uint8_t*)0x20019f99 = 0;
  *(uint8_t*)0x20019f9a = 0;
  *(uint8_t*)0x20019f9b = 0;
  *(uint8_t*)0x20019f9c = 0;
  *(uint8_t*)0x20019f9d = 0;
  *(uint8_t*)0x20019f9e = 0;
  *(uint8_t*)0x20019f9f = 0;
  *(uint8_t*)0x20019fa0 = 0;
  *(uint8_t*)0x20019fa1 = 0;
  *(uint8_t*)0x20019fa2 = 0;
  *(uint8_t*)0x20019fa3 = 0;
  *(uint8_t*)0x20019fa4 = 0;
  *(uint8_t*)0x20019fa5 = 0;
  *(uint8_t*)0x20019fa6 = 0;
  *(uint8_t*)0x20019fa7 = 0;
  *(uint8_t*)0x20019fa8 = 0;
  *(uint8_t*)0x20019fa9 = 0;
  *(uint8_t*)0x20019faa = 0;
  *(uint8_t*)0x20019fab = 0;
  *(uint8_t*)0x20019fac = 0;
  *(uint8_t*)0x20019fad = 0;
  *(uint8_t*)0x20019fae = 0;
  *(uint8_t*)0x20019faf = 0;
  *(uint8_t*)0x20019fb0 = 0;
  *(uint8_t*)0x20019fb1 = 0;
  *(uint8_t*)0x20019fb2 = 0;
  *(uint8_t*)0x20019fb3 = 0;
  *(uint8_t*)0x20019fb4 = 0;
  *(uint8_t*)0x20019fb5 = 0;
  *(uint8_t*)0x20019fb6 = 0;
  *(uint8_t*)0x20019fb7 = 0;
  *(uint32_t*)0x20019fb8 = 0;
  *(uint16_t*)0x20019fbc = 0xa8;
  *(uint16_t*)0x20019fbe = 0xd0;
  *(uint32_t*)0x20019fc0 = 0;
  *(uint64_t*)0x20019fc4 = 0;
  *(uint64_t*)0x20019fcc = 0;
  *(uint16_t*)0x20019fd8 = 0x28;
  memcpy((void*)0x20019fda, "\x41\x55\x44\x49\x54\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x20019ff7 = 0;
  *(uint8_t*)0x20019ff8 = 0;
  *(uint64_t*)0x20012000 = 0;
  *(uint64_t*)0x20012008 = 0;
  *(uint64_t*)0x20012010 = 0;
  *(uint64_t*)0x20012018 = 0;
  *(uint64_t*)0x20012020 = 0;
  *(uint64_t*)0x20012028 = 0;
  *(uint64_t*)0x20012030 = 0;
  *(uint64_t*)0x20012038 = 0;
  syscall(__NR_setsockopt, r[0], 0x29, 0x40, 0x20019c48, 0x3b8);
}

int main()
{
  for (;;) {
    loop();
  }
}
