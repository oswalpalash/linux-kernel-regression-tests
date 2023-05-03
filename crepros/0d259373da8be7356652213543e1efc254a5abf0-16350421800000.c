// https://syzkaller.appspot.com/bug?id=0d259373da8be7356652213543e1efc254a5abf0
// autogenerated by syzkaller (http://github.com/google/syzkaller)

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <linux/kvm.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>

__attribute__((noreturn)) static void doexit(int status)
{
  volatile unsigned i;
  syscall(__NR_exit_group, status);
  for (i = 0;; i++) {
  }
}

#include <stdint.h>
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

#define BITMASK_LEN(type, bf_len) (type)((1ull << (bf_len)) - 1)

#define BITMASK_LEN_OFF(type, bf_off, bf_len)                          \
  (type)(BITMASK_LEN(type, (bf_len)) << (bf_off))

#define STORE_BY_BITMASK(type, addr, val, bf_off, bf_len)              \
  if ((bf_off) == 0 && (bf_len) == 0) {                                \
    *(type*)(addr) = (type)(val);                                      \
  } else {                                                             \
    type new_val = *(type*)(addr);                                     \
    new_val &= ~BITMASK_LEN_OFF(type, (bf_off), (bf_len));             \
    new_val |= ((type)(val)&BITMASK_LEN(type, (bf_len))) << (bf_off);  \
    *(type*)(addr) = new_val;                                          \
  }

const char kvm_asm16_cpl3[] =
    "\x0f\x20\xc0\x66\x83\xc8\x01\x0f\x22\xc0\xb8\xa0\x00\x0f\x00\xd8"
    "\xb8\x2b\x00\x8e\xd8\x8e\xc0\x8e\xe0\x8e\xe8\xbc\x00\x01\xc7\x06"
    "\x00\x01\x1d\xba\xc7\x06\x02\x01\x23\x00\xc7\x06\x04\x01\x00\x01"
    "\xc7\x06\x06\x01\x2b\x00\xcb";
const char kvm_asm32_paged[] =
    "\x0f\x20\xc0\x0d\x00\x00\x00\x80\x0f\x22\xc0";
const char kvm_asm32_vm86[] =
    "\x66\xb8\xb8\x00\x0f\x00\xd8\xea\x00\x00\x00\x00\xd0\x00";
const char kvm_asm32_paged_vm86[] =
    "\x0f\x20\xc0\x0d\x00\x00\x00\x80\x0f\x22\xc0\x66\xb8\xb8\x00\x0f"
    "\x00\xd8\xea\x00\x00\x00\x00\xd0\x00";
const char kvm_asm64_vm86[] = "\x0f\x20\xc0\x0d\x00\x00\x00\x80\x0f\x22"
                              "\xc0\x66\xb8\xb8\x00\x0f\x00\xd8\xea\x00"
                              "\x00\x00\x00\xd0\x00";
const char kvm_asm64_enable_long[] =
    "\x0f\x20\xc0\x0d\x00\x00\x00\x80\x0f\x22\xc0\xea\xde\xc0\xad\x0b"
    "\x50\x00\x48\xc7\xc0\xd8\x00\x00\x00\x0f\x00\xd8";
const char kvm_asm64_init_vm[] =
    "\x0f\x20\xc0\x0d\x00\x00\x00\x80\x0f\x22\xc0\xea\xde\xc0\xad\x0b"
    "\x50\x00\x48\xc7\xc0\xd8\x00\x00\x00\x0f\x00\xd8\x48\xc7\xc1\x3a"
    "\x00\x00\x00\x0f\x32\x48\x83\xc8\x05\x0f\x30\x0f\x20\xe0\x48\x0d"
    "\x00\x20\x00\x00\x0f\x22\xe0\x48\xc7\xc1\x80\x04\x00\x00\x0f\x32"
    "\x48\xc7\xc2\x00\x60\x00\x00\x89\x02\x48\xc7\xc2\x00\x70\x00\x00"
    "\x89\x02\x48\xc7\xc0\x00\x5f\x00\x00\xf3\x0f\xc7\x30\x48\xc7\xc0"
    "\x08\x5f\x00\x00\x66\x0f\xc7\x30\x0f\xc7\x30\x48\xc7\xc1\x81\x04"
    "\x00\x00\x0f\x32\x48\x83\xc8\x3f\x48\x21\xd0\x48\xc7\xc2\x00\x40"
    "\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x02\x40\x00\x00\x48\xb8\x84\x9e"
    "\x99\xf3\x00\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x1e\x40\x00\x00"
    "\x48\xc7\xc0\x81\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc1\x83\x04\x00"
    "\x00\x0f\x32\x48\x0d\xff\x6f\x03\x00\x48\x21\xd0\x48\xc7\xc2\x0c"
    "\x40\x00\x00\x0f\x79\xd0\x48\xc7\xc1\x84\x04\x00\x00\x0f\x32\x48"
    "\x0d\xff\x17\x00\x00\x48\x21\xd0\x48\xc7\xc2\x12\x40\x00\x00\x0f"
    "\x79\xd0\x48\xc7\xc2\x04\x2c\x00\x00\x48\xc7\xc0\x00\x00\x00\x00"
    "\x0f\x79\xd0\x48\xc7\xc2\x00\x28\x00\x00\x48\xc7\xc0\xff\xff\xff"
    "\xff\x0f\x79\xd0\x48\xc7\xc2\x02\x0c\x00\x00\x48\xc7\xc0\x50\x00"
    "\x00\x00\x0f\x79\xd0\x48\xc7\xc0\x58\x00\x00\x00\x48\xc7\xc2\x00"
    "\x0c\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x04\x0c\x00\x00\x0f\x79\xd0"
    "\x48\xc7\xc2\x06\x0c\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x08\x0c\x00"
    "\x00\x0f\x79\xd0\x48\xc7\xc2\x0a\x0c\x00\x00\x0f\x79\xd0\x48\xc7"
    "\xc0\xd8\x00\x00\x00\x48\xc7\xc2\x0c\x0c\x00\x00\x0f\x79\xd0\x48"
    "\xc7\xc2\x02\x2c\x00\x00\x48\xc7\xc0\x00\x05\x00\x00\x0f\x79\xd0"
    "\x48\xc7\xc2\x00\x4c\x00\x00\x48\xc7\xc0\x50\x00\x00\x00\x0f\x79"
    "\xd0\x48\xc7\xc2\x10\x6c\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f"
    "\x79\xd0\x48\xc7\xc2\x12\x6c\x00\x00\x48\xc7\xc0\x00\x00\x00\x00"
    "\x0f\x79\xd0\x0f\x20\xc0\x48\xc7\xc2\x00\x6c\x00\x00\x48\x89\xc0"
    "\x0f\x79\xd0\x0f\x20\xd8\x48\xc7\xc2\x02\x6c\x00\x00\x48\x89\xc0"
    "\x0f\x79\xd0\x0f\x20\xe0\x48\xc7\xc2\x04\x6c\x00\x00\x48\x89\xc0"
    "\x0f\x79\xd0\x48\xc7\xc2\x06\x6c\x00\x00\x48\xc7\xc0\x00\x00\x00"
    "\x00\x0f\x79\xd0\x48\xc7\xc2\x08\x6c\x00\x00\x48\xc7\xc0\x00\x00"
    "\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x0a\x6c\x00\x00\x48\xc7\xc0\x00"
    "\x3a\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x0c\x6c\x00\x00\x48\xc7\xc0"
    "\x00\x10\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x0e\x6c\x00\x00\x48\xc7"
    "\xc0\x00\x38\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x14\x6c\x00\x00\x48"
    "\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x16\x6c\x00\x00"
    "\x48\x8b\x04\x25\x10\x5f\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x00\x00"
    "\x00\x00\x48\xc7\xc0\x01\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x02"
    "\x00\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2"
    "\x00\x20\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48\xc7"
    "\xc2\x02\x20\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48"
    "\xc7\xc2\x04\x20\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0"
    "\x48\xc7\xc2\x06\x20\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79"
    "\xd0\x48\xc7\xc1\x77\x02\x00\x00\x0f\x32\x48\xc1\xe2\x20\x48\x09"
    "\xd0\x48\xc7\xc2\x00\x2c\x00\x00\x48\x89\xc0\x0f\x79\xd0\x48\xc7"
    "\xc2\x04\x40\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48"
    "\xc7\xc2\x0a\x40\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0"
    "\x48\xc7\xc2\x0e\x40\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79"
    "\xd0\x48\xc7\xc2\x10\x40\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f"
    "\x79\xd0\x48\xc7\xc2\x16\x40\x00\x00\x48\xc7\xc0\x00\x00\x00\x00"
    "\x0f\x79\xd0\x48\xc7\xc2\x14\x40\x00\x00\x48\xc7\xc0\x00\x00\x00"
    "\x00\x0f\x79\xd0\x48\xc7\xc2\x00\x60\x00\x00\x48\xc7\xc0\xff\xff"
    "\xff\xff\x0f\x79\xd0\x48\xc7\xc2\x02\x60\x00\x00\x48\xc7\xc0\xff"
    "\xff\xff\xff\x0f\x79\xd0\x48\xc7\xc2\x1c\x20\x00\x00\x48\xc7\xc0"
    "\x00\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x1e\x20\x00\x00\x48\xc7"
    "\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x20\x20\x00\x00\x48"
    "\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x22\x20\x00\x00"
    "\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x00\x08\x00"
    "\x00\x48\xc7\xc0\x58\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x02\x08"
    "\x00\x00\x48\xc7\xc0\x50\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x04"
    "\x08\x00\x00\x48\xc7\xc0\x58\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2"
    "\x06\x08\x00\x00\x48\xc7\xc0\x58\x00\x00\x00\x0f\x79\xd0\x48\xc7"
    "\xc2\x08\x08\x00\x00\x48\xc7\xc0\x58\x00\x00\x00\x0f\x79\xd0\x48"
    "\xc7\xc2\x0a\x08\x00\x00\x48\xc7\xc0\x58\x00\x00\x00\x0f\x79\xd0"
    "\x48\xc7\xc2\x0c\x08\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79"
    "\xd0\x48\xc7\xc2\x0e\x08\x00\x00\x48\xc7\xc0\xd8\x00\x00\x00\x0f"
    "\x79\xd0\x48\xc7\xc2\x12\x68\x00\x00\x48\xc7\xc0\x00\x00\x00\x00"
    "\x0f\x79\xd0\x48\xc7\xc2\x14\x68\x00\x00\x48\xc7\xc0\x00\x3a\x00"
    "\x00\x0f\x79\xd0\x48\xc7\xc2\x16\x68\x00\x00\x48\xc7\xc0\x00\x10"
    "\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x18\x68\x00\x00\x48\xc7\xc0\x00"
    "\x38\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x00\x48\x00\x00\x48\xc7\xc0"
    "\xff\xff\x0f\x00\x0f\x79\xd0\x48\xc7\xc2\x02\x48\x00\x00\x48\xc7"
    "\xc0\xff\xff\x0f\x00\x0f\x79\xd0\x48\xc7\xc2\x04\x48\x00\x00\x48"
    "\xc7\xc0\xff\xff\x0f\x00\x0f\x79\xd0\x48\xc7\xc2\x06\x48\x00\x00"
    "\x48\xc7\xc0\xff\xff\x0f\x00\x0f\x79\xd0\x48\xc7\xc2\x08\x48\x00"
    "\x00\x48\xc7\xc0\xff\xff\x0f\x00\x0f\x79\xd0\x48\xc7\xc2\x0a\x48"
    "\x00\x00\x48\xc7\xc0\xff\xff\x0f\x00\x0f\x79\xd0\x48\xc7\xc2\x0c"
    "\x48\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2"
    "\x0e\x48\x00\x00\x48\xc7\xc0\xff\x1f\x00\x00\x0f\x79\xd0\x48\xc7"
    "\xc2\x10\x48\x00\x00\x48\xc7\xc0\xff\x1f\x00\x00\x0f\x79\xd0\x48"
    "\xc7\xc2\x12\x48\x00\x00\x48\xc7\xc0\xff\x1f\x00\x00\x0f\x79\xd0"
    "\x48\xc7\xc2\x14\x48\x00\x00\x48\xc7\xc0\x93\x40\x00\x00\x0f\x79"
    "\xd0\x48\xc7\xc2\x16\x48\x00\x00\x48\xc7\xc0\x9b\x20\x00\x00\x0f"
    "\x79\xd0\x48\xc7\xc2\x18\x48\x00\x00\x48\xc7\xc0\x93\x40\x00\x00"
    "\x0f\x79\xd0\x48\xc7\xc2\x1a\x48\x00\x00\x48\xc7\xc0\x93\x40\x00"
    "\x00\x0f\x79\xd0\x48\xc7\xc2\x1c\x48\x00\x00\x48\xc7\xc0\x93\x40"
    "\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x1e\x48\x00\x00\x48\xc7\xc0\x93"
    "\x40\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x20\x48\x00\x00\x48\xc7\xc0"
    "\x82\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x22\x48\x00\x00\x48\xc7"
    "\xc0\x8b\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x1c\x68\x00\x00\x48"
    "\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x1e\x68\x00\x00"
    "\x48\xc7\xc0\x00\x91\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x20\x68\x00"
    "\x00\x48\xc7\xc0\x02\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x06\x28"
    "\x00\x00\x48\xc7\xc0\x00\x05\x00\x00\x0f\x79\xd0\x48\xc7\xc2\x0a"
    "\x28\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48\xc7\xc2"
    "\x0c\x28\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48\xc7"
    "\xc2\x0e\x28\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0\x48"
    "\xc7\xc2\x10\x28\x00\x00\x48\xc7\xc0\x00\x00\x00\x00\x0f\x79\xd0"
    "\x0f\x20\xc0\x48\xc7\xc2\x00\x68\x00\x00\x48\x89\xc0\x0f\x79\xd0"
    "\x0f\x20\xd8\x48\xc7\xc2\x02\x68\x00\x00\x48\x89\xc0\x0f\x79\xd0"
    "\x0f\x20\xe0\x48\xc7\xc2\x04\x68\x00\x00\x48\x89\xc0\x0f\x79\xd0"
    "\x48\xc7\xc0\x18\x5f\x00\x00\x48\x8b\x10\x48\xc7\xc0\x20\x5f\x00"
    "\x00\x48\x8b\x08\x48\x31\xc0\x0f\x78\xd0\x48\x31\xc8\x0f\x79\xd0"
    "\x0f\x01\xc2\x48\xc7\xc2\x00\x44\x00\x00\x0f\x78\xd0\xf4";
const char kvm_asm64_vm_exit[] =
    "\x48\xc7\xc3\x00\x44\x00\x00\x0f\x78\xda\x48\xc7\xc3\x02\x44\x00"
    "\x00\x0f\x78\xd9\x48\xc7\xc0\x00\x64\x00\x00\x0f\x78\xc0\x48\xc7"
    "\xc3\x1e\x68\x00\x00\x0f\x78\xdb\xf4";
const char kvm_asm64_cpl3[] =
    "\x0f\x20\xc0\x0d\x00\x00\x00\x80\x0f\x22\xc0\xea\xde\xc0\xad\x0b"
    "\x50\x00\x48\xc7\xc0\xd8\x00\x00\x00\x0f\x00\xd8\x48\xc7\xc0\x6b"
    "\x00\x00\x00\x8e\xd8\x8e\xc0\x8e\xe0\x8e\xe8\x48\xc7\xc4\x80\x0f"
    "\x00\x00\x48\xc7\x04\x24\x1d\xba\x00\x00\x48\xc7\x44\x24\x04\x63"
    "\x00\x00\x00\x48\xc7\x44\x24\x08\x80\x0f\x00\x00\x48\xc7\x44\x24"
    "\x0c\x6b\x00\x00\x00\xcb";

#define ADDR_TEXT 0x0000
#define ADDR_GDT 0x1000
#define ADDR_LDT 0x1800
#define ADDR_PML4 0x2000
#define ADDR_PDP 0x3000
#define ADDR_PD 0x4000
#define ADDR_STACK0 0x0f80
#define ADDR_VAR_HLT 0x2800
#define ADDR_VAR_SYSRET 0x2808
#define ADDR_VAR_SYSEXIT 0x2810
#define ADDR_VAR_IDT 0x3800
#define ADDR_VAR_TSS64 0x3a00
#define ADDR_VAR_TSS64_CPL3 0x3c00
#define ADDR_VAR_TSS16 0x3d00
#define ADDR_VAR_TSS16_2 0x3e00
#define ADDR_VAR_TSS16_CPL3 0x3f00
#define ADDR_VAR_TSS32 0x4800
#define ADDR_VAR_TSS32_2 0x4a00
#define ADDR_VAR_TSS32_CPL3 0x4c00
#define ADDR_VAR_TSS32_VM86 0x4e00
#define ADDR_VAR_VMXON_PTR 0x5f00
#define ADDR_VAR_VMCS_PTR 0x5f08
#define ADDR_VAR_VMEXIT_PTR 0x5f10
#define ADDR_VAR_VMWRITE_FLD 0x5f18
#define ADDR_VAR_VMWRITE_VAL 0x5f20
#define ADDR_VAR_VMXON 0x6000
#define ADDR_VAR_VMCS 0x7000
#define ADDR_VAR_VMEXIT_CODE 0x9000
#define ADDR_VAR_USER_CODE 0x9100
#define ADDR_VAR_USER_CODE2 0x9120

#define SEL_LDT (1 << 3)
#define SEL_CS16 (2 << 3)
#define SEL_DS16 (3 << 3)
#define SEL_CS16_CPL3 ((4 << 3) + 3)
#define SEL_DS16_CPL3 ((5 << 3) + 3)
#define SEL_CS32 (6 << 3)
#define SEL_DS32 (7 << 3)
#define SEL_CS32_CPL3 ((8 << 3) + 3)
#define SEL_DS32_CPL3 ((9 << 3) + 3)
#define SEL_CS64 (10 << 3)
#define SEL_DS64 (11 << 3)
#define SEL_CS64_CPL3 ((12 << 3) + 3)
#define SEL_DS64_CPL3 ((13 << 3) + 3)
#define SEL_CGATE16 (14 << 3)
#define SEL_TGATE16 (15 << 3)
#define SEL_CGATE32 (16 << 3)
#define SEL_TGATE32 (17 << 3)
#define SEL_CGATE64 (18 << 3)
#define SEL_CGATE64_HI (19 << 3)
#define SEL_TSS16 (20 << 3)
#define SEL_TSS16_2 (21 << 3)
#define SEL_TSS16_CPL3 ((22 << 3) + 3)
#define SEL_TSS32 (23 << 3)
#define SEL_TSS32_2 (24 << 3)
#define SEL_TSS32_CPL3 ((25 << 3) + 3)
#define SEL_TSS32_VM86 (26 << 3)
#define SEL_TSS64 (27 << 3)
#define SEL_TSS64_HI (28 << 3)
#define SEL_TSS64_CPL3 ((29 << 3) + 3)
#define SEL_TSS64_CPL3_HI (30 << 3)

#define MSR_IA32_FEATURE_CONTROL 0x3a
#define MSR_IA32_VMX_BASIC 0x480
#define MSR_IA32_SMBASE 0x9e
#define MSR_IA32_SYSENTER_CS 0x174
#define MSR_IA32_SYSENTER_ESP 0x175
#define MSR_IA32_SYSENTER_EIP 0x176
#define MSR_IA32_STAR 0xC0000081
#define MSR_IA32_LSTAR 0xC0000082
#define MSR_IA32_VMX_PROCBASED_CTLS2 0x48B

#define NEXT_INSN $0xbadc0de
#define PREFIX_SIZE 0xba1d

#define KVM_SMI _IO(KVMIO, 0xb7)

#define CR0_PE 1
#define CR0_MP (1 << 1)
#define CR0_EM (1 << 2)
#define CR0_TS (1 << 3)
#define CR0_ET (1 << 4)
#define CR0_NE (1 << 5)
#define CR0_WP (1 << 16)
#define CR0_AM (1 << 18)
#define CR0_NW (1 << 29)
#define CR0_CD (1 << 30)
#define CR0_PG (1 << 31)

#define CR4_VME 1
#define CR4_PVI (1 << 1)
#define CR4_TSD (1 << 2)
#define CR4_DE (1 << 3)
#define CR4_PSE (1 << 4)
#define CR4_PAE (1 << 5)
#define CR4_MCE (1 << 6)
#define CR4_PGE (1 << 7)
#define CR4_PCE (1 << 8)
#define CR4_OSFXSR (1 << 8)
#define CR4_OSXMMEXCPT (1 << 10)
#define CR4_UMIP (1 << 11)
#define CR4_VMXE (1 << 13)
#define CR4_SMXE (1 << 14)
#define CR4_FSGSBASE (1 << 16)
#define CR4_PCIDE (1 << 17)
#define CR4_OSXSAVE (1 << 18)
#define CR4_SMEP (1 << 20)
#define CR4_SMAP (1 << 21)
#define CR4_PKE (1 << 22)

#define EFER_SCE 1
#define EFER_LME (1 << 8)
#define EFER_LMA (1 << 10)
#define EFER_NXE (1 << 11)
#define EFER_SVME (1 << 12)
#define EFER_LMSLE (1 << 13)
#define EFER_FFXSR (1 << 14)
#define EFER_TCE (1 << 15)

#define PDE32_PRESENT 1
#define PDE32_RW (1 << 1)
#define PDE32_USER (1 << 2)
#define PDE32_PS (1 << 7)

#define PDE64_PRESENT 1
#define PDE64_RW (1 << 1)
#define PDE64_USER (1 << 2)
#define PDE64_ACCESSED (1 << 5)
#define PDE64_DIRTY (1 << 6)
#define PDE64_PS (1 << 7)
#define PDE64_G (1 << 8)

struct tss16 {
  uint16_t prev;
  uint16_t sp0;
  uint16_t ss0;
  uint16_t sp1;
  uint16_t ss1;
  uint16_t sp2;
  uint16_t ss2;
  uint16_t ip;
  uint16_t flags;
  uint16_t ax;
  uint16_t cx;
  uint16_t dx;
  uint16_t bx;
  uint16_t sp;
  uint16_t bp;
  uint16_t si;
  uint16_t di;
  uint16_t es;
  uint16_t cs;
  uint16_t ss;
  uint16_t ds;
  uint16_t ldt;
} __attribute__((packed));

struct tss32 {
  uint16_t prev, prevh;
  uint32_t sp0;
  uint16_t ss0, ss0h;
  uint32_t sp1;
  uint16_t ss1, ss1h;
  uint32_t sp2;
  uint16_t ss2, ss2h;
  uint32_t cr3;
  uint32_t ip;
  uint32_t flags;
  uint32_t ax;
  uint32_t cx;
  uint32_t dx;
  uint32_t bx;
  uint32_t sp;
  uint32_t bp;
  uint32_t si;
  uint32_t di;
  uint16_t es, esh;
  uint16_t cs, csh;
  uint16_t ss, ssh;
  uint16_t ds, dsh;
  uint16_t fs, fsh;
  uint16_t gs, gsh;
  uint16_t ldt, ldth;
  uint16_t trace;
  uint16_t io_bitmap;
} __attribute__((packed));

struct tss64 {
  uint32_t reserved0;
  uint64_t rsp[3];
  uint64_t reserved1;
  uint64_t ist[7];
  uint64_t reserved2;
  uint32_t reserved3;
  uint32_t io_bitmap;
} __attribute__((packed));

static void fill_segment_descriptor(uint64_t* dt, uint64_t* lt,
                                    struct kvm_segment* seg)
{
  uint16_t index = seg->selector >> 3;
  uint64_t limit = seg->g ? seg->limit >> 12 : seg->limit;
  uint64_t sd =
      (limit & 0xffff) | (seg->base & 0xffffff) << 16 |
      (uint64_t)seg->type << 40 | (uint64_t)seg->s << 44 |
      (uint64_t)seg->dpl << 45 | (uint64_t)seg->present << 47 |
      (limit & 0xf0000ULL) << 48 | (uint64_t)seg->avl << 52 |
      (uint64_t)seg->l << 53 | (uint64_t)seg->db << 54 |
      (uint64_t)seg->g << 55 | (seg->base & 0xff000000ULL) << 56;
  dt[index] = sd;
  lt[index] = sd;
}

static void fill_segment_descriptor_dword(uint64_t* dt, uint64_t* lt,
                                          struct kvm_segment* seg)
{
  fill_segment_descriptor(dt, lt, seg);
  uint16_t index = seg->selector >> 3;
  dt[index + 1] = 0;
  lt[index + 1] = 0;
}

static void setup_syscall_msrs(int cpufd, uint16_t sel_cs,
                               uint16_t sel_cs_cpl3)
{
  char buf[sizeof(struct kvm_msrs) + 5 * sizeof(struct kvm_msr_entry)];
  memset(buf, 0, sizeof(buf));
  struct kvm_msrs* msrs = (struct kvm_msrs*)buf;
  msrs->nmsrs = 5;
  msrs->entries[0].index = MSR_IA32_SYSENTER_CS;
  msrs->entries[0].data = sel_cs;
  msrs->entries[1].index = MSR_IA32_SYSENTER_ESP;
  msrs->entries[1].data = ADDR_STACK0;
  msrs->entries[2].index = MSR_IA32_SYSENTER_EIP;
  msrs->entries[2].data = ADDR_VAR_SYSEXIT;
  msrs->entries[3].index = MSR_IA32_STAR;
  msrs->entries[3].data =
      ((uint64_t)sel_cs << 32) | ((uint64_t)sel_cs_cpl3 << 48);
  msrs->entries[4].index = MSR_IA32_LSTAR;
  msrs->entries[4].data = ADDR_VAR_SYSRET;
  ioctl(cpufd, KVM_SET_MSRS, msrs);
}

static void setup_32bit_idt(struct kvm_sregs* sregs, char* host_mem,
                            uintptr_t guest_mem)
{
  sregs->idt.base = guest_mem + ADDR_VAR_IDT;
  sregs->idt.limit = 0x1ff;
  uint64_t* idt = (uint64_t*)(host_mem + sregs->idt.base);
  int i;
  for (i = 0; i < 32; i++) {
    struct kvm_segment gate;
    gate.selector = i << 3;
    switch (i % 6) {
    case 0:
      gate.type = 6;
      gate.base = SEL_CS16;
      break;
    case 1:
      gate.type = 7;
      gate.base = SEL_CS16;
      break;
    case 2:
      gate.type = 3;
      gate.base = SEL_TGATE16;
      break;
    case 3:
      gate.type = 14;
      gate.base = SEL_CS32;
      break;
    case 4:
      gate.type = 15;
      gate.base = SEL_CS32;
      break;
    case 6:
      gate.type = 11;
      gate.base = SEL_TGATE32;
      break;
    }
    gate.limit = guest_mem + ADDR_VAR_USER_CODE2;
    gate.present = 1;
    gate.dpl = 0;
    gate.s = 0;
    gate.g = 0;
    gate.db = 0;
    gate.l = 0;
    gate.avl = 0;
    fill_segment_descriptor(idt, idt, &gate);
  }
}

static void setup_64bit_idt(struct kvm_sregs* sregs, char* host_mem,
                            uintptr_t guest_mem)
{
  sregs->idt.base = guest_mem + ADDR_VAR_IDT;
  sregs->idt.limit = 0x1ff;
  uint64_t* idt = (uint64_t*)(host_mem + sregs->idt.base);
  int i;
  for (i = 0; i < 32; i++) {
    struct kvm_segment gate;
    gate.selector = (i * 2) << 3;
    gate.type = (i & 1) ? 14 : 15;
    gate.base = SEL_CS64;
    gate.limit = guest_mem + ADDR_VAR_USER_CODE2;
    gate.present = 1;
    gate.dpl = 0;
    gate.s = 0;
    gate.g = 0;
    gate.db = 0;
    gate.l = 0;
    gate.avl = 0;
    fill_segment_descriptor_dword(idt, idt, &gate);
  }
}

struct kvm_text {
  uintptr_t typ;
  const void* text;
  uintptr_t size;
};

struct kvm_opt {
  uint64_t typ;
  uint64_t val;
};

#define KVM_SETUP_PAGING (1 << 0)
#define KVM_SETUP_PAE (1 << 1)
#define KVM_SETUP_PROTECTED (1 << 2)
#define KVM_SETUP_CPL3 (1 << 3)
#define KVM_SETUP_VIRT86 (1 << 4)
#define KVM_SETUP_SMM (1 << 5)
#define KVM_SETUP_VM (1 << 6)

static uintptr_t syz_kvm_setup_cpu(uintptr_t a0, uintptr_t a1,
                                   uintptr_t a2, uintptr_t a3,
                                   uintptr_t a4, uintptr_t a5,
                                   uintptr_t a6, uintptr_t a7)
{
  const int vmfd = a0;
  const int cpufd = a1;
  char* const host_mem = (char*)a2;
  const struct kvm_text* const text_array_ptr = (struct kvm_text*)a3;
  const uintptr_t text_count = a4;
  const uintptr_t flags = a5;
  const struct kvm_opt* const opt_array_ptr = (struct kvm_opt*)a6;
  uintptr_t opt_count = a7;

  const uintptr_t page_size = 4 << 10;
  const uintptr_t ioapic_page = 10;
  const uintptr_t guest_mem_size = 24 * page_size;
  const uintptr_t guest_mem = 0;

  (void)text_count;
  int text_type = 0;
  const void* text = 0;
  uintptr_t text_size = 0;
  text_type = text_array_ptr[0].typ;
  text = text_array_ptr[0].text;
  text_size = text_array_ptr[0].size;

  uintptr_t i;
  for (i = 0; i < guest_mem_size / page_size; i++) {
    struct kvm_userspace_memory_region memreg;
    memreg.slot = i;
    memreg.flags = 0;
    memreg.guest_phys_addr = guest_mem + i * page_size;
    if (i == ioapic_page)
      memreg.guest_phys_addr = 0xfec00000;
    memreg.memory_size = page_size;
    memreg.userspace_addr = (uintptr_t)host_mem + i * page_size;
    ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &memreg);
  }
  struct kvm_userspace_memory_region memreg;
  memreg.slot = 1 + (1 << 16);
  memreg.flags = 0;
  memreg.guest_phys_addr = 0x30000;
  memreg.memory_size = 64 << 10;
  memreg.userspace_addr = (uintptr_t)host_mem;
  ioctl(vmfd, KVM_SET_USER_MEMORY_REGION, &memreg);

  struct kvm_sregs sregs;
  if (ioctl(cpufd, KVM_GET_SREGS, &sregs))
    return -1;

  struct kvm_regs regs;
  memset(&regs, 0, sizeof(regs));
  regs.rip = guest_mem + ADDR_TEXT;
  regs.rsp = ADDR_STACK0;

  sregs.gdt.base = guest_mem + ADDR_GDT;
  sregs.gdt.limit = 256 * sizeof(uint64_t) - 1;
  uint64_t* gdt = (uint64_t*)(host_mem + sregs.gdt.base);

  struct kvm_segment seg_ldt;
  seg_ldt.selector = SEL_LDT;
  seg_ldt.type = 2;
  seg_ldt.base = guest_mem + ADDR_LDT;
  seg_ldt.limit = 256 * sizeof(uint64_t) - 1;
  seg_ldt.present = 1;
  seg_ldt.dpl = 0;
  seg_ldt.s = 0;
  seg_ldt.g = 0;
  seg_ldt.db = 1;
  seg_ldt.l = 0;
  sregs.ldt = seg_ldt;
  uint64_t* ldt = (uint64_t*)(host_mem + sregs.ldt.base);

  struct kvm_segment seg_cs16;
  seg_cs16.selector = SEL_CS16;
  seg_cs16.type = 11;
  seg_cs16.base = 0;
  seg_cs16.limit = 0xfffff;
  seg_cs16.present = 1;
  seg_cs16.dpl = 0;
  seg_cs16.s = 1;
  seg_cs16.g = 0;
  seg_cs16.db = 0;
  seg_cs16.l = 0;

  struct kvm_segment seg_ds16 = seg_cs16;
  seg_ds16.selector = SEL_DS16;
  seg_ds16.type = 3;

  struct kvm_segment seg_cs16_cpl3 = seg_cs16;
  seg_cs16_cpl3.selector = SEL_CS16_CPL3;
  seg_cs16_cpl3.dpl = 3;

  struct kvm_segment seg_ds16_cpl3 = seg_ds16;
  seg_ds16_cpl3.selector = SEL_DS16_CPL3;
  seg_ds16_cpl3.dpl = 3;

  struct kvm_segment seg_cs32 = seg_cs16;
  seg_cs32.selector = SEL_CS32;
  seg_cs32.db = 1;

  struct kvm_segment seg_ds32 = seg_ds16;
  seg_ds32.selector = SEL_DS32;
  seg_ds32.db = 1;

  struct kvm_segment seg_cs32_cpl3 = seg_cs32;
  seg_cs32_cpl3.selector = SEL_CS32_CPL3;
  seg_cs32_cpl3.dpl = 3;

  struct kvm_segment seg_ds32_cpl3 = seg_ds32;
  seg_ds32_cpl3.selector = SEL_DS32_CPL3;
  seg_ds32_cpl3.dpl = 3;

  struct kvm_segment seg_cs64 = seg_cs16;
  seg_cs64.selector = SEL_CS64;
  seg_cs64.l = 1;

  struct kvm_segment seg_ds64 = seg_ds32;
  seg_ds64.selector = SEL_DS64;

  struct kvm_segment seg_cs64_cpl3 = seg_cs64;
  seg_cs64_cpl3.selector = SEL_CS64_CPL3;
  seg_cs64_cpl3.dpl = 3;

  struct kvm_segment seg_ds64_cpl3 = seg_ds64;
  seg_ds64_cpl3.selector = SEL_DS64_CPL3;
  seg_ds64_cpl3.dpl = 3;

  struct kvm_segment seg_tss32;
  seg_tss32.selector = SEL_TSS32;
  seg_tss32.type = 9;
  seg_tss32.base = ADDR_VAR_TSS32;
  seg_tss32.limit = 0x1ff;
  seg_tss32.present = 1;
  seg_tss32.dpl = 0;
  seg_tss32.s = 0;
  seg_tss32.g = 0;
  seg_tss32.db = 0;
  seg_tss32.l = 0;

  struct kvm_segment seg_tss32_2 = seg_tss32;
  seg_tss32_2.selector = SEL_TSS32_2;
  seg_tss32_2.base = ADDR_VAR_TSS32_2;

  struct kvm_segment seg_tss32_cpl3 = seg_tss32;
  seg_tss32_cpl3.selector = SEL_TSS32_CPL3;
  seg_tss32_cpl3.base = ADDR_VAR_TSS32_CPL3;

  struct kvm_segment seg_tss32_vm86 = seg_tss32;
  seg_tss32_vm86.selector = SEL_TSS32_VM86;
  seg_tss32_vm86.base = ADDR_VAR_TSS32_VM86;

  struct kvm_segment seg_tss16 = seg_tss32;
  seg_tss16.selector = SEL_TSS16;
  seg_tss16.base = ADDR_VAR_TSS16;
  seg_tss16.limit = 0xff;
  seg_tss16.type = 1;

  struct kvm_segment seg_tss16_2 = seg_tss16;
  seg_tss16_2.selector = SEL_TSS16_2;
  seg_tss16_2.base = ADDR_VAR_TSS16_2;
  seg_tss16_2.dpl = 0;

  struct kvm_segment seg_tss16_cpl3 = seg_tss16;
  seg_tss16_cpl3.selector = SEL_TSS16_CPL3;
  seg_tss16_cpl3.base = ADDR_VAR_TSS16_CPL3;
  seg_tss16_cpl3.dpl = 3;

  struct kvm_segment seg_tss64 = seg_tss32;
  seg_tss64.selector = SEL_TSS64;
  seg_tss64.base = ADDR_VAR_TSS64;
  seg_tss64.limit = 0x1ff;

  struct kvm_segment seg_tss64_cpl3 = seg_tss64;
  seg_tss64_cpl3.selector = SEL_TSS64_CPL3;
  seg_tss64_cpl3.base = ADDR_VAR_TSS64_CPL3;
  seg_tss64_cpl3.dpl = 3;

  struct kvm_segment seg_cgate16;
  seg_cgate16.selector = SEL_CGATE16;
  seg_cgate16.type = 4;
  seg_cgate16.base = SEL_CS16 | (2 << 16);
  seg_cgate16.limit = ADDR_VAR_USER_CODE2;
  seg_cgate16.present = 1;
  seg_cgate16.dpl = 0;
  seg_cgate16.s = 0;
  seg_cgate16.g = 0;
  seg_cgate16.db = 0;
  seg_cgate16.l = 0;
  seg_cgate16.avl = 0;

  struct kvm_segment seg_tgate16 = seg_cgate16;
  seg_tgate16.selector = SEL_TGATE16;
  seg_tgate16.type = 3;
  seg_cgate16.base = SEL_TSS16_2;
  seg_tgate16.limit = 0;

  struct kvm_segment seg_cgate32 = seg_cgate16;
  seg_cgate32.selector = SEL_CGATE32;
  seg_cgate32.type = 12;
  seg_cgate32.base = SEL_CS32 | (2 << 16);

  struct kvm_segment seg_tgate32 = seg_cgate32;
  seg_tgate32.selector = SEL_TGATE32;
  seg_tgate32.type = 11;
  seg_tgate32.base = SEL_TSS32_2;
  seg_tgate32.limit = 0;

  struct kvm_segment seg_cgate64 = seg_cgate16;
  seg_cgate64.selector = SEL_CGATE64;
  seg_cgate64.type = 12;
  seg_cgate64.base = SEL_CS64;

  int kvmfd = open("/dev/kvm", O_RDWR);
  char buf[sizeof(struct kvm_cpuid2) +
           128 * sizeof(struct kvm_cpuid_entry2)];
  memset(buf, 0, sizeof(buf));
  struct kvm_cpuid2* cpuid = (struct kvm_cpuid2*)buf;
  cpuid->nent = 128;
  ioctl(kvmfd, KVM_GET_SUPPORTED_CPUID, cpuid);
  ioctl(cpufd, KVM_SET_CPUID2, cpuid);
  close(kvmfd);

  const char* text_prefix = 0;
  int text_prefix_size = 0;
  char* host_text = host_mem + ADDR_TEXT;

  if (text_type == 8) {
    if (flags & KVM_SETUP_SMM) {
      if (flags & KVM_SETUP_PROTECTED) {
        sregs.cs = seg_cs16;
        sregs.ds = sregs.es = sregs.fs = sregs.gs = sregs.ss = seg_ds16;
        sregs.cr0 |= CR0_PE;
      } else {
        sregs.cs.selector = 0;
        sregs.cs.base = 0;
      }

      *(host_mem + ADDR_TEXT) = 0xf4;
      host_text = host_mem + 0x8000;

      ioctl(cpufd, KVM_SMI, 0);
    } else if (flags & KVM_SETUP_VIRT86) {
      sregs.cs = seg_cs32;
      sregs.ds = sregs.es = sregs.fs = sregs.gs = sregs.ss = seg_ds32;
      sregs.cr0 |= CR0_PE;
      sregs.efer |= EFER_SCE;

      setup_syscall_msrs(cpufd, SEL_CS32, SEL_CS32_CPL3);
      setup_32bit_idt(&sregs, host_mem, guest_mem);

      if (flags & KVM_SETUP_PAGING) {
        uint64_t pd_addr = guest_mem + ADDR_PD;
        uint64_t* pd = (uint64_t*)(host_mem + ADDR_PD);
        pd[0] = PDE32_PRESENT | PDE32_RW | PDE32_USER | PDE32_PS;
        sregs.cr3 = pd_addr;
        sregs.cr4 |= CR4_PSE;

        text_prefix = kvm_asm32_paged_vm86;
        text_prefix_size = sizeof(kvm_asm32_paged_vm86) - 1;
      } else {
        text_prefix = kvm_asm32_vm86;
        text_prefix_size = sizeof(kvm_asm32_vm86) - 1;
      }
    } else {
      sregs.cs.selector = 0;
      sregs.cs.base = 0;
    }
  } else if (text_type == 16) {
    if (flags & KVM_SETUP_CPL3) {
      sregs.cs = seg_cs16;
      sregs.ds = sregs.es = sregs.fs = sregs.gs = sregs.ss = seg_ds16;

      text_prefix = kvm_asm16_cpl3;
      text_prefix_size = sizeof(kvm_asm16_cpl3) - 1;
    } else {
      sregs.cr0 |= CR0_PE;
      sregs.cs = seg_cs16;
      sregs.ds = sregs.es = sregs.fs = sregs.gs = sregs.ss = seg_ds16;
    }
  } else if (text_type == 32) {
    sregs.cr0 |= CR0_PE;
    sregs.efer |= EFER_SCE;

    setup_syscall_msrs(cpufd, SEL_CS32, SEL_CS32_CPL3);
    setup_32bit_idt(&sregs, host_mem, guest_mem);

    if (flags & KVM_SETUP_SMM) {
      sregs.cs = seg_cs32;
      sregs.ds = sregs.es = sregs.fs = sregs.gs = sregs.ss = seg_ds32;

      *(host_mem + ADDR_TEXT) = 0xf4;
      host_text = host_mem + 0x8000;

      ioctl(cpufd, KVM_SMI, 0);
    } else if (flags & KVM_SETUP_PAGING) {
      sregs.cs = seg_cs32;
      sregs.ds = sregs.es = sregs.fs = sregs.gs = sregs.ss = seg_ds32;

      uint64_t pd_addr = guest_mem + ADDR_PD;
      uint64_t* pd = (uint64_t*)(host_mem + ADDR_PD);
      pd[0] = PDE32_PRESENT | PDE32_RW | PDE32_USER | PDE32_PS;
      sregs.cr3 = pd_addr;
      sregs.cr4 |= CR4_PSE;

      text_prefix = kvm_asm32_paged;
      text_prefix_size = sizeof(kvm_asm32_paged) - 1;
    } else if (flags & KVM_SETUP_CPL3) {
      sregs.cs = seg_cs32_cpl3;
      sregs.ds = sregs.es = sregs.fs = sregs.gs = sregs.ss =
          seg_ds32_cpl3;
    } else {
      sregs.cs = seg_cs32;
      sregs.ds = sregs.es = sregs.fs = sregs.gs = sregs.ss = seg_ds32;
    }
  } else {
    sregs.efer |= EFER_LME | EFER_SCE;
    sregs.cr0 |= CR0_PE;

    setup_syscall_msrs(cpufd, SEL_CS64, SEL_CS64_CPL3);
    setup_64bit_idt(&sregs, host_mem, guest_mem);

    sregs.cs = seg_cs32;
    sregs.ds = sregs.es = sregs.fs = sregs.gs = sregs.ss = seg_ds32;

    uint64_t pml4_addr = guest_mem + ADDR_PML4;
    uint64_t* pml4 = (uint64_t*)(host_mem + ADDR_PML4);
    uint64_t pdpt_addr = guest_mem + ADDR_PDP;
    uint64_t* pdpt = (uint64_t*)(host_mem + ADDR_PDP);
    uint64_t pd_addr = guest_mem + ADDR_PD;
    uint64_t* pd = (uint64_t*)(host_mem + ADDR_PD);
    pml4[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pdpt_addr;
    pdpt[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | pd_addr;
    pd[0] = PDE64_PRESENT | PDE64_RW | PDE64_USER | PDE64_PS;
    sregs.cr3 = pml4_addr;
    sregs.cr4 |= CR4_PAE;

    if (flags & KVM_SETUP_VM) {
      sregs.cr0 |= CR0_NE;

      *((uint64_t*)(host_mem + ADDR_VAR_VMXON_PTR)) = ADDR_VAR_VMXON;
      *((uint64_t*)(host_mem + ADDR_VAR_VMCS_PTR)) = ADDR_VAR_VMCS;
      memcpy(host_mem + ADDR_VAR_VMEXIT_CODE, kvm_asm64_vm_exit,
             sizeof(kvm_asm64_vm_exit) - 1);
      *((uint64_t*)(host_mem + ADDR_VAR_VMEXIT_PTR)) =
          ADDR_VAR_VMEXIT_CODE;

      text_prefix = kvm_asm64_init_vm;
      text_prefix_size = sizeof(kvm_asm64_init_vm) - 1;
    } else if (flags & KVM_SETUP_CPL3) {
      text_prefix = kvm_asm64_cpl3;
      text_prefix_size = sizeof(kvm_asm64_cpl3) - 1;
    } else {
      text_prefix = kvm_asm64_enable_long;
      text_prefix_size = sizeof(kvm_asm64_enable_long) - 1;
    }
  }

  struct tss16 tss16;
  memset(&tss16, 0, sizeof(tss16));
  tss16.ss0 = tss16.ss1 = tss16.ss2 = SEL_DS16;
  tss16.sp0 = tss16.sp1 = tss16.sp2 = ADDR_STACK0;
  tss16.ip = ADDR_VAR_USER_CODE2;
  tss16.flags = (1 << 1);
  tss16.cs = SEL_CS16;
  tss16.es = tss16.ds = tss16.ss = SEL_DS16;
  tss16.ldt = SEL_LDT;
  struct tss16* tss16_addr =
      (struct tss16*)(host_mem + seg_tss16_2.base);
  memcpy(tss16_addr, &tss16, sizeof(tss16));

  memset(&tss16, 0, sizeof(tss16));
  tss16.ss0 = tss16.ss1 = tss16.ss2 = SEL_DS16;
  tss16.sp0 = tss16.sp1 = tss16.sp2 = ADDR_STACK0;
  tss16.ip = ADDR_VAR_USER_CODE2;
  tss16.flags = (1 << 1);
  tss16.cs = SEL_CS16_CPL3;
  tss16.es = tss16.ds = tss16.ss = SEL_DS16_CPL3;
  tss16.ldt = SEL_LDT;
  struct tss16* tss16_cpl3_addr =
      (struct tss16*)(host_mem + seg_tss16_cpl3.base);
  memcpy(tss16_cpl3_addr, &tss16, sizeof(tss16));

  struct tss32 tss32;
  memset(&tss32, 0, sizeof(tss32));
  tss32.ss0 = tss32.ss1 = tss32.ss2 = SEL_DS32;
  tss32.sp0 = tss32.sp1 = tss32.sp2 = ADDR_STACK0;
  tss32.ip = ADDR_VAR_USER_CODE;
  tss32.flags = (1 << 1) | (1 << 17);
  tss32.ldt = SEL_LDT;
  tss32.cr3 = sregs.cr3;
  tss32.io_bitmap = offsetof(struct tss32, io_bitmap);
  struct tss32* tss32_addr =
      (struct tss32*)(host_mem + seg_tss32_vm86.base);
  memcpy(tss32_addr, &tss32, sizeof(tss32));

  memset(&tss32, 0, sizeof(tss32));
  tss32.ss0 = tss32.ss1 = tss32.ss2 = SEL_DS32;
  tss32.sp0 = tss32.sp1 = tss32.sp2 = ADDR_STACK0;
  tss32.ip = ADDR_VAR_USER_CODE;
  tss32.flags = (1 << 1);
  tss32.cr3 = sregs.cr3;
  tss32.es = tss32.ds = tss32.ss = tss32.gs = tss32.fs = SEL_DS32;
  tss32.cs = SEL_CS32;
  tss32.ldt = SEL_LDT;
  tss32.cr3 = sregs.cr3;
  tss32.io_bitmap = offsetof(struct tss32, io_bitmap);
  struct tss32* tss32_cpl3_addr =
      (struct tss32*)(host_mem + seg_tss32_2.base);
  memcpy(tss32_cpl3_addr, &tss32, sizeof(tss32));

  struct tss64 tss64;
  memset(&tss64, 0, sizeof(tss64));
  tss64.rsp[0] = ADDR_STACK0;
  tss64.rsp[1] = ADDR_STACK0;
  tss64.rsp[2] = ADDR_STACK0;
  tss64.io_bitmap = offsetof(struct tss64, io_bitmap);
  struct tss64* tss64_addr = (struct tss64*)(host_mem + seg_tss64.base);
  memcpy(tss64_addr, &tss64, sizeof(tss64));

  memset(&tss64, 0, sizeof(tss64));
  tss64.rsp[0] = ADDR_STACK0;
  tss64.rsp[1] = ADDR_STACK0;
  tss64.rsp[2] = ADDR_STACK0;
  tss64.io_bitmap = offsetof(struct tss64, io_bitmap);
  struct tss64* tss64_cpl3_addr =
      (struct tss64*)(host_mem + seg_tss64_cpl3.base);
  memcpy(tss64_cpl3_addr, &tss64, sizeof(tss64));

  if (text_size > 1000)
    text_size = 1000;
  if (text_prefix) {
    memcpy(host_text, text_prefix, text_prefix_size);
    void* patch = 0;
    patch = memmem(host_text, text_prefix_size, "\xde\xc0\xad\x0b", 4);
    if (patch)
      *((uint32_t*)patch) =
          guest_mem + ADDR_TEXT + ((char*)patch - host_text) + 6;
    uint16_t magic = PREFIX_SIZE;
    patch = 0;
    patch = memmem(host_text, text_prefix_size, &magic, sizeof(magic));
    if (patch)
      *((uint16_t*)patch) = guest_mem + ADDR_TEXT + text_prefix_size;
  }
  memcpy((void*)(host_text + text_prefix_size), text, text_size);
  *(host_text + text_prefix_size + text_size) = 0xf4;

  memcpy(host_mem + ADDR_VAR_USER_CODE, text, text_size);
  *(host_mem + ADDR_VAR_USER_CODE + text_size) = 0xf4;

  *(host_mem + ADDR_VAR_HLT) = 0xf4;
  memcpy(host_mem + ADDR_VAR_SYSRET, "\x0f\x07\xf4", 3);
  memcpy(host_mem + ADDR_VAR_SYSEXIT, "\x0f\x35\xf4", 3);

  *(uint64_t*)(host_mem + ADDR_VAR_VMWRITE_FLD) = 0;
  *(uint64_t*)(host_mem + ADDR_VAR_VMWRITE_VAL) = 0;

  if (opt_count > 2)
    opt_count = 2;
  for (i = 0; i < opt_count; i++) {
    uint64_t typ = 0;
    uint64_t val = 0;
    typ = opt_array_ptr[i].typ;
    val = opt_array_ptr[i].val;
    switch (typ % 9) {
    case 0:
      sregs.cr0 ^= val & (CR0_MP | CR0_EM | CR0_ET | CR0_NE | CR0_WP |
                          CR0_AM | CR0_NW | CR0_CD);
      break;
    case 1:
      sregs.cr4 ^= val & (CR4_VME | CR4_PVI | CR4_TSD | CR4_DE |
                          CR4_MCE | CR4_PGE | CR4_PCE | CR4_OSFXSR |
                          CR4_OSXMMEXCPT | CR4_UMIP | CR4_VMXE |
                          CR4_SMXE | CR4_FSGSBASE | CR4_PCIDE |
                          CR4_OSXSAVE | CR4_SMEP | CR4_SMAP | CR4_PKE);
      break;
    case 2:
      sregs.efer ^= val & (EFER_SCE | EFER_NXE | EFER_SVME |
                           EFER_LMSLE | EFER_FFXSR | EFER_TCE);
      break;
    case 3:
      val &= ((1 << 8) | (1 << 9) | (1 << 10) | (1 << 12) | (1 << 13) |
              (1 << 14) | (1 << 15) | (1 << 18) | (1 << 19) |
              (1 << 20) | (1 << 21));
      regs.rflags ^= val;
      tss16_addr->flags ^= val;
      tss16_cpl3_addr->flags ^= val;
      tss32_addr->flags ^= val;
      tss32_cpl3_addr->flags ^= val;
      break;
    case 4:
      seg_cs16.type = val & 0xf;
      seg_cs32.type = val & 0xf;
      seg_cs64.type = val & 0xf;
      break;
    case 5:
      seg_cs16_cpl3.type = val & 0xf;
      seg_cs32_cpl3.type = val & 0xf;
      seg_cs64_cpl3.type = val & 0xf;
      break;
    case 6:
      seg_ds16.type = val & 0xf;
      seg_ds32.type = val & 0xf;
      seg_ds64.type = val & 0xf;
      break;
    case 7:
      seg_ds16_cpl3.type = val & 0xf;
      seg_ds32_cpl3.type = val & 0xf;
      seg_ds64_cpl3.type = val & 0xf;
      break;
    case 8:
      *(uint64_t*)(host_mem + ADDR_VAR_VMWRITE_FLD) = (val & 0xffff);
      *(uint64_t*)(host_mem + ADDR_VAR_VMWRITE_VAL) = (val >> 16);
      break;
    default:
      fail("bad kvm setup opt");
    }
  }
  regs.rflags |= 2;

  fill_segment_descriptor(gdt, ldt, &seg_ldt);
  fill_segment_descriptor(gdt, ldt, &seg_cs16);
  fill_segment_descriptor(gdt, ldt, &seg_ds16);
  fill_segment_descriptor(gdt, ldt, &seg_cs16_cpl3);
  fill_segment_descriptor(gdt, ldt, &seg_ds16_cpl3);
  fill_segment_descriptor(gdt, ldt, &seg_cs32);
  fill_segment_descriptor(gdt, ldt, &seg_ds32);
  fill_segment_descriptor(gdt, ldt, &seg_cs32_cpl3);
  fill_segment_descriptor(gdt, ldt, &seg_ds32_cpl3);
  fill_segment_descriptor(gdt, ldt, &seg_cs64);
  fill_segment_descriptor(gdt, ldt, &seg_ds64);
  fill_segment_descriptor(gdt, ldt, &seg_cs64_cpl3);
  fill_segment_descriptor(gdt, ldt, &seg_ds64_cpl3);
  fill_segment_descriptor(gdt, ldt, &seg_tss32);
  fill_segment_descriptor(gdt, ldt, &seg_tss32_2);
  fill_segment_descriptor(gdt, ldt, &seg_tss32_cpl3);
  fill_segment_descriptor(gdt, ldt, &seg_tss32_vm86);
  fill_segment_descriptor(gdt, ldt, &seg_tss16);
  fill_segment_descriptor(gdt, ldt, &seg_tss16_2);
  fill_segment_descriptor(gdt, ldt, &seg_tss16_cpl3);
  fill_segment_descriptor_dword(gdt, ldt, &seg_tss64);
  fill_segment_descriptor_dword(gdt, ldt, &seg_tss64_cpl3);
  fill_segment_descriptor(gdt, ldt, &seg_cgate16);
  fill_segment_descriptor(gdt, ldt, &seg_tgate16);
  fill_segment_descriptor(gdt, ldt, &seg_cgate32);
  fill_segment_descriptor(gdt, ldt, &seg_tgate32);
  fill_segment_descriptor_dword(gdt, ldt, &seg_cgate64);

  if (ioctl(cpufd, KVM_SET_SREGS, &sregs))
    return -1;
  if (ioctl(cpufd, KVM_SET_REGS, &regs))
    return -1;
  return 0;
}

long r[3];
void loop()
{
  memset(r, -1, sizeof(r));
  syscall(__NR_mmap, 0x20000000ul, 0xfff000ul, 0x3ul, 0x32ul,
          0xfffffffffffffffful, 0x0ul);
  memcpy((void*)0x20750000, "/dev/kvm", 9);
  r[0] = syscall(__NR_openat, 0xffffffffffffff9cul, 0x20750000ul, 0x0ul,
                 0x0ul);
  r[1] = syscall(__NR_ioctl, r[0], 0xae01ul, 0x0ul);
  r[2] = syscall(__NR_ioctl, r[1], 0xae41ul, 0x0ul);
  *(uint64_t*)0x2059dfe8 = (uint64_t)0x40;
  *(uint64_t*)0x2059dff0 = (uint64_t)0x201aa000;
  *(uint64_t*)0x2059dff8 = (uint64_t)0x54;
  memcpy((void*)0x201aa000,
         "\xf0\x4e\x09\x01\x66\xba\x61\x00\x66\xb8\xca\x82\x66\xef\x41"
         "\x0f\x00\x1b\xc7\x44\x24\x00\x00\x30\x00\x00\xc7\x44\x24\x02"
         "\x01\x00\x00\x00\xc7\x44\x24\x06\x00\x00\x00\x00\x0f\x01\x14"
         "\x24\xc4\xe2\xf1\xab\x09\xb9\xc2\x00\x00\x00\x0f\x32\xb9\xb2"
         "\x02\x00\x00\x0f\x32\x0f\x00\xd6\x64\x0f\xad\x70\x5a\x0f\x20"
         "\xe0\x35\x00\x00\x01\x00\x0f\x22\xe0",
         84);
  *(uint64_t*)0x207b0000 = (uint64_t)0x7;
  *(uint64_t*)0x207b0008 = (uint64_t)0xc;
  *(uint64_t*)0x207b0010 = (uint64_t)0x8;
  STORE_BY_BITMASK(uint64_t, 0x207b0018, 0x0, 0, 1);
  STORE_BY_BITMASK(uint64_t, 0x207b0018, 0x7, 1, 5);
  STORE_BY_BITMASK(uint64_t, 0x207b0018, 0x0, 6, 4);
  STORE_BY_BITMASK(uint64_t, 0x207b0018, 0x2, 10, 2);
  STORE_BY_BITMASK(uint64_t, 0x207b0018, 0x0, 12, 1);
  STORE_BY_BITMASK(uint64_t, 0x207b0018, 0x890, 13, 2);
  STORE_BY_BITMASK(uint64_t, 0x207b0018, 0x0, 15, 1);
  STORE_BY_BITMASK(uint64_t, 0x207b0018, 0x20, 16, 48);
  syz_kvm_setup_cpu(r[1], r[2], 0x20844000ul, 0x2059dfe8ul, 0x1ul,
                    0xcul, 0x207b0000ul, 0x2ul);
}

int main()
{
  loop();
  return 0;
}
