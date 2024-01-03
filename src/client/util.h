#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>

#include <linux/kvm.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <libmemcached/memcached.h>

#define MAX_IP_ADDR		32
#define MAX_FILE_LEN	70

struct argument {
	char ipaddr[MAX_IP_ADDR];
	int port_num;
	int num_threads;
	int max_key_size;
	int max_val_size;
	int run_inst_size;
	int get_ratio;
};
typedef struct argument Arg;

void help();
void parse_option(void *arg_ptr);
void parse(int argc, char **argv, void *arg_ptr);

#endif /* _UTIL_H_ */
