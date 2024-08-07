// https://syzkaller.appspot.com/bug?id=789652eae4c8cc234c72c0c49c943dad7ceaaef6
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __NR_userfaultfd
#define __NR_userfaultfd 323
#endif

#define USLEEP_FORKED_CHILD (3 * 50 * 1000)

static long handle_clone_ret(long ret)
{
  if (ret != 0) {
    return ret;
  }
  usleep(USLEEP_FORKED_CHILD);
  syscall(__NR_exit, 0);
  while (1) {
  }
}

static long syz_clone(volatile long flags, volatile long stack,
                      volatile long stack_len, volatile long ptid,
                      volatile long ctid, volatile long tls)
{
  long sp = (stack + stack_len) & ~15;
  long ret = (long)syscall(__NR_clone, flags & ~CLONE_VM, sp, ptid, ctid, tls);
  return handle_clone_ret(ret);
}

uint64_t r[1] = {0xffffffffffffffff};

int main(void)
{
  syscall(__NR_mmap, /*addr=*/0x1ffff000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x20000000ul, /*len=*/0x1000000ul,
          /*prot=PROT_WRITE|PROT_READ|PROT_EXEC*/ 7ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  syscall(__NR_mmap, /*addr=*/0x21000000ul, /*len=*/0x1000ul, /*prot=*/0ul,
          /*flags=MAP_FIXED|MAP_ANONYMOUS|MAP_PRIVATE*/ 0x32ul, /*fd=*/-1,
          /*offset=*/0ul);
  intptr_t res = 0;
  res = syscall(__NR_userfaultfd,
                /*flags=UFFD_USER_MODE_ONLY|O_CLOEXEC*/ 0x80001ul);
  if (res != -1)
    r[0] = res;
  *(uint64_t*)0x20000000 = 0xaa;
  *(uint64_t*)0x20000008 = 0;
  *(uint64_t*)0x20000010 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc018aa3f, /*arg=*/0x20000000ul);
  *(uint64_t*)0x20000080 = 0x200e2000;
  *(uint64_t*)0x20000088 = 0xc00000;
  *(uint64_t*)0x20000090 = 2;
  *(uint64_t*)0x20000098 = 0;
  syscall(__NR_ioctl, /*fd=*/r[0], /*cmd=*/0xc020aa00, /*arg=*/0x20000080ul);
  *(uint64_t*)0x20000000 = 0x208f9f09;
  *(uint64_t*)0x20000008 = 0xf7;
  *(uint64_t*)0x20121000 = 0x20217f28;
  *(uint64_t*)0x20121008 = 0xffffff4e;
  syscall(__NR_process_vm_writev, /*pid=*/0, /*loc_vec=*/0x20000000ul,
          /*loc_vlen=*/1ul, /*rem_vec=*/0x20121000ul, /*rem_vlen=*/0x23aul,
          /*flags=*/0ul);
  syz_clone(/*flags=*/0, /*stack=*/0, /*stack_len=*/0, /*parentid=*/0,
            /*childtid=*/0, /*tls=*/0);
  return 0;
}
