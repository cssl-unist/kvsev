#include "util.h"

void help() {
	printf("-h <help>\n");
	printf("-i <server ip address>\n");
	printf("-p <server port address>\n"); 
	printf("-k <key size in bytes>\n");
	printf("-v <val size in bytes>\n");
	printf("-n <number of operations\n");
	printf("-t <number of threads\n");
	printf("-r <GET ratio>\n");

	return;
}

void parse_option(void *arg_ptr) {

	Arg *arg = (Arg *)arg_ptr;
	printf("================================\n");
	printf("[SEVault] Workload Configuration\n");
	printf("================================\n");

	printf("ip\t%s\n", arg->ipaddr);
	printf("port\t%d\n", arg->port_num);
	printf("key\t%dB\n", arg->max_key_size);
	printf("val\t%dB\n", arg->max_val_size);
	printf("thread\t%d\n", arg->num_threads);
	printf("run\t%d\n", arg->run_inst_size);
	printf("ratio\t%d\n", arg->get_ratio);

	return;
}

void parse(int argc, char **argv, void *arg_ptr) {

	int opt;
	Arg *arg = (Arg *)arg_ptr;

	if (argc == 1) {
		help();
		exit(0);
	}

	while ((opt = getopt(argc, argv, "hi:p:k:v:n:t:r:")) != -1) {
		switch(opt) {
			case 'h':
				help();
				exit(0);
				break;
			case 'i':
				memcpy(arg->ipaddr, optarg, strlen(optarg));
				break;
			case 'p':
				arg->port_num = atoi(optarg);
				break;
			case 'k':
				arg->max_key_size = atoi(optarg);
				break;
			case 'v':
				arg->max_val_size = atoi(optarg);
				break;
			case 't':
				arg->num_threads = atoi(optarg);
				break;
			case 'n':
				arg->run_inst_size = atoi(optarg);
				break;
			case 'r':
				arg->get_ratio = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Illegal argument \"%c\"\n", opt);
				help();
				break;
		}
	}
}

