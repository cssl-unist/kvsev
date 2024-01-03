all: load sevault
load:
	gcc load.c util.c -o load -lmemcached
sevault:
	gcc multithread.c util.c sev.c -o sevault -DSEVAULT -lpthread -lmemcached
clean:
	rm load sevault gen_load/*
