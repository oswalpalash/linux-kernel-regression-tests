// https://syzkaller.appspot.com/bug?id=af9247ecbda7b4e70f33613045d181d934dd2b27
// autogenerated by syzkaller (https://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZEOF_IO_URING_SQE 64
#define SIZEOF_IO_URING_CQE 16
#define SQ_HEAD_OFFSET 0
#define SQ_TAIL_OFFSET 64
#define SQ_RING_MASK_OFFSET 256
#define SQ_RING_ENTRIES_OFFSET 264
#define SQ_FLAGS_OFFSET 276
#define SQ_DROPPED_OFFSET 272
#define CQ_HEAD_OFFSET 128
#define CQ_TAIL_OFFSET 192
#define CQ_RING_MASK_OFFSET 260
#define CQ_RING_ENTRIES_OFFSET 268
#define CQ_RING_OVERFLOW_OFFSET 284
#define CQ_FLAGS_OFFSET 280
#define CQ_CQES_OFFSET 320

struct io_sqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t flags;
  uint32_t dropped;
  uint32_t array;
  uint32_t resv1;
  uint64_t resv2;
};

struct io_cqring_offsets {
  uint32_t head;
  uint32_t tail;
  uint32_t ring_mask;
  uint32_t ring_entries;
  uint32_t overflow;
  uint32_t cqes;
  uint64_t resv[2];
};

struct io_uring_params {
  uint32_t sq_entries;
  uint32_t cq_entries;
  uint32_t flags;
  uint32_t sq_thread_cpu;
  uint32_t sq_thread_idle;
  uint32_t features;
  uint32_t resv[4];
  struct io_sqring_offsets sq_off;
  struct io_cqring_offsets cq_off;
};

#define IORING_OFF_SQ_RING 0
#define IORING_OFF_SQES 0x10000000ULL

#define __NR_io_uring_setup 425
static long syz_io_uring_setup(volatile long a0, volatile long a1,
                               volatile long a2, volatile long a3,
                               volatile long a4, volatile long a5)
{
  uint32_t entries = (uint32_t)a0;
  struct io_uring_params* setup_params = (struct io_uring_params*)a1;
  void* vma1 = (void*)a2;
  void* vma2 = (void*)a3;
  void** ring_ptr_out = (void**)a4;
  void** sqes_ptr_out = (void**)a5;
  uint32_t fd_io_uring = syscall(__NR_io_uring_setup, entries, setup_params);
  uint32_t sq_ring_sz =
      setup_params->sq_off.array + setup_params->sq_entries * sizeof(uint32_t);
  uint32_t cq_ring_sz = setup_params->cq_off.cqes +
                        setup_params->cq_entries * SIZEOF_IO_URING_CQE;
  uint32_t ring_sz = sq_ring_sz > cq_ring_sz ? sq_ring_sz : cq_ring_sz;
  *ring_ptr_out = mmap(vma1, ring_sz, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_POPULATE | MAP_FIXED, fd_io_uring,
                       IORING_OFF_SQ_RING);
  uint32_t sqes_sz = setup_params->sq_entries * SIZEOF_IO_URING_SQE;
  *sqes_ptr_out =
      mmap(vma2, sqes_sz, PROT_READ | PROT_WRITE,
           MAP_SHARED | MAP_POPULATE | MAP_FIXED, fd_io_uring, IORING_OFF_SQES);
  return fd_io_uring;
}

static long syz_io_uring_submit(volatile long a0, volatile long a1,
                                volatile long a2, volatile long a3)
{
  char* ring_ptr = (char*)a0;
  char* sqes_ptr = (char*)a1;
  char* sqe = (char*)a2;
  uint32_t sqes_index = (uint32_t)a3;
  uint32_t sq_ring_entries = *(uint32_t*)(ring_ptr + SQ_RING_ENTRIES_OFFSET);
  uint32_t cq_ring_entries = *(uint32_t*)(ring_ptr + CQ_RING_ENTRIES_OFFSET);
  uint32_t sq_array_off =
      (CQ_CQES_OFFSET + cq_ring_entries * SIZEOF_IO_URING_CQE + 63) & ~63;
  if (sq_ring_entries)
    sqes_index %= sq_ring_entries;
  char* sqe_dest = sqes_ptr + sqes_index * SIZEOF_IO_URING_SQE;
  memcpy(sqe_dest, sqe, SIZEOF_IO_URING_SQE);
  uint32_t sq_ring_mask = *(uint32_t*)(ring_ptr + SQ_RING_MASK_OFFSET);
  uint32_t* sq_tail_ptr = (uint32_t*)(ring_ptr + SQ_TAIL_OFFSET);
  uint32_t sq_tail = *sq_tail_ptr & sq_ring_mask;
  uint32_t sq_tail_next = *sq_tail_ptr + 1;
  uint32_t* sq_array = (uint32_t*)(ring_ptr + sq_array_off);
  *(sq_array + sq_tail) = sqes_index;
  __atomic_store_n(sq_tail_ptr, sq_tail_next, __ATOMIC_RELEASE);
  return 0;
}

#ifndef __NR_io_uring_enter
#define __NR_io_uring_enter 426
#endif
#ifndef __NR_mmap
#define __NR_mmap 192
#endif
#ifndef __NR_signalfd4
#define __NR_signalfd4 327
#endif
#undef __NR_mmap
#define __NR_mmap __NR_mmap2

uint64_t r[3] = {0xffffffffffffffff, 0x0, 0x0};

int main(void)
{
  syscall(__NR_mmap, 0x1ffff000, 0x1000, 0, 0x32, -1, 0);
  syscall(__NR_mmap, 0x20000000, 0x1000000, 7, 0x32, -1, 0);
  syscall(__NR_mmap, 0x21000000, 0x1000, 0, 0x32, -1, 0);
  intptr_t res = 0;
  *(uint32_t*)0x20000100 = 0;
  *(uint32_t*)0x20000104 = 0;
  syscall(__NR_signalfd4, -1, 0x20000100, 8, 0);
  *(uint32_t*)0x20000084 = 0;
  *(uint32_t*)0x20000088 = 0;
  *(uint32_t*)0x2000008c = 0;
  *(uint32_t*)0x20000090 = 0;
  *(uint32_t*)0x20000098 = -1;
  *(uint32_t*)0x2000009c = 0;
  *(uint32_t*)0x200000a0 = 0;
  *(uint32_t*)0x200000a4 = 0;
  res = -1;
  res = syz_io_uring_setup(0x87, 0x20000080, 0x20ee7000, 0x206d4000, 0x20000000,
                           0x20000040);
  if (res != -1) {
    r[0] = res;
    r[1] = *(uint64_t*)0x20000000;
    r[2] = *(uint64_t*)0x20000040;
  }
  *(uint8_t*)0x20000200 = 6;
  *(uint8_t*)0x20000201 = 0;
  *(uint16_t*)0x20000202 = 0;
  *(uint32_t*)0x20000204 = 3;
  *(uint64_t*)0x20000208 = 0;
  *(uint64_t*)0x20000210 = 0;
  *(uint32_t*)0x20000218 = 0;
  *(uint16_t*)0x2000021c = 0;
  *(uint16_t*)0x2000021e = 0;
  *(uint64_t*)0x20000220 = 0;
  *(uint16_t*)0x20000228 = 0;
  *(uint16_t*)0x2000022a = 0;
  *(uint8_t*)0x2000022c = 0;
  *(uint8_t*)0x2000022d = 0;
  *(uint8_t*)0x2000022e = 0;
  *(uint8_t*)0x2000022f = 0;
  *(uint8_t*)0x20000230 = 0;
  *(uint8_t*)0x20000231 = 0;
  *(uint8_t*)0x20000232 = 0;
  *(uint8_t*)0x20000233 = 0;
  *(uint8_t*)0x20000234 = 0;
  *(uint8_t*)0x20000235 = 0;
  *(uint8_t*)0x20000236 = 0;
  *(uint8_t*)0x20000237 = 0;
  *(uint8_t*)0x20000238 = 0;
  *(uint8_t*)0x20000239 = 0;
  *(uint8_t*)0x2000023a = 0;
  *(uint8_t*)0x2000023b = 0;
  *(uint8_t*)0x2000023c = 0;
  *(uint8_t*)0x2000023d = 0;
  *(uint8_t*)0x2000023e = 0;
  *(uint8_t*)0x2000023f = 0;
  syz_io_uring_submit(r[1], r[2], 0x20000200, 0);
  syscall(__NR_io_uring_enter, (intptr_t)r[0], 0x450c, 0, 0, 0, 0);
  syz_io_uring_submit(r[1], 0, 0, 5);
  return 0;
}
