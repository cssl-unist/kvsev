#include "util.h"
#include "sev.h"
#include <libmemcached/memcached.h>

#define MAX_VM		32
//#define NUM_LOAD	1000
//#define NUM_LOAD	10000000UL
#define NUM_LOAD	1000000UL

struct param {
	int thread_id;
};
typedef struct param Param;

/* Global Variables */
Arg arg;
pthread_mutex_t mutex;
pthread_barrier_t barrier;
static char ***keys 	= NULL;
static char ***vals 	= NULL;
static int **vmfd 		= NULL;
static int **vcpufd 	= NULL;
static int **handle		= NULL;
struct kvm_run ***run	= NULL;
uint8_t ***mem			= NULL;

static char **gen		= NULL;

char ***cmd_set 		= NULL;
char ***cmd_get 		= NULL;
int cmd_len				= 0;
int digits				= 0;

#if 0
memcached_server_st 	*server = NULL;
memcached_st 			*memc;
memcached_return 		rc;
#endif

struct timeval TEST_START;
struct timeval TEST_END;

const uint8_t code[] = {
	0xba, 0xf8, 0x03,
	0x00, 0xd8,
	0x04, '0',
	0xee,
	0xb0, '\n',
	0xee,
	0xf4,
};

void cleanup() {
#ifdef SEVAULT
	for (int i = 0; i < arg.num_threads; i++) {
		for (int j = 0; j < MAX_VM / arg.num_threads; j++) {
			close(vcpufd[i][j]);
			close(vmfd[i][j]);
		}
	}
	free(mem);
	free(vmfd);
	free(vcpufd);
	free(handle);
#endif
	free(keys);
	free(vals);
	free(gen);
}

void vm_init() {

	int kvm;
	uint8_t measure[48];

	vmfd = (int **)malloc(sizeof(int *) * arg.num_threads);
	vcpufd = (int **)malloc(sizeof(int *) * arg.num_threads);
	handle = (int **)malloc(sizeof(int *) * arg.num_threads);
	mem = (uint8_t ***)malloc(sizeof(uint8_t **) * arg.num_threads);

	for (int i = 0; i < arg.num_threads; i++) {
		vmfd[i] = (int *)malloc(sizeof(int) * (MAX_VM / arg.num_threads));
		vcpufd[i] = (int *)malloc(sizeof(int) * (MAX_VM / arg.num_threads));
		handle[i] = (int *)malloc(sizeof(int) * (MAX_VM / arg.num_threads));
		mem[i] = (uint8_t **)malloc(sizeof(uint8_t *) * (MAX_VM / arg.num_threads));
	}

	for (int i = 0; i < arg.num_threads; i++) {
		for (int j = 0; j < MAX_VM / arg.num_threads; j++) {
			kvm_vm_init(&kvm, &vmfd[i][j]);
			vcpufd[i][j] = sev_vm_init(vmfd[i][j]);
			handle[i][j] = sev_vm_create(vmfd[i][j], 0);
			mem[i][j] = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE,
							 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
			if (mem[i][j] == NULL) {
				fprintf(stdout, "[SEVault] failed to allocate guest memory\n");
				exit(1);
			}
			memcpy(mem[i][j], code, sizeof(code));
		}
	}

	for (int i = 0; i < arg.num_threads; i++) {
		for (int j = 0; j < MAX_VM / arg.num_threads; j++) {
			sev_vm_update(vmfd[i][j], mem[i][j], 0x1000);
			sev_vm_measure(vmfd[i][j], &measure, 48);
			sev_vm_finish(vmfd[i][j]);
		}
	}

	//close(kvm);
	return;
}

void configuration_init() {
	char LOCAL_IP[MAX_IP_ADDR] = "127.0.0.1";
	memcpy(arg.ipaddr, LOCAL_IP, MAX_IP_ADDR);
	arg.port_num = 11211;
	arg.max_key_size = 16;
	arg.max_val_size = 64;
	arg.num_threads = 4;
	arg.run_inst_size = -1;
	arg.get_ratio = 100;
}

void gen_test() {

	int tmp;
	
	for (int i = 0; i < arg.num_threads; i++) {
		for (int j = 0; j < arg.run_inst_size / arg.num_threads; j++) {
			int index = rand() % NUM_LOAD;
			for (int k = 0; k < arg.max_key_size; k++)
				keys[i][j][k] = gen[index][k];
		}
	}

	for (int i = 0; i < arg.num_threads; i++) {
		for (int j = 0; j < arg.run_inst_size / arg.num_threads; j++) {
			for (int k = 0; k < arg.max_val_size; k++) {
				tmp = rand() & 0x7f;
				while (tmp < 33 || tmp > 126)
					tmp = rand() & 0x7f;
				vals[i][j][k] = tmp;
			}
		}
	}

	for (int i = 0; i < arg.num_threads; i++) {
		for (int j = 0; j < arg.run_inst_size / arg.num_threads; j++) {
			for (int k = 0; k < arg.max_key_size; k++) {
				cmd_get[i][j][k + 4] = keys[i][j][k];
				cmd_set[i][j][k + 4] = keys[i][j][k];
				cmd_set[i][j][k + digits + 27] = vals[i][j][k];
			}
		}
	}
}

void file_to_memory() {
	FILE *fp;
	char *buf;
	int thread_id = 0;
	char filename[MAX_FILE_LEN];
	int set_count = 0;
	int size;
	int tmp;

	gen = (char **)malloc(sizeof(char *) * NUM_LOAD);
	for (int i = 0; i < NUM_LOAD; i++)
		gen[i] = (char *)malloc(sizeof(char) * arg.max_key_size);

	sprintf(filename, "gen_load/load_%d_%d.txt", arg.max_key_size, arg.max_val_size);
	fp = fopen(filename, "r");
	if (fp == NULL)
		fprintf("[SEVault] load file does not exist: %s\n", filename);
	for (int i = 0; i < NUM_LOAD; i++)
		getline(&gen[i], &size, fp);

	keys = (char ***)malloc(sizeof(char **) * arg.num_threads);
	for (int i = 0; i < arg.num_threads; i++) {
		keys[i] = (char **)malloc(sizeof(char *) * (arg.run_inst_size / arg.num_threads));
		for (int j = 0; j < arg.run_inst_size / arg.num_threads; j++)
			keys[i][j] = (char *)malloc(sizeof(char) * arg.max_key_size);
	}
#if 0
	for (int i = 0; i < arg.num_threads; i++) {
		for (int j = 0; j < arg.run_inst_size / arg.num_threads; j++) {
			int index = rand() % NUM_LOAD;
			for (int k = 0; k < arg.max_key_size; k++)
				keys[i][j][k] = gen[index][k];
		}
	}
#endif
#if 0
	sprintf(filename, "gen_load/load_%d_%d_%d.txt", arg.max_key_size, arg.max_val_size, arg.run_inst_size);
	//fprintf(stdout, "[SEVault] trace file name: %s\n", filename);
	fp = fopen(filename, "r");

	if (fp == NULL)
		fprintf(stdout, "[SEVault] Load file does not exist: %s\n", filename);
	for (int i = 0; i < arg.num_threads; i++) {
		for (int j = 0; j < arg.run_inst_size / arg.num_threads; j++)
			getline(&keys[i][j], &size, fp);
	}
#endif

	vals = (char ***)malloc(sizeof(char **) * arg.num_threads);
	for (int i = 0; i < arg.num_threads; i++) {
		vals[i] = (char **)malloc(sizeof(char *) * (arg.run_inst_size / arg.num_threads));
		for (int j = 0; j < arg.run_inst_size / arg.num_threads; j++)
			vals[i][j] = (char *)malloc(sizeof(char) * arg.max_val_size);
	}
#if 0
	/* generate random values */
	for (int i = 0; i < arg.num_threads; i++) {
		for (int j = 0; j < arg.run_inst_size / arg.num_threads; j++) {
			for (int k = 0; k < arg.max_val_size; k++) {
				tmp = rand() & 0x7f;
				while (tmp < 33 || tmp > 126)
					tmp = rand() & 0x7f;
				vals[i][j][k] = tmp;
			}
		}
	}
#endif
	cmd_len = 13 + arg.max_key_size + digits + arg.max_val_size;

	cmd_set = (char ***)malloc(sizeof(char **) * arg.num_threads);
	cmd_get = (char ***)malloc(sizeof(char **) * arg.num_threads);
	for (int i = 0; i < arg.num_threads; i++) {
		cmd_set[i] = (char **)malloc(sizeof(char *) * (arg.run_inst_size / arg.num_threads));
		cmd_get[i] = (char **)malloc(sizeof(char *) * (arg.run_inst_size / arg.num_threads));
		for (int j = 0; j < arg.run_inst_size / arg.num_threads; j++) {
			cmd_set[i][j] = (char *)malloc(sizeof(char) * cmd_len);
			cmd_get[i][j] = (char *)malloc(sizeof(char) * 22);
		}
	}

	for (int i = 0; i < arg.num_threads; i++) {
		for (int j = 0; j < arg.run_inst_size / arg.num_threads; j++) {
			cmd_get[i][j][0] = 'g';
			cmd_get[i][j][1] = 'e';
			cmd_get[i][j][2] = 't';
			cmd_get[i][j][3] = ' ';
			cmd_get[i][j][20] = '\r';
			cmd_get[i][j][21] = '\n';
#if 0
			for (int k = 0; k < arg.max_key_size; k++) {
				cmd_get[i][j][4 + k] = keys[i][j][k];
				cmd_set[i][j][4 + k] = keys[i][j][k];
			}
#endif
			cmd_set[i][j][0] = 's';
			cmd_set[i][j][1] = 'e';
			cmd_set[i][j][2] = 't';
			cmd_set[i][j][3] = ' ';
			cmd_set[i][j][20] = ' ';
			cmd_set[i][j][21] = '0';
			cmd_set[i][j][22] = ' ';
			cmd_set[i][j][23] = '0';
			cmd_set[i][j][24] = ' ';

			if (arg.max_val_size == 16) {
				cmd_set[i][j][25] = '1';
				cmd_set[i][j][26] = '6';
			}
			else if (arg.max_val_size == 64) {
				cmd_set[i][j][25] = '6';
				cmd_set[i][j][26] = '4';
			}
			else if (arg.max_val_size == 128) {
				cmd_set[i][j][25] = '1';
				cmd_set[i][j][26] = '2';
				cmd_set[i][j][27] = '8';
			}
			else if (arg.max_val_size == 256) {
				cmd_set[i][j][25] = '2';
				cmd_set[i][j][26] = '5';
				cmd_set[i][j][27] = '6';
			}
			else if (arg.max_val_size == 512) {
				cmd_set[i][j][25] = '5';
				cmd_set[i][j][26] = '1';
				cmd_set[i][j][27] = '2';
			}
			else if (arg.max_val_size == 1024) {
				cmd_set[i][j][25] = '1';
				cmd_set[i][j][26] = '0';
				cmd_set[i][j][27] = '2';
				cmd_set[i][j][28] = '4';
			}
			else if (arg.max_val_size == 2048) {
				cmd_set[i][j][25] = '2';
				cmd_set[i][j][26] = '0';
				cmd_set[i][j][27] = '4';
				cmd_set[i][j][28] = '8';
			}
			else if (arg.max_val_size == 4096) {
				cmd_set[i][j][25] = '4';
				cmd_set[i][j][26] = '0';
				cmd_set[i][j][27] = '9';
				cmd_set[i][j][28] = '6';
			}

			cmd_set[i][j][25 + digits] = '\r';
			cmd_set[i][j][26 + digits] = '\n';
			cmd_set[i][j][cmd_len - 2] = '\r';
			cmd_set[i][j][cmd_len - 1] = '\n';
#if 0
			for (int k = 0; k < arg.max_val_size; k++)
				cmd_set[i][j][27 + digits + k] = vals[i][j][k];
#endif
		}
	}
}

void *client_worker(void *data_) {
#if 0
	memcached_server_st 	*server = NULL;
	memcached_st 			*memc;
	memcached_return 		rc;
#endif
	int helper, sev;
	int vm_count = 0;
	struct kvm_sev_cmd cmd;
	struct sev_helper_deactivate_cmd deact;	

	pthread_mutex_lock(&mutex);
	Param *data = data_;
	
	int thread_id = data->thread_id;
	int tmp;
	int len;
	uint8_t measure[48];
	uint32_t flags;

	int client_len;
	int client_sockfd;
	struct sockaddr_in clientaddr;

	char *buf_in;
	char *buf_out;

	struct timeval START;
	struct timeval END;
	unsigned long time = 0;

	buf_in = (char *)malloc(sizeof(char) * 8192);
	buf_out = (char *)malloc(sizeof(char) * 8192);

	client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = inet_addr(arg.ipaddr);
	clientaddr.sin_port = htons(arg.port_num);
	client_len = sizeof(clientaddr);	

	pthread_mutex_unlock(&mutex);
	pthread_barrier_wait(&barrier);

	if (connect(client_sockfd, (struct sockaddr *)&clientaddr, client_len) < 0) {
		fprintf(stderr, "[SEVault] unable to connect to server: thread %d\n", thread_id);
		pthread_exit(NULL);
	}

	helper = open("/dev/etx_device", O_RDWR);
	if (helper < 0) {
		fprintf(stdout, "[SEVault] error opening helper device\n");
		exit(0);
	}

	int err;

#if 0
	memc = memcached_create(NULL);
	server = memcached_server_list_append(server, arg.ipaddr, arg.port_num, &rc);
	rc = memcached_server_push(memc, server);
	if (rc != MEMCACHED_SUCCESS) {
		fprintf(stdout, "[SEVault] Couldn't connect to server: thread %d\n", thread_id);
		pthread_exit(NULL);
	}
#endif
	for (int i = 0; i < arg.run_inst_size / arg.num_threads; i++) {
		
		tmp = rand() % 100;
		if (tmp < arg.get_ratio) {
#if 0
			memcached_get(memc, keys[thread_id][i], arg.max_key_size, &len, &flags, &rc);
			if (rc != MEMCACHED_SUCCESS)
				fprintf(stdout, "[SEVault] failed to retrieve key, errno: %d\n", rc);
#endif
			memset(buf_in, 0, 8192);
			strncpy(buf_in, cmd_get[thread_id][i], 22);	
			err = write(client_sockfd, buf_in, 22);
			if (err == -1) {
				fprintf(stderr, "[SEVault] failed to get value\n");
				pthread_exit(NULL);
			}
			err = read(client_sockfd, buf_out, 8192);
			if (err == -1) {
				fprintf(stderr, "[SEVault] failed to read response\n");
				pthread_exit(NULL);
			}
		}
		else {
			memset(buf_in, 0, 8192);
			strncpy(buf_in, cmd_set[thread_id][i], cmd_len);
			err = write(client_sockfd, buf_in, cmd_len);
			if (err == -1) {
				fprintf(stderr, "[SEVault] failed to set value\n");
				pthread_exit(NULL);
			}
			err = read(client_sockfd, buf_out, 8192);
			if (err == -1) {
				fprintf(stderr, "[SEVault] failed to set value\n");
				pthread_exit(NULL);
			}
#ifdef SEVAULT
			/* deactivate VM */
			sev = open_path_or_exit(SEV_DEV_PATH, 0);
			cmd.id = 0;
			deact.handle = handle[thread_id][vm_count];
			cmd.data = (uint64_t)(&deact);
			cmd.sev_fd = sev;
			ioctl(helper, SEV_DEACTIVATE, &cmd);

			if (vm_count == (MAX_VM / arg.num_threads) - 1) {
				ioctl(helper, SEV_WBINVD, &cmd);
				for (int j = 0; j < MAX_VM / arg.num_threads; j++) {
					//gettimeofday(&START, NULL);
					close(vcpufd[thread_id][j]);
					vcpufd[thread_id][j] = sev_vm_init(vmfd[thread_id][j]);
					handle[thread_id][j] = sev_vm_create(vmfd[thread_id][j], 0);
					sev_vm_update(vmfd[thread_id][j], mem[thread_id][j], 0x1000);
					sev_vm_measure(vmfd[thread_id][j], &measure, 48);
					sev_vm_finish(vmfd[thread_id][j]);
					//gettimeofday(&END, NULL);
					//time = (END.tv_sec - START.tv_sec) * 1000000 +
					  	//END.tv_usec - START.tv_usec;	

					//printf("test: %lu\n", time);
				}
				vm_count = 0;
			}
			else
				vm_count++;
			close(sev);
#endif
#if 0
			rc = memcached_replace(memc, keys[thread_id][i], arg.max_key_size, vals[thread_id][i], arg.max_val_size, (time_t)0, (uint32_t)0);
			if (rc != MEMCACHED_SUCCESS)
				fprintf(stdout, "[SEVault] failed to retrieve key, errno: %d\n", rc);
#endif
		}
	}
	close(helper);
}

int main(int argc, char **argv) {

	int status;
	int opt;
	int tmp;
	unsigned long execution_time = 0;

	configuration_init();
	
	if (argc == 1) {
		help();
		exit(0);
	}

	parse(argc, argv, (void *)&arg);
	//parse_option((void *)&arg);

	printf("\n===================================\n");
	printf("[SEVault] Starting TEST: %d-%d (%d)\n", arg.max_key_size, arg.max_val_size,
			arg.get_ratio);
	printf("===================================\n");

	srand(time(NULL));

#ifdef SEVAULT
	vm_init();
#endif

	pthread_t *threads;
	Param *parameter;

	pthread_mutex_init(&mutex, NULL);
	pthread_barrier_init(&barrier, NULL, arg.num_threads);

	threads = (pthread_t *)malloc(sizeof(pthread_t) * arg.num_threads);
	parameter = (Param *)malloc(sizeof(Param) * arg.num_threads);

	if (arg.max_val_size / 100 == 0)
		digits = 2;
	else if (arg.max_val_size / 1000 == 0)
		digits = 3;
	else if (arg.max_val_size / 10000 == 0)
		digits = 4;

	file_to_memory();

#if 0
	memc = memcached_create(NULL);
	server = memcached_server_list_append(server, arg.ipaddr, arg.port_num, &rc);
	rc = memcached_server_push(memc, server);
	if (rc != MEMCACHED_SUCCESS) {
		fprintf(stdout, "[SEVault] Couldn't connect to server\n");
		exit(0);
	}
#endif	
	gen_test();

	gettimeofday(&TEST_START, NULL);
	for (int i = 0; i < arg.num_threads; i++) {
		parameter[i].thread_id = i;
		pthread_create(&threads[i], NULL, &client_worker, (void *)&parameter[i]);
	}
	for (int i = 0; i < arg.num_threads; i++)
		pthread_join(threads[i], (void **)&status);

	gettimeofday(&TEST_END, NULL);
	execution_time += (TEST_END.tv_sec - TEST_START.tv_sec) * 1000000 +
					  	TEST_END.tv_usec - TEST_START.tv_usec;	

	printf("%lu\n", execution_time);
	cleanup();
	return 0;
}
