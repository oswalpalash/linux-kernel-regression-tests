// https://syzkaller.appspot.com/bug?id=f8d4f67fe01399591448f63f31ffe3bc49116c06
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <pthread.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include <string.h>

static __thread int skip_segv;
static __thread jmp_buf segv_env;

static void segv_handler(int sig, siginfo_t* info, void* uctx)
{
  uintptr_t addr = (uintptr_t)info->si_addr;
  const uintptr_t prog_start = 1 << 20;
  const uintptr_t prog_end = 100 << 20;
  if (__atomic_load_n(&skip_segv, __ATOMIC_RELAXED) &&
      (addr < prog_start || addr > prog_end)) {
    _longjmp(segv_env, 1);
  }
  doexit(sig);
  for (;;) {
  }
}

static void install_segv_handler()
{
  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = SIG_IGN;
  syscall(SYS_rt_sigaction, 0x20, &sa, NULL, 8);
  syscall(SYS_rt_sigaction, 0x21, &sa, NULL, 8);

  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_NODEFER | SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
}

#define NONFAILING(...)                                                \
  {                                                                    \
    __atomic_fetch_add(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
    if (_setjmp(segv_env) == 0) {                                      \
      __VA_ARGS__;                                                     \
    }                                                                  \
    __atomic_fetch_sub(&skip_segv, 1, __ATOMIC_SEQ_CST);               \
  }

static void test();

void loop()
{
  while (1) {
    test();
  }
}

long r[189];
void* thr(void* arg)
{
  switch ((long)arg) {
  case 0:
    r[0] = syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
                   0xfffffffffffffffful, 0x0ul);
    break;
  case 1:
    r[1] = syscall(__NR_socket, 0x26ul, 0x5ul, 0x0ul);
    break;
  case 2:
    NONFAILING(*(uint16_t*)0x20939fa8 = (uint16_t)0x26);
    NONFAILING(memcpy(
        (void*)0x20939faa,
        "\x61\x65\x61\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        14));
    NONFAILING(*(uint32_t*)0x20939fb8 = (uint32_t)0x0);
    NONFAILING(*(uint32_t*)0x20939fbc = (uint32_t)0x0);
    NONFAILING(memcpy((void*)0x20939fc0,
                      "\x72\x66\x63\x34\x35\x34\x33\x28\x67\x63\x6d\x28"
                      "\x61\x65\x73\x29\x29\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                      "\x00\x00\x00\x00",
                      64));
    r[7] = syscall(__NR_bind, r[1], 0x20939fa8ul, 0x58ul);
    break;
  case 3:
    NONFAILING(memcpy((void*)0x20890fec, "\x79\x73\x39\x76\xd8\xc3\x0a"
                                         "\x4a\xea\xd9\x60\x10\xbe\x13"
                                         "\xcf\xe6\x27\x85\xce\x10",
                      20));
    r[9] = syscall(__NR_setsockopt, r[1], 0x117ul, 0x1ul, 0x20890fecul,
                   0x14ul);
    break;
  case 4:
    r[10] = syscall(__NR_accept, r[1], 0x0ul, 0x0ul);
    break;
  case 5:
    NONFAILING(*(uint64_t*)0x20e5b000 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20e5b008 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20e5b010 = (uint64_t)0x20bbe000);
    NONFAILING(*(uint64_t*)0x20e5b018 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20e5b020 = (uint64_t)0x203aa000);
    NONFAILING(*(uint64_t*)0x20e5b028 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20e5b030 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x203aa000 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x203aa008 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x203aa00c = (uint32_t)0x3);
    NONFAILING(*(uint32_t*)0x203aa010 = (uint32_t)0x1);
    r[22] = syscall(__NR_sendmmsg, r[10], 0x20e5b000ul, 0x1ul, 0x0ul);
    break;
  case 6:
    NONFAILING(*(uint64_t*)0x20fd9fc8 = (uint64_t)0x202fc000);
    NONFAILING(*(uint32_t*)0x20fd9fd0 = (uint32_t)0x9);
    NONFAILING(*(uint64_t*)0x20fd9fd8 = (uint64_t)0x20f7dfc0);
    NONFAILING(*(uint64_t*)0x20fd9fe0 = (uint64_t)0x1);
    NONFAILING(*(uint64_t*)0x20fd9fe8 = (uint64_t)0x20aa5000);
    NONFAILING(*(uint64_t*)0x20fd9ff0 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20fd9ff8 = (uint32_t)0x4);
    NONFAILING(*(uint64_t*)0x20f7dfc0 = (uint64_t)0x20249000);
    NONFAILING(*(uint64_t*)0x20f7dfc8 = (uint64_t)0x480);
    r[32] = syscall(__NR_recvmsg, r[10], 0x20fd9fc8ul, 0x2002ul);
    break;
  case 7:
    NONFAILING(*(uint64_t*)0x20e78000 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20e78008 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20e78010 = (uint64_t)0x205dbf80);
    NONFAILING(*(uint64_t*)0x20e78018 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20e78020 = (uint64_t)0x20b7cfe8);
    NONFAILING(*(uint64_t*)0x20e78028 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20e78030 = (uint32_t)0x4001);
    NONFAILING(*(uint64_t*)0x20e78038 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20e78040 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20e78048 = (uint64_t)0x20d51ff0);
    NONFAILING(*(uint64_t*)0x20e78050 = (uint64_t)0x1);
    NONFAILING(*(uint64_t*)0x20e78058 = (uint64_t)0x200fd000);
    NONFAILING(*(uint64_t*)0x20e78060 = (uint64_t)0xd8);
    NONFAILING(*(uint32_t*)0x20e78068 = (uint32_t)0x4);
    NONFAILING(*(uint64_t*)0x20e78070 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20e78078 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20e78080 = (uint64_t)0x20a9dfe0);
    NONFAILING(*(uint64_t*)0x20e78088 = (uint64_t)0x2);
    NONFAILING(*(uint64_t*)0x20e78090 = (uint64_t)0x200261eb);
    NONFAILING(*(uint64_t*)0x20e78098 = (uint64_t)0x30);
    NONFAILING(*(uint32_t*)0x20e780a0 = (uint32_t)0x20008040);
    NONFAILING(*(uint64_t*)0x20e780a8 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20e780b0 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20e780b8 = (uint64_t)0x20de3000);
    NONFAILING(*(uint64_t*)0x20e780c0 = (uint64_t)0x7);
    NONFAILING(*(uint64_t*)0x20e780c8 = (uint64_t)0x20e4dfb8);
    NONFAILING(*(uint64_t*)0x20e780d0 = (uint64_t)0x48);
    NONFAILING(*(uint32_t*)0x20e780d8 = (uint32_t)0x20000000);
    NONFAILING(*(uint64_t*)0x20e780e0 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20e780e8 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20e780f0 = (uint64_t)0x20358fd0);
    NONFAILING(*(uint64_t*)0x20e780f8 = (uint64_t)0x3);
    NONFAILING(*(uint64_t*)0x20e78100 = (uint64_t)0x20000000);
    NONFAILING(*(uint64_t*)0x20e78108 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20e78110 = (uint32_t)0x8001);
    NONFAILING(*(uint64_t*)0x20e78118 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20e78120 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20e78128 = (uint64_t)0x2069d000);
    NONFAILING(*(uint64_t*)0x20e78130 = (uint64_t)0x3);
    NONFAILING(*(uint64_t*)0x20e78138 = (uint64_t)0x20d34000);
    NONFAILING(*(uint64_t*)0x20e78140 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20e78148 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20e78150 = (uint64_t)0x0);
    NONFAILING(*(uint32_t*)0x20e78158 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20e78160 = (uint64_t)0x209bfff0);
    NONFAILING(*(uint64_t*)0x20e78168 = (uint64_t)0x1);
    NONFAILING(*(uint64_t*)0x20e78170 = (uint64_t)0x207ab000);
    NONFAILING(*(uint64_t*)0x20e78178 = (uint64_t)0x30);
    NONFAILING(*(uint32_t*)0x20e78180 = (uint32_t)0x8004);
    NONFAILING(*(uint64_t*)0x20b7cfe8 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20b7cff0 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x20b7cff4 = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x20b7cff8 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20d51ff0 = (uint64_t)0x2098af09);
    NONFAILING(*(uint64_t*)0x20d51ff8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x200fd000 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x200fd008 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x200fd00c = (uint32_t)0x3);
    NONFAILING(*(uint32_t*)0x200fd010 = (uint32_t)0x1);
    NONFAILING(*(uint64_t*)0x200fd018 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x200fd020 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x200fd024 = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x200fd028 = (uint32_t)0x7);
    NONFAILING(*(uint64_t*)0x200fd030 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x200fd038 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x200fd03c = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x200fd040 = (uint32_t)0x401);
    NONFAILING(*(uint64_t*)0x200fd048 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x200fd050 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x200fd054 = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x200fd058 = (uint32_t)0x8);
    NONFAILING(*(uint64_t*)0x200fd060 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x200fd068 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x200fd06c = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x200fd070 = (uint32_t)0x3);
    NONFAILING(*(uint64_t*)0x200fd078 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x200fd080 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x200fd084 = (uint32_t)0x3);
    NONFAILING(*(uint32_t*)0x200fd088 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x200fd090 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x200fd098 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x200fd09c = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x200fd0a0 = (uint32_t)0x7f);
    NONFAILING(*(uint64_t*)0x200fd0a8 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x200fd0b0 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x200fd0b4 = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x200fd0b8 = (uint32_t)0xffffffff);
    NONFAILING(*(uint64_t*)0x200fd0c0 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x200fd0c8 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x200fd0cc = (uint32_t)0x3);
    NONFAILING(*(uint32_t*)0x200fd0d0 = (uint32_t)0x1);
    NONFAILING(*(uint64_t*)0x20a9dfe0 = (uint64_t)0x204b8000);
    NONFAILING(*(uint64_t*)0x20a9dfe8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20a9dff0 = (uint64_t)0x20d50000);
    NONFAILING(*(uint64_t*)0x20a9dff8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x200261eb = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x200261f3 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x200261f7 = (uint32_t)0x2);
    NONFAILING(*(uint32_t*)0x200261fb = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20026203 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x2002620b = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x2002620f = (uint32_t)0x2);
    NONFAILING(*(uint32_t*)0x20026213 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x20de3000 = (uint64_t)0x20c47000);
    NONFAILING(*(uint64_t*)0x20de3008 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20de3010 = (uint64_t)0x20f62f72);
    NONFAILING(*(uint64_t*)0x20de3018 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20de3020 = (uint64_t)0x207b8f6f);
    NONFAILING(*(uint64_t*)0x20de3028 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20de3030 = (uint64_t)0x206dc000);
    NONFAILING(*(uint64_t*)0x20de3038 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20de3040 = (uint64_t)0x20936000);
    NONFAILING(*(uint64_t*)0x20de3048 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20de3050 = (uint64_t)0x20de4f99);
    NONFAILING(*(uint64_t*)0x20de3058 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20de3060 = (uint64_t)0x2097e000);
    NONFAILING(*(uint64_t*)0x20de3068 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20e4dfb8 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20e4dfc0 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x20e4dfc4 = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x20e4dfc8 = (uint32_t)0x7);
    NONFAILING(*(uint64_t*)0x20e4dfd0 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20e4dfd8 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x20e4dfdc = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x20e4dfe0 = (uint32_t)0x1fffe0000000);
    NONFAILING(*(uint64_t*)0x20e4dfe8 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20e4dff0 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x20e4dff4 = (uint32_t)0x3);
    NONFAILING(*(uint32_t*)0x20e4dff8 = (uint32_t)0x1);
    NONFAILING(*(uint64_t*)0x20358fd0 = (uint64_t)0x20ca7000);
    NONFAILING(*(uint64_t*)0x20358fd8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20358fe0 = (uint64_t)0x20d0ef7a);
    NONFAILING(*(uint64_t*)0x20358fe8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20358ff0 = (uint64_t)0x20319f36);
    NONFAILING(*(uint64_t*)0x20358ff8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x2069d000 = (uint64_t)0x20023000);
    NONFAILING(*(uint64_t*)0x2069d008 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x2069d010 = (uint64_t)0x20f6c000);
    NONFAILING(*(uint64_t*)0x2069d018 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x2069d020 = (uint64_t)0x20a6a000);
    NONFAILING(*(uint64_t*)0x2069d028 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x20d34000 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x20d34008 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x20d3400c = (uint32_t)0x4);
    NONFAILING(*(uint32_t*)0x20d34010 = (uint32_t)0x2);
    NONFAILING(*(uint64_t*)0x209bfff0 = (uint64_t)0x20dcf000);
    NONFAILING(*(uint64_t*)0x209bfff8 = (uint64_t)0x0);
    NONFAILING(*(uint64_t*)0x207ab000 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x207ab008 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x207ab00c = (uint32_t)0x3);
    NONFAILING(*(uint32_t*)0x207ab010 = (uint32_t)0x0);
    NONFAILING(*(uint64_t*)0x207ab018 = (uint64_t)0x18);
    NONFAILING(*(uint32_t*)0x207ab020 = (uint32_t)0x117);
    NONFAILING(*(uint32_t*)0x207ab024 = (uint32_t)0x3);
    NONFAILING(*(uint32_t*)0x207ab028 = (uint32_t)0x0);
    r[188] = syscall(__NR_sendmmsg, r[10], 0x20e78000ul, 0x7ul, 0x4ul);
    break;
  }
  return 0;
}

void test()
{
  long i;
  pthread_t th[16];

  memset(r, -1, sizeof(r));
  for (i = 0; i < 8; i++) {
    pthread_create(&th[i], 0, thr, (void*)i);
    usleep(rand() % 10000);
  }
  usleep(rand() % 100000);
}

int main()
{
  int i;
  for (i = 0; i < 8; i++) {
    if (fork() == 0) {
      install_segv_handler();
      loop();
      return 0;
    }
  }
  sleep(1000000);
  return 0;
}
