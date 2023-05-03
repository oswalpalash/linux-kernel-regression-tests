// https://syzkaller.appspot.com/bug?id=a8d38d1b68ffc744c53bd9b9fc1dbd6c86b1afe2
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE
#include <endian.h>
#include <linux/futex.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
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

struct thread_t {
  int created, running, call;
  pthread_t th;
};

static struct thread_t threads[16];
static void execute_call(int call);
static int running;

static void* thr(void* arg)
{
  struct thread_t* th = (struct thread_t*)arg;
  for (;;) {
    while (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE))
      syscall(SYS_futex, &th->running, FUTEX_WAIT, 0, 0);
    execute_call(th->call);
    __atomic_fetch_sub(&running, 1, __ATOMIC_RELAXED);
    __atomic_store_n(&th->running, 0, __ATOMIC_RELEASE);
    syscall(SYS_futex, &th->running, FUTEX_WAKE);
  }
  return 0;
}

static void execute(int num_calls)
{
  int call, thread;
  running = 0;
  for (call = 0; call < num_calls; call++) {
    for (thread = 0; thread < sizeof(threads) / sizeof(threads[0]); thread++) {
      struct thread_t* th = &threads[thread];
      if (!th->created) {
        th->created = 1;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 << 10);
        pthread_create(&th->th, &attr, thr, th);
      }
      if (!__atomic_load_n(&th->running, __ATOMIC_ACQUIRE)) {
        th->call = call;
        __atomic_fetch_add(&running, 1, __ATOMIC_RELAXED);
        __atomic_store_n(&th->running, 1, __ATOMIC_RELEASE);
        syscall(SYS_futex, &th->running, FUTEX_WAKE);
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 20 * 1000 * 1000;
        syscall(SYS_futex, &th->running, FUTEX_WAIT, 1, &ts);
        if (running)
          usleep((call == num_calls - 1) ? 10000 : 1000);
        break;
      }
    }
  }
}

#ifndef __NR_listen
#define __NR_listen 363
#endif
#ifndef __NR_accept4
#define __NR_accept4 364
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_socket
#define __NR_socket 359
#endif
#ifndef __NR_sendto
#define __NR_sendto 369
#endif
#ifndef __NR_sendmmsg
#define __NR_sendmmsg 345
#endif
#ifndef __NR_setsockopt
#define __NR_setsockopt 366
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

long r[1];
void execute_call(int call)
{
  switch (call) {
  case 0:
    syscall(__NR_mmap, 0x20000000, 0xfff000, 3, 0x32, -1, 0);
    break;
  case 1:
    r[0] = syscall(__NR_socket, 0xa, 1, 0x84);
    break;
  case 2:
    memcpy(
        (void*)0x20bc2000,
        "\x81\x90\xe2\xb1\x80\xb2\xe9\x16\x0f\x8f\xab\x58\xf3\x7d\x7f\xc9\x59"
        "\x53\x35\x05\x53\xf5\xf8\x6d\x21\x44\xd9\xb7\x88\xcd\xe2\x8b\xbf\xe4"
        "\x1f\xd7\xcf\xeb\x66\xd2\xa8\x36\x00\x30\x77\xf7\xd0\x81\x44\xdb\x1d"
        "\x41\xfa\x7f\x12\x55\xdf\xf1\xfe\x48\xd0\x81\xfa\xdc\x68\xa9\x3a\x7a"
        "\xdd\xe6\x02\xd9\xd0\x10\x34\x19\x9e\xbd\x5e\x4b\x0e\xd5\x35\x8a\x45"
        "\x4c\xc0\x91\x6d\x6c\x0d\x3f\x00\xa7\x06\x43\x05\xcc\xd4\x6b\xe9\x55"
        "\x3b\x97\x82\x43\xab\x9b\x8e\x61\x08\xf5\x0d\x9c\xe9\x8a\x06\x45\x92"
        "\xb4\xa1\xe4\xc6\x5d\x17\x8e\xd2\x79\x71\x63\x9e\xe2\x61\x11\x47\xce"
        "\x5f\x1a\x3f\x1d\xdf\x21\x3f\xe4\xdb\xb0\xe1\xa9\xe4\xf3\x07\xb8\x0c"
        "\xe8\x53\xea\xd9\x42\x04\x9f\x8c\x56\xdc\x2b\x88\x0e\xb0\xe9\xe6\x6c"
        "\x67\x9e\x2b\x5c\x22\x99\xbd\x20\xb3\x94\x33\x03\x3a\xe7\x11\x9f\xc6"
        "\x1b\xaa\xab\x9e\x67\xa2\xd1\xb2\x4c\x8f\x41\x6f\x2d\xa9\xa7\x32\x4e"
        "\x45\x6a\x12\x30\xef\xea\xde\x28\xf8\x9d\x03\xcd\xee\x5d\x86\x7d\x15"
        "\x5e\x23\x2b\xb8\x44\x75\x12\xe4\xa9\xba\xee\x76\xfd\x43\x6a\xcd\x56"
        "\x8f\x28\x5e\x08\xfe\xde\x7e\x55\x6b\x7e\x6f\x67\x4c\x34\xbb\xc1\x15"
        "\xec\x3d\x94\xd3\x92\x8a\x47\xef\x03\xac\x3e\x29\xa0\xbd\xb3\xd5\xd9"
        "\xeb\xa7\xbb\xa7\xd9\xfd\x25\xe7\x6d\xae\xe4\x80\xf3\x98\x31\xfa\xbb"
        "\xed\xdd\x3c\xd2\x29\x1d\x11\xba\x8a\x2f\xda\xfd\x03\x87\x91\x9e\x29"
        "\x30\xba\x5e\x91\x55\x28\x11\xb3\xb9\x02\x52\x42\x0a\x66\x08\xfb\xe9"
        "\x3e\xad\x3e\xe6\x6a\x92\x83\xff\xb1\x09\xb3\x20\x59\xa9\xe2\xfc\xe7"
        "\x7f\xff\xef\xdf\xb7\xd9\x9b\x6a\x61\x88\x7e\x32\x0b\xc0\x19\xd2\x46"
        "\x3c\x1a\x66\x50\x88\xbe\x9c\xa6\xb9\xf8\xc2\xfe\x56\xe5\xb8\x61\xcd"
        "\x6f\x48\x97\xa1\x38\x6d\x29\x4c\x73\xe1\xc5\x35\x0c\xa7\x4b\x43\x28"
        "\x0f\x89\x8a\x30\xca\xc1\xec\x91\xea\xd0\x92\x12\x3d\xe6\xf2\xb6\xe9"
        "\x16\x5d\xa8\x4e\xc6\x0f\x1c\x68\x2d\x72\x88\xa7\x58\x19\x77\x9f\x6b"
        "\x07\x1d\xbf\x08\x0b\xee\xe6\x40\x82\x68\xe8\x76\xad\xf4\x9f\xa4\x12"
        "\x31\xbf\xb9\x07\x75\x5e\x1f\x2a\x9e\xa1\x59\xb1\xc0\x90\x32\xe7\x97"
        "\x04\x61\x39\x54\x7c\x0f\xb8\x81\x67\xa7\x78\x0d\x68\x91\x06\xe0\x30"
        "\x3a\x2c\x1c\x9f\x05\x77\x76\xb4\xf7\xbd\x62\x06\x4f\x72\xae\x1a\x08"
        "\x0e\x39\xe5\xa1\xd4\x67\xf7\x74\x29\x8f\x75\x5f\x64\xc4\x51\xf9\xe0"
        "\x1a\xed\x89\x6e\xd8\x4a\xb1\xac\x72\xb1\x44\x8b\xef\x14\x95\x77\x45"
        "\x73\xf3\x01\x31\xf3\xb0\x2a\x35\xf7\x42\x22\x43\x02\x51\xb5\xb1\x24"
        "\x6c\xde\xf5\xb7\x53\x5f\x3d\x32\xd5\xae\x31\x5d\x2a\x0a\x8f\x34\x85"
        "\xd0\x7c\x25\x7d\x3f\x3d\xa0\x0e\xf2\xa2\x76\xdb\x61\x53\x7e\x8e\x0b"
        "\x52\x0a\x1b\x1c\x06\x9d\x13\x68\x2a\x73\x76\x7e\xad\x8a\x83\x90\x48"
        "\x73\xef\xf3\xb4\x27\xf7\xc2\xfe\x6e\x59\x19\x6a\xa2\xe9\x8b\x94\x7c"
        "\x99\x68\x16\xfe\x6f\x62\x32\x8f\xe8\xe0\x52\xd8\x86\x41\xc5\x6d\xc8"
        "\x3e\xaa\x3b\xab\x29\x3f\x46\x69\x88\x45\xb7\xa7\x1e\x2d\xb4\xa4\xc8"
        "\x9f\x18\x7e\x97\xd2\xaf\x64\xf9\xce\xb8\xbc\xe2\x77\x07\x27\x5d\x98"
        "\x07\x01\x87\xd3\x00\x4b\xbc\xc8\xf9\x94\x0b\x0c\xef\x24\xe9\xcb\x87"
        "\xfa\x4a\x77\x47\x0b\x24\x5a\x2a\x3d\x63\xfc\xcd\xf3\x18\xab\xe4\x0c"
        "\x78\x47\x1e\x66\x0f\x4e\xf3\xb3\x15\xed\x51\x3e\xb2\x44\x76\xe3\x4f"
        "\xad\xc0\x20\xc2\xfd\x82\xfa\x08\xb4\xa5\xe6\xeb\x65\x62\x75\xf8\x45"
        "\xaf\x92\x34\xb3\xc5\xdf\xad\x9a\xe3\x78\xee\x46\x6d\x83\xd0\x24\xbf"
        "\x52\x79\x11\xf7\x4f\xe7\x21\x5a\xa7\xfe\x75\x6e\xee\xa2\x5d\x88\x5c"
        "\x0c\xae\xc5\x3b\x7d\xff\xf0\x06\x25\x13\x75\xe2\xc7\x03\xf0\xae\x1b"
        "\x79\xd8\x0e\x3d\x03\x0b\x4e\xec\x94\x84\x51\x0a\xa6\x99\xbf\x34\xde"
        "\xe8\x45\xa6\x4e\xc0\x34\x20\x58\xdf\x9f\x74\x71\x89\x6c\x64\xe6\xa1"
        "\x0e\x4b\x07\xed\x55\x56\x2d\x2c\x90\x5b\x2b\x47\xbc\x4d\x36\x47\x79"
        "\xfb\xe2\x60\xce\x38\x2d\xb4\x9d\x52\x5c\x70\xcd\x6b\x8a\xb1\x86\x8c"
        "\xe9\xd0\x60\x87\x9b\x75\x04\x5f\x9d\xe6\x34\x8c\x6a\xea\x1e\x57\x22"
        "\xd2\xbb\xde\xf6\x60\x65\x20\x63\xbb\x21\x2e\xf2\x55\x4a\xa5\x32\x6d"
        "\x79\xbe\x50\x83\xac\x53\xe3\x25\x28\x1a\xdb\x0d\xc5\x63\x7e\x98\x97"
        "\xe6\x4f\x64\xc5\xdb\x04\x3d\x4a\xf1\x1a\xe4\xe1\xd7\xd8\x8b\x46\xf9"
        "\xf0\xb0\xcb\x38\x99\x24\xf2\x6f\x60\xd6\xb3\xa5\x21\x24\x45\x0e\xc8"
        "\x2b\x8a\x00\x38\xdc\xb3\x6c\x15\x72\xdc\x90\xf3\x8b\xef\x14\xc8\x83"
        "\x60\x96\xb5\xd8\xcf\x8a\x57\x66\xea\x57\x35\x42\x0a\x0d\x7f\xfb\x6c"
        "\xca\x15\xed\xd8\x5f\xd1\x4d\x4a\x98\xce\x4f\x1e\xad\xfb\xdb\x18\x1b"
        "\xbd\xd6\x0c\x94\xa3\xc3\xb7\x07\xb0\xff\xe8\x0e\xfb\xef\xed\xa5\xc3"
        "\x07\x9c\x23\x1f\x84\x57\xf1\xeb\x81\xd5\x2f\xd8\x5d\xcb\x2b\x1f\xeb"
        "\xce\x6e\xe2\x3f\x69\xd8\x8e\x79\x74\x1e\x9e\xe1\x75\x0c\x61\x66\x93"
        "\x4b\xf3\xef\x48\xa4\x66\x76\x9d\xfa\xc5\xb7\x88\x13\x86\xbc\xf7\x23"
        "\x7a\x99\x02\xad\xde\x5d\xbc\x0f\x7d\xc6\x6c\xe9\x0e\x08\xd3\x14\xb0"
        "\xed\xa1\xc7\x0e\xf5\x39\x42\x88\xe9\xe5\x7f\xa5\xac\xb6\x89\x03\xec"
        "\x18\x85\xff\xfe\xaa\xb7\x5c\x7c\x9f\x3e\x76\x33\x56\xf5\x33\xd7\x1b"
        "\xe0\xf3\x57\x8a\x5e\xae\x8d\x19\x62\xa4\x55\x50\xe7\x59\x13\x20\x5e"
        "\xd6\x9b\x7d\xd2\x38\xdf\x01\xf4\x74\x33\x5d\x7a\xff\x5e\xeb\x50\x4b"
        "\xd3\xfd\xf1\xd6\x39\x83\xa8\x22\xb4\x71\xd2\x53\x5f\x33\x3f\x5d\x86"
        "\xcd\xb7\xf1\x25\x31\xde\xb0\x0e\x30\xb7\x4e\x60\xde\x42\x88\xfc\xd7"
        "\xd5\x86\x5e\x57\x68\x52\xe0\x9f\x9d\x6f\xa8\xef\x33\xe2\xf3\xa8\x4c"
        "\x96\x04\x19\xf6\xab\x0f\xa2\x9f\xfa\xe4\x66\xfb\x19\x10\x17\xf9\x35"
        "\xbf\xed\x88\x6e\x92\xf6\x1d\x0f\xff\xfe\x70\x0d\x8c\xf4\x05\x13\x38"
        "\xe6\x93\x25\x9e\xb9\x22\x63\xdd\xa7",
        1233);
    *(uint16_t*)0x20606fe4 = 0xa;
    *(uint16_t*)0x20606fe6 = htobe16(0x4e20);
    *(uint32_t*)0x20606fe8 = 0;
    *(uint8_t*)0x20606fec = 0xfe;
    *(uint8_t*)0x20606fed = 0x80;
    *(uint8_t*)0x20606fee = 0;
    *(uint8_t*)0x20606fef = 0;
    *(uint8_t*)0x20606ff0 = 0;
    *(uint8_t*)0x20606ff1 = 0;
    *(uint8_t*)0x20606ff2 = 0;
    *(uint8_t*)0x20606ff3 = 0;
    *(uint8_t*)0x20606ff4 = 0;
    *(uint8_t*)0x20606ff5 = 0;
    *(uint8_t*)0x20606ff6 = 0;
    *(uint8_t*)0x20606ff7 = 0;
    *(uint8_t*)0x20606ff8 = 0;
    *(uint8_t*)0x20606ff9 = 0;
    *(uint8_t*)0x20606ffa = 0;
    *(uint8_t*)0x20606ffb = 0xaa;
    *(uint32_t*)0x20606ffc = 1;
    syscall(__NR_sendto, r[0], 0x20bc2000, 0x4d1, 0, 0x20606fe4, 0x1c);
    break;
  case 3:
    *(uint32_t*)0x20e8e000 = 0x208b6000;
    *(uint32_t*)0x20e8e004 = 0x1c;
    *(uint32_t*)0x20e8e008 = 0x20231ff0;
    *(uint32_t*)0x20e8e00c = 1;
    *(uint32_t*)0x20e8e010 = 0x20dffeb8;
    *(uint32_t*)0x20e8e014 = 0;
    *(uint32_t*)0x20e8e018 = 0;
    *(uint32_t*)0x20e8e01c = 0;
    *(uint16_t*)0x208b6000 = 0xa;
    *(uint16_t*)0x208b6002 = htobe16(0x4e20);
    *(uint32_t*)0x208b6004 = 0;
    *(uint8_t*)0x208b6008 = 0xfe;
    *(uint8_t*)0x208b6009 = 0x80;
    *(uint8_t*)0x208b600a = 0;
    *(uint8_t*)0x208b600b = 0;
    *(uint8_t*)0x208b600c = 0;
    *(uint8_t*)0x208b600d = 0;
    *(uint8_t*)0x208b600e = 0;
    *(uint8_t*)0x208b600f = 0;
    *(uint8_t*)0x208b6010 = 0;
    *(uint8_t*)0x208b6011 = 0;
    *(uint8_t*)0x208b6012 = 0;
    *(uint8_t*)0x208b6013 = 0;
    *(uint8_t*)0x208b6014 = 0;
    *(uint8_t*)0x208b6015 = 0;
    *(uint8_t*)0x208b6016 = 0;
    *(uint8_t*)0x208b6017 = 0xaa;
    *(uint32_t*)0x208b6018 = 1;
    *(uint32_t*)0x20231ff0 = 0x20f89000;
    *(uint32_t*)0x20231ff4 = 1;
    memcpy((void*)0x20f89000, "t", 1);
    syscall(__NR_sendmmsg, r[0], 0x20e8e000, 1, 0);
    break;
  case 4:
    *(uint32_t*)0x202fe000 = 0;
    syscall(__NR_setsockopt, r[0], 1, 7, 0x202fe000, 4);
    break;
  case 5:
    memcpy((void*)0x2010bf14, "\x14", 1);
    *(uint16_t*)0x204d9000 = 0xa;
    *(uint16_t*)0x204d9002 = htobe16(0x4e20);
    *(uint32_t*)0x204d9004 = 0;
    *(uint8_t*)0x204d9008 = 0xfe;
    *(uint8_t*)0x204d9009 = 0x80;
    *(uint8_t*)0x204d900a = 0;
    *(uint8_t*)0x204d900b = 0;
    *(uint8_t*)0x204d900c = 0;
    *(uint8_t*)0x204d900d = 0;
    *(uint8_t*)0x204d900e = 0;
    *(uint8_t*)0x204d900f = 0;
    *(uint8_t*)0x204d9010 = 0;
    *(uint8_t*)0x204d9011 = 0;
    *(uint8_t*)0x204d9012 = 0;
    *(uint8_t*)0x204d9013 = 0;
    *(uint8_t*)0x204d9014 = 0;
    *(uint8_t*)0x204d9015 = 0;
    *(uint8_t*)0x204d9016 = 0;
    *(uint8_t*)0x204d9017 = 0xaa;
    *(uint32_t*)0x204d9018 = 1;
    syscall(__NR_sendto, r[0], 0x2010bf14, 1, 0, 0x204d9000, 0x1c);
    break;
  case 6:
    syscall(__NR_listen, r[0], 7);
    break;
  case 7:
    *(uint32_t*)0x20192000 = 0xfffffe75;
    syscall(__NR_accept4, r[0], 0x20000000, 0x20192000, 0x80000);
    break;
  }
}

void test()
{
  memset(r, -1, sizeof(r));
  execute(8);
}

int main()
{
  for (;;) {
    loop();
  }
}
