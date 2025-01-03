// https://syzkaller.appspot.com/bug?id=8e40dd7560a0b689564f9d56830e27904f69bef7
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const int kFailStatus = 67;
const int kRetryStatus = 69;

static void fail(const char* msg, ...)
{
  int e = errno;
  va_list args;
  va_start(args, msg);
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, " (errno %d)\n", e);
  doexit((e == ENOMEM || e == EAGAIN) ? kRetryStatus : kFailStatus);
}

static void loop();

static void sandbox_common()
{
  prctl(PR_SET_PDEATHSIG, SIGKILL, 0, 0, 0);
  setpgrp();
  setsid();

  struct rlimit rlim;
  rlim.rlim_cur = rlim.rlim_max = 128 << 20;
  setrlimit(RLIMIT_AS, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 8 << 20;
  setrlimit(RLIMIT_MEMLOCK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_FSIZE, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 1 << 20;
  setrlimit(RLIMIT_STACK, &rlim);
  rlim.rlim_cur = rlim.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &rlim);

#define CLONE_NEWCGROUP 0x02000000

  if (unshare(CLONE_NEWNS)) {
  }
  if (unshare(CLONE_NEWIPC)) {
  }
  if (unshare(CLONE_NEWCGROUP)) {
  }
  if (unshare(CLONE_NEWUTS)) {
  }
  if (unshare(CLONE_SYSVSEM)) {
  }
}

static int do_sandbox_none(void)
{
  if (unshare(CLONE_NEWPID)) {
  }
  int pid = fork();
  if (pid < 0)
    fail("sandbox fork failed");
  if (pid)
    return pid;

  sandbox_common();
  if (unshare(CLONE_NEWNET)) {
  }

  loop();
  doexit(1);
}

uint64_t r[1] = {0xffffffffffffffff};
void loop()
{
  long res;
  res = syscall(__NR_socket, 0xa, 6, 0);
  if (res != -1)
    r[0] = res;
  memcpy((void*)0x20000140, "\x66\x69\x6c\x74\x65\x72\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20000160 = 0xe;
  *(uint32_t*)0x20000164 = 2;
  *(uint32_t*)0x20000168 = 0xb98;
  *(uint64_t*)0x20000170 = 0;
  *(uint64_t*)0x20000178 = 0x20000240;
  *(uint64_t*)0x20000180 = 0x20000d78;
  *(uint64_t*)0x20000188 = 0x20000da8;
  *(uint64_t*)0x20000190 = 0;
  *(uint64_t*)0x20000198 = 0;
  *(uint32_t*)0x200001a0 = 0;
  *(uint64_t*)0x200001a8 = 0x20000480;
  *(uint64_t*)0x200001b0 = 0x20000240;
  *(uint32_t*)0x20000240 = 0;
  memcpy((void*)0x20000244, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20000264 = 0;
  *(uint32_t*)0x20000268 = -1;
  *(uint32_t*)0x2000026c = 2;
  *(uint32_t*)0x20000270 = 0x11;
  *(uint32_t*)0x20000274 = 0;
  *(uint16_t*)0x20000278 = htobe16(0);
  memcpy((void*)0x2000027a,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  memcpy((void*)0x2000028a,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  memcpy((void*)0x2000029a,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  memcpy((void*)0x200002aa,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x200002ba = 0xaa;
  *(uint8_t*)0x200002bb = 0xaa;
  *(uint8_t*)0x200002bc = 0xaa;
  *(uint8_t*)0x200002bd = 0xaa;
  *(uint8_t*)0x200002be = 0;
  *(uint8_t*)0x200002bf = -1;
  *(uint8_t*)0x200002c0 = 0;
  *(uint8_t*)0x200002c1 = 0;
  *(uint8_t*)0x200002c2 = 0;
  *(uint8_t*)0x200002c3 = 0;
  *(uint8_t*)0x200002c4 = 0;
  *(uint8_t*)0x200002c5 = 0;
  *(uint8_t*)0x200002c6 = 0xaa;
  *(uint8_t*)0x200002c7 = 0xaa;
  *(uint8_t*)0x200002c8 = 0xaa;
  *(uint8_t*)0x200002c9 = 0xaa;
  *(uint8_t*)0x200002ca = 0;
  *(uint8_t*)0x200002cb = -1;
  *(uint8_t*)0x200002cc = 0;
  *(uint8_t*)0x200002cd = 0;
  *(uint8_t*)0x200002ce = 0;
  *(uint8_t*)0x200002cf = 0;
  *(uint8_t*)0x200002d0 = 0;
  *(uint8_t*)0x200002d1 = 0;
  *(uint32_t*)0x200002d4 = 0x70;
  *(uint32_t*)0x200002d8 = 0xc0;
  *(uint32_t*)0x200002dc = 0xf8;
  memcpy((void*)0x200002e0, "\x49\x44\x4c\x45\x54\x49\x4d\x45\x52\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20000300 = 0x28;
  *(uint32_t*)0x20000308 = 4;
  memcpy((void*)0x2000030c, "\x73\x79\x7a\x30\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00",
         28);
  *(uint64_t*)0x20000328 = 0;
  memcpy((void*)0x20000330, "\x6d\x61\x72\x6b\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20000350 = 0x10;
  *(uint64_t*)0x20000358 = 0;
  *(uint32_t*)0x20000360 = 0xfffffffd;
  *(uint32_t*)0x20000368 = 5;
  *(uint32_t*)0x2000036c = 0;
  *(uint16_t*)0x20000370 = htobe16(0);
  memcpy((void*)0x20000372,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  memcpy((void*)0x20000382,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  memcpy((void*)0x20000392,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  memcpy((void*)0x200003a2,
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
         16);
  *(uint8_t*)0x200003b2 = 1;
  *(uint8_t*)0x200003b3 = 0x80;
  *(uint8_t*)0x200003b4 = 0xc2;
  *(uint8_t*)0x200003b5 = 0;
  *(uint8_t*)0x200003b6 = 0;
  *(uint8_t*)0x200003b7 = 0;
  *(uint8_t*)0x200003b8 = 0;
  *(uint8_t*)0x200003b9 = 0;
  *(uint8_t*)0x200003ba = 0;
  *(uint8_t*)0x200003bb = 0;
  *(uint8_t*)0x200003bc = 0;
  *(uint8_t*)0x200003bd = 0;
  *(uint8_t*)0x200003be = 0xaa;
  *(uint8_t*)0x200003bf = 0xaa;
  *(uint8_t*)0x200003c0 = 0xaa;
  *(uint8_t*)0x200003c1 = 0xaa;
  *(uint8_t*)0x200003c2 = 0;
  *(uint8_t*)0x200003c3 = 0;
  *(uint8_t*)0x200003c4 = 0;
  *(uint8_t*)0x200003c5 = 0;
  *(uint8_t*)0x200003c6 = 0;
  *(uint8_t*)0x200003c7 = 0;
  *(uint8_t*)0x200003c8 = 0;
  *(uint8_t*)0x200003c9 = 0;
  *(uint32_t*)0x200003cc = 0x9c0;
  *(uint32_t*)0x200003d0 = 0x9c0;
  *(uint32_t*)0x200003d4 = 0xa10;
  memcpy((void*)0x200003d8, "\x61\x6d\x6f\x6e\x67\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x200003f8 = 0x8f8;
  *(uint32_t*)0x20000400 = 0x8001;
  *(uint32_t*)0x20000404 = 0;
  *(uint32_t*)0x20000408 = 0;
  *(uint32_t*)0x2000040c = 0;
  *(uint32_t*)0x20000410 = 0;
  *(uint32_t*)0x20000414 = 0;
  *(uint32_t*)0x20000418 = 0;
  *(uint32_t*)0x2000041c = 0;
  *(uint32_t*)0x20000420 = 0;
  *(uint32_t*)0x20000424 = 0;
  *(uint32_t*)0x20000428 = 0;
  *(uint32_t*)0x2000042c = 0;
  *(uint32_t*)0x20000430 = 0;
  *(uint32_t*)0x20000434 = 0;
  *(uint32_t*)0x20000438 = 0;
  *(uint32_t*)0x2000043c = 0;
  *(uint32_t*)0x20000440 = 0;
  *(uint32_t*)0x20000444 = 0;
  *(uint32_t*)0x20000448 = 0;
  *(uint32_t*)0x2000044c = 0;
  *(uint32_t*)0x20000450 = 0;
  *(uint32_t*)0x20000454 = 0;
  *(uint32_t*)0x20000458 = 0;
  *(uint32_t*)0x2000045c = 0;
  *(uint32_t*)0x20000460 = 0;
  *(uint32_t*)0x20000464 = 0;
  *(uint32_t*)0x20000468 = 0;
  *(uint32_t*)0x2000046c = 0;
  *(uint32_t*)0x20000470 = 0;
  *(uint32_t*)0x20000474 = 0;
  *(uint32_t*)0x20000478 = 0;
  *(uint32_t*)0x2000047c = 0;
  *(uint32_t*)0x20000480 = 0;
  *(uint32_t*)0x20000484 = 0;
  *(uint32_t*)0x20000488 = 0;
  *(uint32_t*)0x2000048c = 0;
  *(uint32_t*)0x20000490 = 0;
  *(uint32_t*)0x20000494 = 0;
  *(uint32_t*)0x20000498 = 0;
  *(uint32_t*)0x2000049c = 0;
  *(uint32_t*)0x200004a0 = 0;
  *(uint32_t*)0x200004a4 = 0;
  *(uint32_t*)0x200004a8 = 0;
  *(uint32_t*)0x200004ac = 0;
  *(uint32_t*)0x200004b0 = 0;
  *(uint32_t*)0x200004b4 = 0;
  *(uint32_t*)0x200004b8 = 0;
  *(uint32_t*)0x200004bc = 0;
  *(uint32_t*)0x200004c0 = 0;
  *(uint32_t*)0x200004c4 = 0;
  *(uint32_t*)0x200004c8 = 0;
  *(uint32_t*)0x200004cc = 0;
  *(uint32_t*)0x200004d0 = 0;
  *(uint32_t*)0x200004d4 = 0;
  *(uint32_t*)0x200004d8 = 0;
  *(uint32_t*)0x200004dc = 0;
  *(uint32_t*)0x200004e0 = 0;
  *(uint32_t*)0x200004e4 = 0;
  *(uint32_t*)0x200004e8 = 0;
  *(uint32_t*)0x200004ec = 0;
  *(uint32_t*)0x200004f0 = 0;
  *(uint32_t*)0x200004f4 = 0;
  *(uint32_t*)0x200004f8 = 0;
  *(uint32_t*)0x200004fc = 0;
  *(uint32_t*)0x20000500 = 0;
  *(uint32_t*)0x20000504 = 0;
  *(uint32_t*)0x20000508 = 0;
  *(uint32_t*)0x2000050c = 0;
  *(uint32_t*)0x20000510 = 0;
  *(uint32_t*)0x20000514 = 0;
  *(uint32_t*)0x20000518 = 0;
  *(uint32_t*)0x2000051c = 0;
  *(uint32_t*)0x20000520 = 0;
  *(uint32_t*)0x20000524 = 0;
  *(uint32_t*)0x20000528 = 0;
  *(uint32_t*)0x2000052c = 0;
  *(uint32_t*)0x20000530 = 0;
  *(uint32_t*)0x20000534 = 0;
  *(uint32_t*)0x20000538 = 0;
  *(uint32_t*)0x2000053c = 0;
  *(uint32_t*)0x20000540 = 0;
  *(uint32_t*)0x20000544 = 0;
  *(uint32_t*)0x20000548 = 0;
  *(uint32_t*)0x2000054c = 0;
  *(uint32_t*)0x20000550 = 0;
  *(uint32_t*)0x20000554 = 0;
  *(uint32_t*)0x20000558 = 0;
  *(uint32_t*)0x2000055c = 0;
  *(uint32_t*)0x20000560 = 0;
  *(uint32_t*)0x20000564 = 0;
  *(uint32_t*)0x20000568 = 0;
  *(uint32_t*)0x2000056c = 0;
  *(uint32_t*)0x20000570 = 0;
  *(uint32_t*)0x20000574 = 0;
  *(uint32_t*)0x20000578 = 0;
  *(uint32_t*)0x2000057c = 0;
  *(uint32_t*)0x20000580 = 0;
  *(uint32_t*)0x20000584 = 0;
  *(uint32_t*)0x20000588 = 0;
  *(uint32_t*)0x2000058c = 0;
  *(uint32_t*)0x20000590 = 0;
  *(uint32_t*)0x20000594 = 0;
  *(uint32_t*)0x20000598 = 0;
  *(uint32_t*)0x2000059c = 0;
  *(uint32_t*)0x200005a0 = 0;
  *(uint32_t*)0x200005a4 = 0;
  *(uint32_t*)0x200005a8 = 0;
  *(uint32_t*)0x200005ac = 0;
  *(uint32_t*)0x200005b0 = 0;
  *(uint32_t*)0x200005b4 = 0;
  *(uint32_t*)0x200005b8 = 0;
  *(uint32_t*)0x200005bc = 0;
  *(uint32_t*)0x200005c0 = 0;
  *(uint32_t*)0x200005c4 = 0;
  *(uint32_t*)0x200005c8 = 0;
  *(uint32_t*)0x200005cc = 0;
  *(uint32_t*)0x200005d0 = 0;
  *(uint32_t*)0x200005d4 = 0;
  *(uint32_t*)0x200005d8 = 0;
  *(uint32_t*)0x200005dc = 0;
  *(uint32_t*)0x200005e0 = 0;
  *(uint32_t*)0x200005e4 = 0;
  *(uint32_t*)0x200005e8 = 0;
  *(uint32_t*)0x200005ec = 0;
  *(uint32_t*)0x200005f0 = 0;
  *(uint32_t*)0x200005f4 = 0;
  *(uint32_t*)0x200005f8 = 0;
  *(uint32_t*)0x200005fc = 0;
  *(uint32_t*)0x20000600 = 0;
  *(uint32_t*)0x20000604 = 0;
  *(uint32_t*)0x20000608 = 0;
  *(uint32_t*)0x2000060c = 0;
  *(uint32_t*)0x20000610 = 0;
  *(uint32_t*)0x20000614 = 0;
  *(uint32_t*)0x20000618 = 0;
  *(uint32_t*)0x2000061c = 0;
  *(uint32_t*)0x20000620 = 0;
  *(uint32_t*)0x20000624 = 0;
  *(uint32_t*)0x20000628 = 0;
  *(uint32_t*)0x2000062c = 0;
  *(uint32_t*)0x20000630 = 0;
  *(uint32_t*)0x20000634 = 0;
  *(uint32_t*)0x20000638 = 0;
  *(uint32_t*)0x2000063c = 0;
  *(uint32_t*)0x20000640 = 0;
  *(uint32_t*)0x20000644 = 0;
  *(uint32_t*)0x20000648 = 0;
  *(uint32_t*)0x2000064c = 0;
  *(uint32_t*)0x20000650 = 0;
  *(uint32_t*)0x20000654 = 0;
  *(uint32_t*)0x20000658 = 0;
  *(uint32_t*)0x2000065c = 0;
  *(uint32_t*)0x20000660 = 0;
  *(uint32_t*)0x20000664 = 0;
  *(uint32_t*)0x20000668 = 0;
  *(uint32_t*)0x2000066c = 0;
  *(uint32_t*)0x20000670 = 0;
  *(uint32_t*)0x20000674 = 0;
  *(uint32_t*)0x20000678 = 0;
  *(uint32_t*)0x2000067c = 0;
  *(uint32_t*)0x20000680 = 0;
  *(uint32_t*)0x20000684 = 0;
  *(uint32_t*)0x20000688 = 0;
  *(uint32_t*)0x2000068c = 0;
  *(uint32_t*)0x20000690 = 0;
  *(uint32_t*)0x20000694 = 0;
  *(uint32_t*)0x20000698 = 0;
  *(uint32_t*)0x2000069c = 0;
  *(uint32_t*)0x200006a0 = 0;
  *(uint32_t*)0x200006a4 = 0;
  *(uint32_t*)0x200006a8 = 0;
  *(uint32_t*)0x200006ac = 0;
  *(uint32_t*)0x200006b0 = 0;
  *(uint32_t*)0x200006b4 = 0;
  *(uint32_t*)0x200006b8 = 0;
  *(uint32_t*)0x200006bc = 0;
  *(uint32_t*)0x200006c0 = 0;
  *(uint32_t*)0x200006c4 = 0;
  *(uint32_t*)0x200006c8 = 0;
  *(uint32_t*)0x200006cc = 0;
  *(uint32_t*)0x200006d0 = 0;
  *(uint32_t*)0x200006d4 = 0;
  *(uint32_t*)0x200006d8 = 0;
  *(uint32_t*)0x200006dc = 0;
  *(uint32_t*)0x200006e0 = 0;
  *(uint32_t*)0x200006e4 = 0;
  *(uint32_t*)0x200006e8 = 0;
  *(uint32_t*)0x200006ec = 0;
  *(uint32_t*)0x200006f0 = 0;
  *(uint32_t*)0x200006f4 = 0;
  *(uint32_t*)0x200006f8 = 0;
  *(uint32_t*)0x200006fc = 0;
  *(uint32_t*)0x20000700 = 0;
  *(uint32_t*)0x20000704 = 0;
  *(uint32_t*)0x20000708 = 0;
  *(uint32_t*)0x2000070c = 0;
  *(uint32_t*)0x20000710 = 0;
  *(uint32_t*)0x20000714 = 0;
  *(uint32_t*)0x20000718 = 0;
  *(uint32_t*)0x2000071c = 0;
  *(uint32_t*)0x20000720 = 0;
  *(uint32_t*)0x20000724 = 0;
  *(uint32_t*)0x20000728 = 0;
  *(uint32_t*)0x2000072c = 0;
  *(uint32_t*)0x20000730 = 0;
  *(uint32_t*)0x20000734 = 0;
  *(uint32_t*)0x20000738 = 0;
  *(uint32_t*)0x2000073c = 0;
  *(uint32_t*)0x20000740 = 0;
  *(uint32_t*)0x20000744 = 0;
  *(uint32_t*)0x20000748 = 0;
  *(uint32_t*)0x2000074c = 0;
  *(uint32_t*)0x20000750 = 0;
  *(uint32_t*)0x20000754 = 0;
  *(uint32_t*)0x20000758 = 0;
  *(uint32_t*)0x2000075c = 0;
  *(uint32_t*)0x20000760 = 0;
  *(uint32_t*)0x20000764 = 0;
  *(uint32_t*)0x20000768 = 0;
  *(uint32_t*)0x2000076c = 0;
  *(uint32_t*)0x20000770 = 0;
  *(uint32_t*)0x20000774 = 0;
  *(uint32_t*)0x20000778 = 0;
  *(uint32_t*)0x2000077c = 0;
  *(uint32_t*)0x20000780 = 0;
  *(uint32_t*)0x20000784 = 0;
  *(uint32_t*)0x20000788 = 0;
  *(uint32_t*)0x2000078c = 0;
  *(uint32_t*)0x20000790 = 0;
  *(uint32_t*)0x20000794 = 0;
  *(uint32_t*)0x20000798 = 0;
  *(uint32_t*)0x2000079c = 0;
  *(uint32_t*)0x200007a0 = 0;
  *(uint32_t*)0x200007a4 = 0;
  *(uint32_t*)0x200007a8 = 0;
  *(uint32_t*)0x200007ac = 0;
  *(uint32_t*)0x200007b0 = 0;
  *(uint32_t*)0x200007b4 = 0;
  *(uint32_t*)0x200007b8 = 0;
  *(uint32_t*)0x200007bc = 0;
  *(uint32_t*)0x200007c0 = 0;
  *(uint32_t*)0x200007c4 = 0;
  *(uint32_t*)0x200007c8 = 0;
  *(uint32_t*)0x200007cc = 0;
  *(uint32_t*)0x200007d0 = 0;
  *(uint32_t*)0x200007d4 = 0;
  *(uint32_t*)0x200007d8 = 0;
  *(uint32_t*)0x200007dc = 0;
  *(uint32_t*)0x200007e0 = 0;
  *(uint32_t*)0x200007e4 = 0;
  *(uint32_t*)0x200007e8 = 0;
  *(uint32_t*)0x200007ec = 0;
  *(uint32_t*)0x200007f0 = 0;
  *(uint32_t*)0x200007f4 = 0;
  *(uint32_t*)0x200007f8 = 0;
  *(uint32_t*)0x200007fc = 0;
  *(uint32_t*)0x20000800 = 0;
  *(uint32_t*)0x20000804 = 0;
  *(uint32_t*)0x20000808 = 0;
  *(uint32_t*)0x2000080c = 0;
  *(uint32_t*)0x20000810 = 8;
  *(uint32_t*)0x20000814 = 0;
  *(uint32_t*)0x20000818 = 0;
  *(uint32_t*)0x2000081c = htobe32(-1);
  *(uint32_t*)0x20000820 = 0;
  *(uint32_t*)0x20000824 = 0;
  *(uint32_t*)0x20000828 = htobe32(0xe0000001);
  *(uint32_t*)0x2000082c = 0;
  *(uint32_t*)0x20000830 = 0;
  *(uint32_t*)0x20000834 = htobe32(-1);
  *(uint32_t*)0x20000838 = 0;
  *(uint32_t*)0x2000083c = 0;
  *(uint32_t*)0x20000840 = htobe32(0xe0000001);
  *(uint32_t*)0x20000844 = 0;
  *(uint32_t*)0x20000848 = 0;
  *(uint32_t*)0x2000084c = htobe32(-1);
  *(uint32_t*)0x20000850 = 0;
  *(uint32_t*)0x20000854 = 0;
  *(uint32_t*)0x20000858 = htobe32(-1);
  *(uint32_t*)0x2000085c = 0;
  *(uint32_t*)0x20000860 = 0;
  *(uint8_t*)0x20000864 = 0xac;
  *(uint8_t*)0x20000865 = 0x14;
  *(uint8_t*)0x20000866 = -1;
  *(uint8_t*)0x20000867 = 0xaa;
  *(uint32_t*)0x20000868 = 0;
  *(uint32_t*)0x2000086c = 0;
  *(uint8_t*)0x20000870 = 0xac;
  *(uint8_t*)0x20000871 = 0x14;
  *(uint8_t*)0x20000872 = 0;
  *(uint8_t*)0x20000873 = 0;
  *(uint32_t*)0x20000874 = 0;
  *(uint32_t*)0x20000878 = 0;
  *(uint32_t*)0x2000087c = 0;
  *(uint32_t*)0x20000880 = 0;
  *(uint32_t*)0x20000884 = 0;
  *(uint32_t*)0x20000888 = 0;
  *(uint32_t*)0x2000088c = 0;
  *(uint32_t*)0x20000890 = 0;
  *(uint32_t*)0x20000894 = 0;
  *(uint32_t*)0x20000898 = 0;
  *(uint32_t*)0x2000089c = 0;
  *(uint32_t*)0x200008a0 = 0;
  *(uint32_t*)0x200008a4 = 0;
  *(uint32_t*)0x200008a8 = 0;
  *(uint32_t*)0x200008ac = 0;
  *(uint32_t*)0x200008b0 = 0;
  *(uint32_t*)0x200008b4 = 0;
  *(uint32_t*)0x200008b8 = 0;
  *(uint32_t*)0x200008bc = 0;
  *(uint32_t*)0x200008c0 = 0;
  *(uint32_t*)0x200008c4 = 0;
  *(uint32_t*)0x200008c8 = 0;
  *(uint32_t*)0x200008cc = 0;
  *(uint32_t*)0x200008d0 = 0;
  *(uint32_t*)0x200008d4 = 0;
  *(uint32_t*)0x200008d8 = 0;
  *(uint32_t*)0x200008dc = 0;
  *(uint32_t*)0x200008e0 = 0;
  *(uint32_t*)0x200008e4 = 0;
  *(uint32_t*)0x200008e8 = 0;
  *(uint32_t*)0x200008ec = 0;
  *(uint32_t*)0x200008f0 = 0;
  *(uint32_t*)0x200008f4 = 0;
  *(uint32_t*)0x200008f8 = 0;
  *(uint32_t*)0x200008fc = 0;
  *(uint32_t*)0x20000900 = 0;
  *(uint32_t*)0x20000904 = 0;
  *(uint32_t*)0x20000908 = 0;
  *(uint32_t*)0x2000090c = 0;
  *(uint32_t*)0x20000910 = 0;
  *(uint32_t*)0x20000914 = 0;
  *(uint32_t*)0x20000918 = 0;
  *(uint32_t*)0x2000091c = 0;
  *(uint32_t*)0x20000920 = 0;
  *(uint32_t*)0x20000924 = 0;
  *(uint32_t*)0x20000928 = 0;
  *(uint32_t*)0x2000092c = 0;
  *(uint32_t*)0x20000930 = 0;
  *(uint32_t*)0x20000934 = 0;
  *(uint32_t*)0x20000938 = 0;
  *(uint32_t*)0x2000093c = 0;
  *(uint32_t*)0x20000940 = 0;
  *(uint32_t*)0x20000944 = 0;
  *(uint32_t*)0x20000948 = 0;
  *(uint32_t*)0x2000094c = 0;
  *(uint32_t*)0x20000950 = 0;
  *(uint32_t*)0x20000954 = 0;
  *(uint32_t*)0x20000958 = 0;
  *(uint32_t*)0x2000095c = 0;
  *(uint32_t*)0x20000960 = 0;
  *(uint32_t*)0x20000964 = 0;
  *(uint32_t*)0x20000968 = 0;
  *(uint32_t*)0x2000096c = 0;
  *(uint32_t*)0x20000970 = 0;
  *(uint32_t*)0x20000974 = 0;
  *(uint32_t*)0x20000978 = 0;
  *(uint32_t*)0x2000097c = 0;
  *(uint32_t*)0x20000980 = 0;
  *(uint32_t*)0x20000984 = 0;
  *(uint32_t*)0x20000988 = 0;
  *(uint32_t*)0x2000098c = 0;
  *(uint32_t*)0x20000990 = 0;
  *(uint32_t*)0x20000994 = 0;
  *(uint32_t*)0x20000998 = 0;
  *(uint32_t*)0x2000099c = 0;
  *(uint32_t*)0x200009a0 = 0;
  *(uint32_t*)0x200009a4 = 0;
  *(uint32_t*)0x200009a8 = 0;
  *(uint32_t*)0x200009ac = 0;
  *(uint32_t*)0x200009b0 = 0;
  *(uint32_t*)0x200009b4 = 0;
  *(uint32_t*)0x200009b8 = 0;
  *(uint32_t*)0x200009bc = 0;
  *(uint32_t*)0x200009c0 = 0;
  *(uint32_t*)0x200009c4 = 0;
  *(uint32_t*)0x200009c8 = 0;
  *(uint32_t*)0x200009cc = 0;
  *(uint32_t*)0x200009d0 = 0;
  *(uint32_t*)0x200009d4 = 0;
  *(uint32_t*)0x200009d8 = 0;
  *(uint32_t*)0x200009dc = 0;
  *(uint32_t*)0x200009e0 = 0;
  *(uint32_t*)0x200009e4 = 0;
  *(uint32_t*)0x200009e8 = 0;
  *(uint32_t*)0x200009ec = 0;
  *(uint32_t*)0x200009f0 = 0;
  *(uint32_t*)0x200009f4 = 0;
  *(uint32_t*)0x200009f8 = 0;
  *(uint32_t*)0x200009fc = 0;
  *(uint32_t*)0x20000a00 = 0;
  *(uint32_t*)0x20000a04 = 0;
  *(uint32_t*)0x20000a08 = 0;
  *(uint32_t*)0x20000a0c = 0;
  *(uint32_t*)0x20000a10 = 0;
  *(uint32_t*)0x20000a14 = 0;
  *(uint32_t*)0x20000a18 = 0;
  *(uint32_t*)0x20000a1c = 0;
  *(uint32_t*)0x20000a20 = 0;
  *(uint32_t*)0x20000a24 = 0;
  *(uint32_t*)0x20000a28 = 0;
  *(uint32_t*)0x20000a2c = 0;
  *(uint32_t*)0x20000a30 = 0;
  *(uint32_t*)0x20000a34 = 0;
  *(uint32_t*)0x20000a38 = 0;
  *(uint32_t*)0x20000a3c = 0;
  *(uint32_t*)0x20000a40 = 0;
  *(uint32_t*)0x20000a44 = 0;
  *(uint32_t*)0x20000a48 = 0;
  *(uint32_t*)0x20000a4c = 0;
  *(uint32_t*)0x20000a50 = 0;
  *(uint32_t*)0x20000a54 = 0;
  *(uint32_t*)0x20000a58 = 0;
  *(uint32_t*)0x20000a5c = 0;
  *(uint32_t*)0x20000a60 = 0;
  *(uint32_t*)0x20000a64 = 0;
  *(uint32_t*)0x20000a68 = 0;
  *(uint32_t*)0x20000a6c = 0;
  *(uint32_t*)0x20000a70 = 0;
  *(uint32_t*)0x20000a74 = 0;
  *(uint32_t*)0x20000a78 = 0;
  *(uint32_t*)0x20000a7c = 0;
  *(uint32_t*)0x20000a80 = 0;
  *(uint32_t*)0x20000a84 = 0;
  *(uint32_t*)0x20000a88 = 0;
  *(uint32_t*)0x20000a8c = 0;
  *(uint32_t*)0x20000a90 = 0;
  *(uint32_t*)0x20000a94 = 0;
  *(uint32_t*)0x20000a98 = 0;
  *(uint32_t*)0x20000a9c = 0;
  *(uint32_t*)0x20000aa0 = 0;
  *(uint32_t*)0x20000aa4 = 0;
  *(uint32_t*)0x20000aa8 = 0;
  *(uint32_t*)0x20000aac = 0;
  *(uint32_t*)0x20000ab0 = 0;
  *(uint32_t*)0x20000ab4 = 0;
  *(uint32_t*)0x20000ab8 = 0;
  *(uint32_t*)0x20000abc = 0;
  *(uint32_t*)0x20000ac0 = 0;
  *(uint32_t*)0x20000ac4 = 0;
  *(uint32_t*)0x20000ac8 = 0;
  *(uint32_t*)0x20000acc = 0;
  *(uint32_t*)0x20000ad0 = 0;
  *(uint32_t*)0x20000ad4 = 0;
  *(uint32_t*)0x20000ad8 = 0;
  *(uint32_t*)0x20000adc = 0;
  *(uint32_t*)0x20000ae0 = 0;
  *(uint32_t*)0x20000ae4 = 0;
  *(uint32_t*)0x20000ae8 = 0;
  *(uint32_t*)0x20000aec = 0;
  *(uint32_t*)0x20000af0 = 0;
  *(uint32_t*)0x20000af4 = 0;
  *(uint32_t*)0x20000af8 = 0;
  *(uint32_t*)0x20000afc = 0;
  *(uint32_t*)0x20000b00 = 0;
  *(uint32_t*)0x20000b04 = 0;
  *(uint32_t*)0x20000b08 = 0;
  *(uint32_t*)0x20000b0c = 0;
  *(uint32_t*)0x20000b10 = 0;
  *(uint32_t*)0x20000b14 = 0;
  *(uint32_t*)0x20000b18 = 0;
  *(uint32_t*)0x20000b1c = 0;
  *(uint32_t*)0x20000b20 = 0;
  *(uint32_t*)0x20000b24 = 0;
  *(uint32_t*)0x20000b28 = 0;
  *(uint32_t*)0x20000b2c = 0;
  *(uint32_t*)0x20000b30 = 0;
  *(uint32_t*)0x20000b34 = 0;
  *(uint32_t*)0x20000b38 = 0;
  *(uint32_t*)0x20000b3c = 0;
  *(uint32_t*)0x20000b40 = 0;
  *(uint32_t*)0x20000b44 = 0;
  *(uint32_t*)0x20000b48 = 0;
  *(uint32_t*)0x20000b4c = 0;
  *(uint32_t*)0x20000b50 = 0;
  *(uint32_t*)0x20000b54 = 0;
  *(uint32_t*)0x20000b58 = 0;
  *(uint32_t*)0x20000b5c = 0;
  *(uint32_t*)0x20000b60 = 0;
  *(uint32_t*)0x20000b64 = 0;
  *(uint32_t*)0x20000b68 = 0;
  *(uint32_t*)0x20000b6c = 0;
  *(uint32_t*)0x20000b70 = 0;
  *(uint32_t*)0x20000b74 = 0;
  *(uint32_t*)0x20000b78 = 0;
  *(uint32_t*)0x20000b7c = 0;
  *(uint32_t*)0x20000b80 = 0;
  *(uint32_t*)0x20000b84 = 0;
  *(uint32_t*)0x20000b88 = 0;
  *(uint32_t*)0x20000b8c = 0;
  *(uint32_t*)0x20000b90 = 0;
  *(uint32_t*)0x20000b94 = 0;
  *(uint32_t*)0x20000b98 = 0;
  *(uint32_t*)0x20000b9c = 0;
  *(uint32_t*)0x20000ba0 = 0;
  *(uint32_t*)0x20000ba4 = 0;
  *(uint32_t*)0x20000ba8 = 0;
  *(uint32_t*)0x20000bac = 0;
  *(uint32_t*)0x20000bb0 = 0;
  *(uint32_t*)0x20000bb4 = 0;
  *(uint32_t*)0x20000bb8 = 0;
  *(uint32_t*)0x20000bbc = 0;
  *(uint32_t*)0x20000bc0 = 0;
  *(uint32_t*)0x20000bc4 = 0;
  *(uint32_t*)0x20000bc8 = 0;
  *(uint32_t*)0x20000bcc = 0;
  *(uint32_t*)0x20000bd0 = 0;
  *(uint32_t*)0x20000bd4 = 0;
  *(uint32_t*)0x20000bd8 = 0;
  *(uint32_t*)0x20000bdc = 0;
  *(uint32_t*)0x20000be0 = 0;
  *(uint32_t*)0x20000be4 = 0;
  *(uint32_t*)0x20000be8 = 0;
  *(uint32_t*)0x20000bec = 0;
  *(uint32_t*)0x20000bf0 = 0;
  *(uint32_t*)0x20000bf4 = 0;
  *(uint32_t*)0x20000bf8 = 0;
  *(uint32_t*)0x20000bfc = 0;
  *(uint32_t*)0x20000c00 = 0;
  *(uint32_t*)0x20000c04 = 0;
  *(uint32_t*)0x20000c08 = 0;
  *(uint32_t*)0x20000c0c = 0;
  *(uint32_t*)0x20000c10 = 0;
  *(uint32_t*)0x20000c14 = 0;
  *(uint32_t*)0x20000c18 = 0;
  *(uint32_t*)0x20000c1c = 0;
  *(uint32_t*)0x20000c20 = 0;
  *(uint32_t*)0x20000c24 = 0;
  *(uint32_t*)0x20000c28 = 0;
  *(uint32_t*)0x20000c2c = 0;
  *(uint32_t*)0x20000c30 = 0;
  *(uint32_t*)0x20000c34 = 0;
  *(uint32_t*)0x20000c38 = 0;
  *(uint32_t*)0x20000c3c = 0;
  *(uint32_t*)0x20000c40 = 0;
  *(uint32_t*)0x20000c44 = 0;
  *(uint32_t*)0x20000c48 = 0;
  *(uint32_t*)0x20000c4c = 0;
  *(uint32_t*)0x20000c50 = 0;
  *(uint32_t*)0x20000c54 = 0;
  *(uint32_t*)0x20000c58 = 0;
  *(uint32_t*)0x20000c5c = 0;
  *(uint32_t*)0x20000c60 = 0;
  *(uint32_t*)0x20000c64 = 0;
  *(uint32_t*)0x20000c68 = 0;
  *(uint32_t*)0x20000c6c = 0;
  *(uint32_t*)0x20000c70 = 0;
  *(uint32_t*)0x20000c74 = 0;
  *(uint32_t*)0x20000c78 = 0xa;
  *(uint32_t*)0x20000c7c = 0;
  *(uint32_t*)0x20000c80 = 0;
  *(uint8_t*)0x20000c84 = 0xac;
  *(uint8_t*)0x20000c85 = 0x14;
  *(uint8_t*)0x20000c86 = -1;
  *(uint8_t*)0x20000c87 = 0xbb;
  *(uint32_t*)0x20000c88 = 0;
  *(uint32_t*)0x20000c8c = 0;
  *(uint32_t*)0x20000c90 = htobe32(0xe0000001);
  *(uint32_t*)0x20000c94 = 0;
  *(uint32_t*)0x20000c98 = 0;
  *(uint8_t*)0x20000c9c = 0xac;
  *(uint8_t*)0x20000c9d = 0x14;
  *(uint8_t*)0x20000c9e = -1;
  *(uint8_t*)0x20000c9f = 0xaa;
  *(uint32_t*)0x20000ca0 = 0;
  *(uint32_t*)0x20000ca4 = 0;
  *(uint32_t*)0x20000ca8 = htobe32(0x7f000001);
  *(uint32_t*)0x20000cac = 0;
  *(uint32_t*)0x20000cb0 = 0;
  *(uint32_t*)0x20000cb4 = htobe32(0);
  *(uint32_t*)0x20000cb8 = 0;
  *(uint32_t*)0x20000cbc = 0;
  *(uint32_t*)0x20000cc0 = htobe32(0x7f000001);
  *(uint32_t*)0x20000cc4 = 0;
  *(uint32_t*)0x20000cc8 = 0;
  *(uint32_t*)0x20000ccc = htobe32(0xe0000001);
  *(uint32_t*)0x20000cd0 = 0;
  *(uint32_t*)0x20000cd4 = 0;
  *(uint32_t*)0x20000cd8 = htobe32(0xe0000002);
  *(uint32_t*)0x20000cdc = 0;
  *(uint32_t*)0x20000ce0 = 0;
  *(uint32_t*)0x20000ce4 = htobe32(-1);
  *(uint32_t*)0x20000ce8 = 0;
  *(uint32_t*)0x20000cec = 0;
  *(uint32_t*)0x20000cf0 = htobe32(-1);
  memcpy((void*)0x20000cf8, "\x63\x6f\x6e\x6e\x6c\x61\x62\x65\x6c\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20000d18 = 8;
  *(uint16_t*)0x20000d20 = 0;
  *(uint16_t*)0x20000d22 = 0;
  memcpy((void*)0x20000d28, "\x4c\x45\x44\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20000d48 = 0x28;
  memcpy((void*)0x20000d50, "\x73\x79\x7a\x30\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00",
         27);
  *(uint8_t*)0x20000d6b = 0;
  *(uint32_t*)0x20000d6c = 0;
  *(uint64_t*)0x20000d70 = 0;
  *(uint32_t*)0x20000d78 = 0;
  memcpy((void*)0x20000d7c, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20000d9c = 2;
  *(uint32_t*)0x20000da0 = -1;
  *(uint32_t*)0x20000da4 = 0;
  *(uint32_t*)0x20000da8 = 0;
  memcpy((void*)0x20000dac, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                            "\x00\x00\x00\x00\x00\x00\x00\x00",
         32);
  *(uint32_t*)0x20000dcc = 2;
  *(uint32_t*)0x20000dd0 = 0xfffffffe;
  *(uint32_t*)0x20000dd4 = 0;
  syscall(__NR_setsockopt, r[0], 0, 0x80, 0x20000140, 0xc10);
}

int main()
{
  syscall(__NR_mmap, 0x20000000, 0x1000000, 3, 0x32, -1, 0);
  int pid = do_sandbox_none();
  int status = 0;
  while (waitpid(pid, &status, __WALL) != pid) {
  }
  return 0;
}
