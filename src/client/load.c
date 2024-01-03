#include "util.h"
#include <errno.h>

//#define NUM_LOAD 1000
//#define NUM_LOAD 100000UL
//#define NUM_LOAD 10000000UL
#define NUM_LOAD	1000000UL

Arg arg;

int main(int argc, char **argv) {

	memcached_server_st *servers = NULL;
	memcached_st 		*memc;
	memcached_return 	rc;

	int client_len;
	int client_sockfd;
	struct sockaddr_in clientaddr;

	int tmp;
	char **key;
	char **val;

	char *cmd_in;

	char *buf_in;
	char *buf_out;

	FILE *fptr;
	char fname[100];

	if (argc == 1) { help(); exit(0); }
	parse(argc, argv, (void *)&arg);
	parse_option((void *)&arg);

	key = malloc(NUM_LOAD * sizeof(char *));
	val = malloc(NUM_LOAD * sizeof(char *));
	if (key == NULL || val == NULL) {
		fprintf(stderr, "[SEVault] failed to allocate memory (1)\n");
		exit(0);
	}

	for (int i = 0; i < NUM_LOAD; i++) {
		key[i] = malloc(arg.max_key_size * sizeof(char));
		val[i] = malloc(arg.max_val_size * sizeof(char));
		if (key[i] == NULL || val[i] == NULL) {
			fprintf(stderr, "[SEVault] failed to allocated memory (2)\n");
			exit(0);
		}
	}
	
	int digits;
	if (arg.max_val_size / 100 == 0)
		digits = 2;
	else if (arg.max_val_size / 1000 == 0)
		digits = 3;
	else if (arg.max_val_size / 10000 == 0)
		digits = 4;

	srand(time(NULL));

#if 0
	memc = memcached_create(NULL);
	servers = memcached_server_list_append(servers, arg.ipaddr, arg.port_num, &rc);
	rc = memcached_server_push(memc, servers);
	if (rc != MEMCACHED_SUCCESS)
		fprintf(stdout, "[SEVault] Couldn't connect to server\n");
#endif

	client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	clientaddr.sin_family = AF_INET;
	clientaddr.sin_addr.s_addr = inet_addr(arg.ipaddr);
	clientaddr.sin_port = htons(arg.port_num);
	client_len = sizeof(clientaddr);

	if (connect(client_sockfd, (struct sockaddr *)&clientaddr, client_len) < 0) {
		perror("[SEVault] unable to connect to the server\n");
		exit(0);
	}

	buf_in = (char *)malloc(sizeof(char) * 8192);
	//cmd_in = (char *)malloc(sizeof(char) * );
	buf_out = (char *)malloc(sizeof(char) * 8192);

	/* Populate KVS and write to load file */
	sprintf(fname, "gen_load/load_%d_%d.txt", arg.max_key_size, arg.max_val_size);
	fptr = fopen(fname, "w");

	printf("\n================================\n");
	printf("[SEVault] Populating KVS...");

	int cmd_len = 13 + arg.max_key_size + digits + arg.max_val_size;
	cmd_in = (char *)malloc(sizeof(char) * cmd_len);
	cmd_in[0] = 's';
	cmd_in[1] = 'e';
	cmd_in[2] = 't';
	cmd_in[3] = ' ';
	cmd_in[20] = ' ';
	cmd_in[21] = '0';
	cmd_in[22] = ' ';
	cmd_in[23] = '0';
	cmd_in[24] = ' ';

	if (arg.max_val_size == 16) {
		cmd_in[25] = '1';
		cmd_in[26] = '6';
	}
	else if (arg.max_val_size == 64) {
		cmd_in[25] = '6';
		cmd_in[26] = '4';
	}
	else if (arg.max_val_size == 128) {
		cmd_in[25] = '1';
		cmd_in[26] = '2';
		cmd_in[27] = '8';
	}
	else if (arg.max_val_size == 256) {
		cmd_in[25] = '2';
		cmd_in[26] = '5';
		cmd_in[27] = '6';
	}
	else if (arg.max_val_size == 512) {
		cmd_in[25] = '5';
		cmd_in[26] = '1';
		cmd_in[27] = '2';
	}
	else if (arg.max_val_size == 1024) {
		cmd_in[25] = '1';
		cmd_in[26] = '0';
		cmd_in[27] = '2';
		cmd_in[28] = '4';
	}
	else if (arg.max_val_size == 2048) {
		cmd_in[25] = '2';
		cmd_in[26] = '0';
		cmd_in[27] = '4';
		cmd_in[28] = '8';
	}
	else if (arg.max_val_size == 4096) {
		cmd_in[25] = '4';
		cmd_in[26] = '0';
		cmd_in[27] = '9';
		cmd_in[28] = '6';
	}

	cmd_in[25 + digits] = '\r';
	cmd_in[26 + digits] = '\n';

	cmd_in[cmd_len - 2] = '\r';
	cmd_in[cmd_len - 1] = '\n';

	for (int i = 0; i < NUM_LOAD; i++) {
        
		/* generate key */
        for (int j = 0; j < arg.max_key_size; j++) {
            tmp = rand()&0x7f;
            while (tmp < 48 || tmp > 57)
                tmp = rand() & 0x7f;
            key[i][j] = tmp;
			cmd_in[j + 4] = tmp;
			fprintf(fptr, "%c", key[i][j]);
        }
		fprintf(fptr, "\n");

        /* generate value */
        for (int j = 0; j < arg.max_val_size; j++) {
            tmp = rand() & 0x7f;
            while(tmp < 33 || tmp > 126)
                tmp = rand() & 0x7f;
            val[i][j] = tmp;
			cmd_in[27 + digits + j] = tmp;
        }

		memset(buf_in, 0, 8192);
		memset(buf_out, 0, 8192);

		strncpy(buf_in, cmd_in, cmd_len);
		int err = write(client_sockfd, buf_in, cmd_len);
		if (err == -1)
			fprintf(stderr, "[SEVault] failed to write to socket\n");
		err = read(client_sockfd, buf_out, 8192);
		if (strncmp(buf_out, "STORED\r\n", 8) != 0)
			fprintf(stderr, "[SEVault] failed to store key\n");

#if 0
		/* SET key-value */ 
        rc = memcached_set(memc, key[i], arg.max_key_size, val[i], arg.max_val_size, (time_t)0, (uint32_t)0);
        if (rc != MEMCACHED_SUCCESS) {
            printf("Couldn't store key: %s\n", memcached_strerror(memc, rc));
			exit(0);
		}
#endif
    }

	/* generate file to test */
	/* select random keys among loaded data */
	/* random values for REPLACE will be genearted in the test program */
#if 0
	for (int i = 0; i < NUM_LOAD; i++) {
		for (int j = 0; j < arg.max_key_size; j++)
			fprintf(fptr, "%c", key[i][j]);
		fprintf(fptr, "\n");
	}
#endif
    fclose(fptr);
	free(key);
	free(val);
	
	printf("Done!\n");
	printf("================================\n\n");

	return 0;	
}
