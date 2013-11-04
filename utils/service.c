#include<stdio.h>
#include<string.h>
#include<stdlib.h>

int main(int argc, char **argv) {

	printf("Hello World\n");

	int server_port = 8080;
	int num_servers = 0;

	if(argc != 3) {
		printf("Usage: /bin/service <number for servers to start, between 3-11>\n");
		return 1;
	}

	num_servers = atoi(argv[1]);

	if(!strcmp(argv[2], "start")) {
		printf("service start called\n");
		for(size_t i = 0; i < num_servers ; ++i) {
			printf("Hey   %d\n", server_port + i);
		}	
	}

	if(!strcmp(argv[2], "stop")) {
	
	}
	

	return 0;
}
