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
  syscall(__NR_mmap, 0x20000000, 0x1e000, 3, 0x32, -1, 0);
  r[0] = syscall(__NR_socket, 0xa, 2, 0);
  memcpy((void*)0x2001dc40, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x2001dc60 = 0xe;
  *(uint32_t*)0x2001dc64 = 4;
  *(uint32_t*)0x2001dc68 = 0x358;
  *(uint32_t*)0x2001dc6c = -1;
  *(uint32_t*)0x2001dc70 = 0;
  *(uint32_t*)0x2001dc74 = 0;
  *(uint32_t*)0x2001dc78 = 0;
  *(uint32_t*)0x2001dc7c = -1;
  *(uint32_t*)0x2001dc80 = -1;
  *(uint32_t*)0x2001dc84 = 0;
  *(uint32_t*)0x2001dc88 = 0;
  *(uint32_t*)0x2001dc8c = 0;
  *(uint32_t*)0x2001dc90 = -1;
  *(uint32_t*)0x2001dc94 = 4;
  *(uint64_t*)0x2001dc98 = 0x20012000;
  *(uint8_t*)0x2001dca0 = 0;
  *(uint8_t*)0x2001dca1 = 0;
  *(uint8_t*)0x2001dca2 = 0;
  *(uint8_t*)0x2001dca3 = 0;
  *(uint8_t*)0x2001dca4 = 0;
  *(uint8_t*)0x2001dca5 = 0;
  *(uint8_t*)0x2001dca6 = 0;
  *(uint8_t*)0x2001dca7 = 0;
  *(uint8_t*)0x2001dca8 = 0;
  *(uint8_t*)0x2001dca9 = 0;
  *(uint8_t*)0x2001dcaa = 0;
  *(uint8_t*)0x2001dcab = 0;
  *(uint8_t*)0x2001dcac = 0;
  *(uint8_t*)0x2001dcad = 0;
  *(uint8_t*)0x2001dcae = 0;
  *(uint8_t*)0x2001dcaf = 0;
  *(uint8_t*)0x2001dcb0 = 0;
  *(uint8_t*)0x2001dcb1 = 0;
  *(uint8_t*)0x2001dcb2 = 0;
  *(uint8_t*)0x2001dcb3 = 0;
  *(uint8_t*)0x2001dcb4 = 0;
  *(uint8_t*)0x2001dcb5 = 0;
  *(uint8_t*)0x2001dcb6 = 0;
  *(uint8_t*)0x2001dcb7 = 0;
  *(uint8_t*)0x2001dcb8 = 0;
  *(uint8_t*)0x2001dcb9 = 0;
  *(uint8_t*)0x2001dcba = 0;
  *(uint8_t*)0x2001dcbb = 0;
  *(uint8_t*)0x2001dcbc = 0;
  *(uint8_t*)0x2001dcbd = 0;
  *(uint8_t*)0x2001dcbe = 0;
  *(uint8_t*)0x2001dcbf = 0;
  *(uint8_t*)0x2001dcc0 = 0;
  *(uint8_t*)0x2001dcc1 = 0;
  *(uint8_t*)0x2001dcc2 = 0;
  *(uint8_t*)0x2001dcc3 = 0;
  *(uint8_t*)0x2001dcc4 = 0;
  *(uint8_t*)0x2001dcc5 = 0;
  *(uint8_t*)0x2001dcc6 = 0;
  *(uint8_t*)0x2001dcc7 = 0;
  *(uint8_t*)0x2001dcc8 = 0;
  *(uint8_t*)0x2001dcc9 = 0;
  *(uint8_t*)0x2001dcca = 0;
  *(uint8_t*)0x2001dccb = 0;
  *(uint8_t*)0x2001dccc = 0;
  *(uint8_t*)0x2001dccd = 0;
  *(uint8_t*)0x2001dcce = 0;
  *(uint8_t*)0x2001dccf = 0;
  *(uint8_t*)0x2001dcd0 = 0;
  *(uint8_t*)0x2001dcd1 = 0;
  *(uint8_t*)0x2001dcd2 = 0;
  *(uint8_t*)0x2001dcd3 = 0;
  *(uint8_t*)0x2001dcd4 = 0;
  *(uint8_t*)0x2001dcd5 = 0;
  *(uint8_t*)0x2001dcd6 = 0;
  *(uint8_t*)0x2001dcd7 = 0;
  *(uint8_t*)0x2001dcd8 = 0;
  *(uint8_t*)0x2001dcd9 = 0;
  *(uint8_t*)0x2001dcda = 0;
  *(uint8_t*)0x2001dcdb = 0;
  *(uint8_t*)0x2001dcdc = 0;
  *(uint8_t*)0x2001dcdd = 0;
  *(uint8_t*)0x2001dcde = 0;
  *(uint8_t*)0x2001dcdf = 0;
  *(uint8_t*)0x2001dce0 = 0;
  *(uint8_t*)0x2001dce1 = 0;
  *(uint8_t*)0x2001dce2 = 0;
  *(uint8_t*)0x2001dce3 = 0;
  *(uint8_t*)0x2001dce4 = 0;
  *(uint8_t*)0x2001dce5 = 0;
  *(uint8_t*)0x2001dce6 = 0;
  *(uint8_t*)0x2001dce7 = 0;
  *(uint8_t*)0x2001dce8 = 0;
  *(uint8_t*)0x2001dce9 = 0;
  *(uint8_t*)0x2001dcea = 0;
  *(uint8_t*)0x2001dceb = 0;
  *(uint8_t*)0x2001dcec = 0;
  *(uint8_t*)0x2001dced = 0;
  *(uint8_t*)0x2001dcee = 0;
  *(uint8_t*)0x2001dcef = 0;
  *(uint8_t*)0x2001dcf0 = 0;
  *(uint8_t*)0x2001dcf1 = 0;
  *(uint8_t*)0x2001dcf2 = 0;
  *(uint8_t*)0x2001dcf3 = 0;
  *(uint8_t*)0x2001dcf4 = 0;
  *(uint8_t*)0x2001dcf5 = 0;
  *(uint8_t*)0x2001dcf6 = 0;
  *(uint8_t*)0x2001dcf7 = 0;
  *(uint8_t*)0x2001dcf8 = 0;
  *(uint8_t*)0x2001dcf9 = 0;
  *(uint8_t*)0x2001dcfa = 0;
  *(uint8_t*)0x2001dcfb = 0;
  *(uint8_t*)0x2001dcfc = 0;
  *(uint8_t*)0x2001dcfd = 0;
  *(uint8_t*)0x2001dcfe = 0;
  *(uint8_t*)0x2001dcff = 0;
  *(uint8_t*)0x2001dd00 = 0;
  *(uint8_t*)0x2001dd01 = 0;
  *(uint8_t*)0x2001dd02 = 0;
  *(uint8_t*)0x2001dd03 = 0;
  *(uint8_t*)0x2001dd04 = 0;
  *(uint8_t*)0x2001dd05 = 0;
  *(uint8_t*)0x2001dd06 = 0;
  *(uint8_t*)0x2001dd07 = 0;
  *(uint8_t*)0x2001dd08 = 0;
  *(uint8_t*)0x2001dd09 = 0;
  *(uint8_t*)0x2001dd0a = 0;
  *(uint8_t*)0x2001dd0b = 0;
  *(uint8_t*)0x2001dd0c = 0;
  *(uint8_t*)0x2001dd0d = 0;
  *(uint8_t*)0x2001dd0e = 0;
  *(uint8_t*)0x2001dd0f = 0;
  *(uint8_t*)0x2001dd10 = 0;
  *(uint8_t*)0x2001dd11 = 0;
  *(uint8_t*)0x2001dd12 = 0;
  *(uint8_t*)0x2001dd13 = 0;
  *(uint8_t*)0x2001dd14 = 0;
  *(uint8_t*)0x2001dd15 = 0;
  *(uint8_t*)0x2001dd16 = 0;
  *(uint8_t*)0x2001dd17 = 0;
  *(uint8_t*)0x2001dd18 = 0;
  *(uint8_t*)0x2001dd19 = 0;
  *(uint8_t*)0x2001dd1a = 0;
  *(uint8_t*)0x2001dd1b = 0;
  *(uint8_t*)0x2001dd1c = 0;
  *(uint8_t*)0x2001dd1d = 0;
  *(uint8_t*)0x2001dd1e = 0;
  *(uint8_t*)0x2001dd1f = 0;
  *(uint8_t*)0x2001dd20 = 0;
  *(uint8_t*)0x2001dd21 = 0;
  *(uint8_t*)0x2001dd22 = 0;
  *(uint8_t*)0x2001dd23 = 0;
  *(uint8_t*)0x2001dd24 = 0;
  *(uint8_t*)0x2001dd25 = 0;
  *(uint8_t*)0x2001dd26 = 0;
  *(uint8_t*)0x2001dd27 = 0;
  *(uint32_t*)0x2001dd28 = 0;
  *(uint16_t*)0x2001dd2c = 0xa8;
  *(uint16_t*)0x2001dd2e = 0xd0;
  *(uint32_t*)0x2001dd30 = 0;
  *(uint64_t*)0x2001dd38 = 0;
  *(uint64_t*)0x2001dd40 = 0;
  *(uint16_t*)0x2001dd48 = 0x28;
  memcpy((void*)0x2001dd4a, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2001dd67 = 0;
  *(uint32_t*)0x2001dd68 = 0xfffffffe;
  *(uint8_t*)0x2001dd70 = 0;
  *(uint8_t*)0x2001dd71 = 0;
  *(uint8_t*)0x2001dd72 = 0;
  *(uint8_t*)0x2001dd73 = 0;
  *(uint8_t*)0x2001dd74 = 0;
  *(uint8_t*)0x2001dd75 = 0;
  *(uint8_t*)0x2001dd76 = 0;
  *(uint8_t*)0x2001dd77 = 0;
  *(uint8_t*)0x2001dd78 = 0;
  *(uint8_t*)0x2001dd79 = 0;
  *(uint8_t*)0x2001dd7a = 0;
  *(uint8_t*)0x2001dd7b = 0;
  *(uint8_t*)0x2001dd7c = 0;
  *(uint8_t*)0x2001dd7d = 0;
  *(uint8_t*)0x2001dd7e = 0;
  *(uint8_t*)0x2001dd7f = 0;
  *(uint8_t*)0x2001dd80 = 0;
  *(uint8_t*)0x2001dd81 = 0;
  *(uint8_t*)0x2001dd82 = 0;
  *(uint8_t*)0x2001dd83 = 0;
  *(uint8_t*)0x2001dd84 = 0;
  *(uint8_t*)0x2001dd85 = 0;
  *(uint8_t*)0x2001dd86 = 0;
  *(uint8_t*)0x2001dd87 = 0;
  *(uint8_t*)0x2001dd88 = 0;
  *(uint8_t*)0x2001dd89 = 0;
  *(uint8_t*)0x2001dd8a = 0;
  *(uint8_t*)0x2001dd8b = 0;
  *(uint8_t*)0x2001dd8c = 0;
  *(uint8_t*)0x2001dd8d = 0;
  *(uint8_t*)0x2001dd8e = 0;
  *(uint8_t*)0x2001dd8f = 0;
  *(uint8_t*)0x2001dd90 = 0;
  *(uint8_t*)0x2001dd91 = 0;
  *(uint8_t*)0x2001dd92 = 0;
  *(uint8_t*)0x2001dd93 = 0;
  *(uint8_t*)0x2001dd94 = 0;
  *(uint8_t*)0x2001dd95 = 0;
  *(uint8_t*)0x2001dd96 = 0;
  *(uint8_t*)0x2001dd97 = 0;
  *(uint8_t*)0x2001dd98 = 0;
  *(uint8_t*)0x2001dd99 = 0;
  *(uint8_t*)0x2001dd9a = 0;
  *(uint8_t*)0x2001dd9b = 0;
  *(uint8_t*)0x2001dd9c = 0;
  *(uint8_t*)0x2001dd9d = 0;
  *(uint8_t*)0x2001dd9e = 0;
  *(uint8_t*)0x2001dd9f = 0;
  *(uint8_t*)0x2001dda0 = 0;
  *(uint8_t*)0x2001dda1 = 0;
  *(uint8_t*)0x2001dda2 = 0;
  *(uint8_t*)0x2001dda3 = 0;
  *(uint8_t*)0x2001dda4 = 0;
  *(uint8_t*)0x2001dda5 = 0;
  *(uint8_t*)0x2001dda6 = 0;
  *(uint8_t*)0x2001dda7 = 0;
  *(uint8_t*)0x2001dda8 = 0;
  *(uint8_t*)0x2001dda9 = 0;
  *(uint8_t*)0x2001ddaa = 0;
  *(uint8_t*)0x2001ddab = 0;
  *(uint8_t*)0x2001ddac = 0;
  *(uint8_t*)0x2001ddad = 0;
  *(uint8_t*)0x2001ddae = 0;
  *(uint8_t*)0x2001ddaf = 0;
  *(uint8_t*)0x2001ddb0 = 0;
  *(uint8_t*)0x2001ddb1 = 0;
  *(uint8_t*)0x2001ddb2 = 0;
  *(uint8_t*)0x2001ddb3 = 0;
  *(uint8_t*)0x2001ddb4 = 0;
  *(uint8_t*)0x2001ddb5 = 0;
  *(uint8_t*)0x2001ddb6 = 0;
  *(uint8_t*)0x2001ddb7 = 0;
  *(uint8_t*)0x2001ddb8 = 0;
  *(uint8_t*)0x2001ddb9 = 0;
  *(uint8_t*)0x2001ddba = 0;
  *(uint8_t*)0x2001ddbb = 0;
  *(uint8_t*)0x2001ddbc = 0xe4;
  *(uint8_t*)0x2001ddbd = 0;
  *(uint8_t*)0x2001ddbe = 0;
  *(uint8_t*)0x2001ddbf = 0;
  *(uint8_t*)0x2001ddc0 = 0;
  *(uint8_t*)0x2001ddc1 = 0;
  *(uint8_t*)0x2001ddc2 = 0;
  *(uint8_t*)0x2001ddc3 = 0;
  *(uint8_t*)0x2001ddc4 = 0;
  *(uint8_t*)0x2001ddc5 = 0;
  *(uint8_t*)0x2001ddc6 = 0;
  *(uint8_t*)0x2001ddc7 = 0;
  *(uint8_t*)0x2001ddc8 = 0;
  *(uint8_t*)0x2001ddc9 = 0;
  *(uint8_t*)0x2001ddca = 0;
  *(uint8_t*)0x2001ddcb = 0;
  *(uint8_t*)0x2001ddcc = 0;
  *(uint8_t*)0x2001ddcd = 0;
  *(uint8_t*)0x2001ddce = 0;
  *(uint8_t*)0x2001ddcf = 0;
  *(uint8_t*)0x2001ddd0 = 0;
  *(uint8_t*)0x2001ddd1 = 0;
  *(uint8_t*)0x2001ddd2 = 0;
  *(uint8_t*)0x2001ddd3 = 0;
  *(uint8_t*)0x2001ddd4 = 0;
  *(uint8_t*)0x2001ddd5 = 0;
  *(uint8_t*)0x2001ddd6 = 0;
  *(uint8_t*)0x2001ddd7 = 0;
  *(uint8_t*)0x2001ddd8 = 0;
  *(uint8_t*)0x2001ddd9 = 0;
  *(uint8_t*)0x2001ddda = 0;
  *(uint8_t*)0x2001dddb = 0;
  *(uint8_t*)0x2001dddc = 0;
  *(uint8_t*)0x2001dddd = 0;
  *(uint8_t*)0x2001ddde = 0;
  *(uint8_t*)0x2001dddf = 0;
  *(uint8_t*)0x2001dde0 = 0;
  *(uint8_t*)0x2001dde1 = 0;
  *(uint8_t*)0x2001dde2 = 0;
  *(uint8_t*)0x2001dde3 = 0;
  *(uint8_t*)0x2001dde4 = 0;
  *(uint8_t*)0x2001dde5 = 0;
  *(uint8_t*)0x2001dde6 = 0;
  *(uint8_t*)0x2001dde7 = 0;
  *(uint8_t*)0x2001dde8 = 0;
  *(uint8_t*)0x2001dde9 = 0;
  *(uint8_t*)0x2001ddea = 0;
  *(uint8_t*)0x2001ddeb = 0;
  *(uint8_t*)0x2001ddec = 0;
  *(uint8_t*)0x2001dded = 0;
  *(uint8_t*)0x2001ddee = 0;
  *(uint8_t*)0x2001ddef = 0;
  *(uint8_t*)0x2001ddf0 = 6;
  *(uint8_t*)0x2001ddf1 = 0;
  *(uint8_t*)0x2001ddf2 = 0;
  *(uint8_t*)0x2001ddf3 = 5;
  *(uint8_t*)0x2001ddf4 = 0x6e;
  *(uint8_t*)0x2001ddf5 = 0;
  *(uint8_t*)0x2001ddf6 = 0;
  *(uint8_t*)0x2001ddf7 = 0;
  *(uint32_t*)0x2001ddf8 = 0;
  *(uint16_t*)0x2001ddfc = 0xa8;
  *(uint16_t*)0x2001ddfe = 0xd0;
  *(uint32_t*)0x2001de00 = 0;
  *(uint64_t*)0x2001de04 = 0;
  *(uint64_t*)0x2001de0c = 0;
  *(uint16_t*)0x2001de18 = 0x28;
  memcpy((void*)0x2001de1a, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2001de37 = 0;
  *(uint32_t*)0x2001de38 = 1;
  *(uint8_t*)0x2001de40 = 0;
  *(uint8_t*)0x2001de41 = 0;
  *(uint8_t*)0x2001de42 = 0;
  *(uint8_t*)0x2001de43 = 0;
  *(uint8_t*)0x2001de44 = 0;
  *(uint8_t*)0x2001de45 = 0;
  *(uint8_t*)0x2001de46 = 0;
  *(uint8_t*)0x2001de47 = 0;
  *(uint8_t*)0x2001de48 = 0;
  *(uint8_t*)0x2001de49 = 0;
  *(uint8_t*)0x2001de4a = 0;
  *(uint8_t*)0x2001de4b = 0;
  *(uint8_t*)0x2001de4c = 0;
  *(uint8_t*)0x2001de4d = 0;
  *(uint8_t*)0x2001de4e = 0;
  *(uint8_t*)0x2001de4f = 0;
  *(uint8_t*)0x2001de50 = 0;
  *(uint8_t*)0x2001de51 = 0;
  *(uint8_t*)0x2001de52 = 0;
  *(uint8_t*)0x2001de53 = 0;
  *(uint8_t*)0x2001de54 = 0;
  *(uint8_t*)0x2001de55 = 0;
  *(uint8_t*)0x2001de56 = 0;
  *(uint8_t*)0x2001de57 = 0;
  *(uint8_t*)0x2001de58 = 0;
  *(uint8_t*)0x2001de59 = 0;
  *(uint8_t*)0x2001de5a = 0;
  *(uint8_t*)0x2001de5b = 0;
  *(uint8_t*)0x2001de5c = 0;
  *(uint8_t*)0x2001de5d = 0;
  *(uint8_t*)0x2001de5e = 0;
  *(uint8_t*)0x2001de5f = 0;
  *(uint8_t*)0x2001de60 = 0;
  *(uint8_t*)0x2001de61 = 0;
  *(uint8_t*)0x2001de62 = 0;
  *(uint8_t*)0x2001de63 = 0;
  *(uint8_t*)0x2001de64 = 0;
  *(uint8_t*)0x2001de65 = 0;
  *(uint8_t*)0x2001de66 = 0;
  *(uint8_t*)0x2001de67 = 0;
  *(uint8_t*)0x2001de68 = 0;
  *(uint8_t*)0x2001de69 = 0;
  *(uint8_t*)0x2001de6a = 0;
  *(uint8_t*)0x2001de6b = 0;
  *(uint8_t*)0x2001de6c = 0;
  *(uint8_t*)0x2001de6d = 0;
  *(uint8_t*)0x2001de6e = 0;
  *(uint8_t*)0x2001de6f = 0;
  *(uint8_t*)0x2001de70 = 0;
  *(uint8_t*)0x2001de71 = 0;
  *(uint8_t*)0x2001de72 = 0;
  *(uint8_t*)0x2001de73 = 0;
  *(uint8_t*)0x2001de74 = 0;
  *(uint8_t*)0x2001de75 = 0;
  *(uint8_t*)0x2001de76 = 0;
  *(uint8_t*)0x2001de77 = 0;
  *(uint8_t*)0x2001de78 = 0;
  *(uint8_t*)0x2001de79 = 0;
  *(uint8_t*)0x2001de7a = 0;
  *(uint8_t*)0x2001de7b = 0;
  *(uint8_t*)0x2001de7c = 0;
  *(uint8_t*)0x2001de7d = 0;
  *(uint8_t*)0x2001de7e = 0;
  *(uint8_t*)0x2001de7f = 0;
  *(uint8_t*)0x2001de80 = 0;
  *(uint8_t*)0x2001de81 = 0;
  *(uint8_t*)0x2001de82 = 0;
  *(uint8_t*)0x2001de83 = 0;
  *(uint8_t*)0x2001de84 = 0;
  *(uint8_t*)0x2001de85 = 0;
  *(uint8_t*)0x2001de86 = 0;
  *(uint8_t*)0x2001de87 = 0;
  *(uint8_t*)0x2001de88 = 0;
  *(uint8_t*)0x2001de89 = 0;
  *(uint8_t*)0x2001de8a = 0;
  *(uint8_t*)0x2001de8b = 0;
  *(uint8_t*)0x2001de8c = 0;
  *(uint8_t*)0x2001de8d = 0;
  *(uint8_t*)0x2001de8e = 0;
  *(uint8_t*)0x2001de8f = 0;
  *(uint8_t*)0x2001de90 = 0;
  *(uint8_t*)0x2001de91 = 0;
  *(uint8_t*)0x2001de92 = 0;
  *(uint8_t*)0x2001de93 = 0;
  *(uint8_t*)0x2001de94 = 0;
  *(uint8_t*)0x2001de95 = 0;
  *(uint8_t*)0x2001de96 = 0;
  *(uint8_t*)0x2001de97 = 0;
  *(uint8_t*)0x2001de98 = 0;
  *(uint8_t*)0x2001de99 = 0;
  *(uint8_t*)0x2001de9a = 0;
  *(uint8_t*)0x2001de9b = 0;
  *(uint8_t*)0x2001de9c = 0;
  *(uint8_t*)0x2001de9d = 0;
  *(uint8_t*)0x2001de9e = 0;
  *(uint8_t*)0x2001de9f = 0;
  *(uint8_t*)0x2001dea0 = 0;
  *(uint8_t*)0x2001dea1 = 0;
  *(uint8_t*)0x2001dea2 = 0;
  *(uint8_t*)0x2001dea3 = 0;
  *(uint8_t*)0x2001dea4 = 0;
  *(uint8_t*)0x2001dea5 = 0;
  *(uint8_t*)0x2001dea6 = 0;
  *(uint8_t*)0x2001dea7 = 0;
  *(uint8_t*)0x2001dea8 = 0;
  *(uint8_t*)0x2001dea9 = 0;
  *(uint8_t*)0x2001deaa = 0;
  *(uint8_t*)0x2001deab = 0;
  *(uint8_t*)0x2001deac = 0;
  *(uint8_t*)0x2001dead = 0;
  *(uint8_t*)0x2001deae = 0;
  *(uint8_t*)0x2001deaf = 0;
  *(uint8_t*)0x2001deb0 = 0;
  *(uint8_t*)0x2001deb1 = 0;
  *(uint8_t*)0x2001deb2 = 0;
  *(uint8_t*)0x2001deb3 = 0;
  *(uint8_t*)0x2001deb4 = 0;
  *(uint8_t*)0x2001deb5 = 0;
  *(uint8_t*)0x2001deb6 = 0;
  *(uint8_t*)0x2001deb7 = 0;
  *(uint8_t*)0x2001deb8 = 0;
  *(uint8_t*)0x2001deb9 = 0;
  *(uint8_t*)0x2001deba = 0;
  *(uint8_t*)0x2001debb = 0;
  *(uint8_t*)0x2001debc = 0;
  *(uint8_t*)0x2001debd = 0;
  *(uint8_t*)0x2001debe = 0;
  *(uint8_t*)0x2001debf = 0;
  *(uint8_t*)0x2001dec0 = 0;
  *(uint8_t*)0x2001dec1 = 0;
  *(uint8_t*)0x2001dec2 = 0;
  *(uint8_t*)0x2001dec3 = 0;
  *(uint8_t*)0x2001dec4 = 0;
  *(uint8_t*)0x2001dec5 = 0;
  *(uint8_t*)0x2001dec6 = 0;
  *(uint8_t*)0x2001dec7 = 0;
  *(uint32_t*)0x2001dec8 = 0;
  *(uint16_t*)0x2001decc = 0xa8;
  *(uint16_t*)0x2001dece = 0xe8;
  *(uint32_t*)0x2001ded0 = 0;
  *(uint64_t*)0x2001ded4 = 0;
  *(uint64_t*)0x2001dedc = 0;
  *(uint16_t*)0x2001dee8 = 0x40;
  memcpy((void*)0x2001deea, "\x52\x41\x54\x45\x45\x53\x54\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2001df07 = 0;
  memcpy((void*)0x2001df08,
         "\x73\x79\x7a\x30\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x2001df18 = 0;
  *(uint8_t*)0x2001df19 = 0;
  *(uint64_t*)0x2001df20 = 0;
  *(uint8_t*)0x2001df28 = 0xfe;
  *(uint8_t*)0x2001df29 = 0x80;
  *(uint8_t*)0x2001df2a = 0;
  *(uint8_t*)0x2001df2b = 0;
  *(uint8_t*)0x2001df2c = 0;
  *(uint8_t*)0x2001df2d = 0;
  *(uint8_t*)0x2001df2e = 0;
  *(uint8_t*)0x2001df2f = 0;
  *(uint8_t*)0x2001df30 = 0;
  *(uint8_t*)0x2001df31 = 0;
  *(uint8_t*)0x2001df32 = 0;
  *(uint8_t*)0x2001df33 = 0;
  *(uint8_t*)0x2001df34 = 0;
  *(uint8_t*)0x2001df35 = 0;
  *(uint8_t*)0x2001df36 = 0;
  *(uint8_t*)0x2001df37 = 0xbb;
  *(uint8_t*)0x2001df38 = 0;
  *(uint8_t*)0x2001df39 = 0;
  *(uint8_t*)0x2001df3a = 0;
  *(uint8_t*)0x2001df3b = 0;
  *(uint8_t*)0x2001df3c = 0;
  *(uint8_t*)0x2001df3d = 0;
  *(uint8_t*)0x2001df3e = 0;
  *(uint8_t*)0x2001df3f = 0;
  *(uint8_t*)0x2001df40 = 0;
  *(uint8_t*)0x2001df41 = 0;
  *(uint8_t*)0x2001df42 = 0;
  *(uint8_t*)0x2001df43 = 0;
  *(uint8_t*)0x2001df44 = 0;
  *(uint8_t*)0x2001df45 = 0;
  *(uint8_t*)0x2001df46 = 0;
  *(uint8_t*)0x2001df47 = 0;
  *(uint32_t*)0x2001df48 = htobe32(0);
  *(uint32_t*)0x2001df4c = htobe32(0);
  *(uint32_t*)0x2001df50 = htobe32(0);
  *(uint32_t*)0x2001df54 = htobe32(0);
  *(uint32_t*)0x2001df58 = htobe32(0);
  *(uint32_t*)0x2001df5c = htobe32(0);
  *(uint32_t*)0x2001df60 = htobe32(0);
  *(uint32_t*)0x2001df64 = htobe32(0);
  *(uint8_t*)0x2001df68 = 0x73;
  *(uint8_t*)0x2001df69 = 0x79;
  *(uint8_t*)0x2001df6a = 0x7a;
  *(uint8_t*)0x2001df6b = 0x30;
  *(uint8_t*)0x2001df6c = 0;
  memcpy((void*)0x2001df78,
         "\xe4\x6a\x92\x2f\x8e\xfd\xf5\x55\xae\xad\xc0\x19\x97\xa6\xf7\xbb",
         16);
  *(uint8_t*)0x2001df88 = 0;
  *(uint8_t*)0x2001df89 = 0;
  *(uint8_t*)0x2001df8a = 0;
  *(uint8_t*)0x2001df8b = 0;
  *(uint8_t*)0x2001df8c = 0;
  *(uint8_t*)0x2001df8d = 0;
  *(uint8_t*)0x2001df8e = 0;
  *(uint8_t*)0x2001df8f = 0;
  *(uint8_t*)0x2001df90 = 0;
  *(uint8_t*)0x2001df91 = 0;
  *(uint8_t*)0x2001df92 = 0;
  *(uint8_t*)0x2001df93 = 0;
  *(uint8_t*)0x2001df94 = 0;
  *(uint8_t*)0x2001df95 = 0;
  *(uint8_t*)0x2001df96 = 0;
  *(uint8_t*)0x2001df97 = 0;
  *(uint8_t*)0x2001df98 = 0;
  *(uint8_t*)0x2001df99 = 0;
  *(uint8_t*)0x2001df9a = 0;
  *(uint8_t*)0x2001df9b = 0;
  *(uint8_t*)0x2001df9c = 0;
  *(uint8_t*)0x2001df9d = 0;
  *(uint8_t*)0x2001df9e = 0;
  *(uint8_t*)0x2001df9f = 0;
  *(uint8_t*)0x2001dfa0 = 0;
  *(uint8_t*)0x2001dfa1 = 0;
  *(uint8_t*)0x2001dfa2 = 0;
  *(uint8_t*)0x2001dfa3 = 0;
  *(uint8_t*)0x2001dfa4 = 0;
  *(uint8_t*)0x2001dfa5 = 0;
  *(uint8_t*)0x2001dfa6 = 0;
  *(uint8_t*)0x2001dfa7 = 0;
  *(uint16_t*)0x2001dfa8 = 0;
  *(uint8_t*)0x2001dfaa = 0;
  *(uint8_t*)0x2001dfab = 0;
  *(uint8_t*)0x2001dfac = 0;
  *(uint32_t*)0x2001dfb0 = 0;
  *(uint16_t*)0x2001dfb4 = 0xa8;
  *(uint16_t*)0x2001dfb6 = 0xd0;
  *(uint32_t*)0x2001dfb8 = 0;
  *(uint64_t*)0x2001dfbc = 0;
  *(uint64_t*)0x2001dfc4 = 0;
  *(uint16_t*)0x2001dfd0 = 0x28;
  memcpy((void*)0x2001dfd2, "\x52\x45\x4a\x45\x43\x54\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00",
         29);
  *(uint8_t*)0x2001dfef = 0;
  *(uint32_t*)0x2001dff0 = 0xfffffffd;
  *(uint64_t*)0x20012000 = 0;
  *(uint64_t*)0x20012008 = 0;
  *(uint64_t*)0x20012010 = 0;
  *(uint64_t*)0x20012018 = 0;
  *(uint64_t*)0x20012020 = 0;
  *(uint64_t*)0x20012028 = 0;
  *(uint64_t*)0x20012030 = 0;
  *(uint64_t*)0x20012038 = 0;
  syscall(__NR_setsockopt, r[0], 0x29, 0x40, 0x2001dc40, 0x3b8);
}

int main()
{
  for (;;) {
    loop();
  }
}
