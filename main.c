#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

int server(uint16_t port);
int client(const char * addr, uint16_t port);

#define MAX_MSG_LENGTH (1300)
#define MAX_BACK_LOG (5)

int main(int argc, char ** argv)
{
	if (argc < 3 || (argv[1][0] == 'c' && argc < 4)) {
		printf("usage: myprog c <port> <address> or myprog s <port>\n");
		return 0;
	}

	uint32_t number = atoi(argv[2]);
	if (number < 1024 || number > 65535) {
		fprintf(stderr, "port number should be larger than 1023 and less than 65536\n");
		return 0;
	}

	uint16_t port = atoi(argv[2]);
	
	if (argv[1][0] == 'c') {
		return client(argv[3], port);
	} else if (argv[1][0] == 's') {
		return server(port);
	} else {
		fprintf(stderr, "unkonwn commend type %s\n", argv[1]);
		return 0;
	}
	return 0;
}

int client(const char * addr, uint16_t port)
{
	int sock;
	struct sockaddr_in server_addr;
	char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH];

	if ((sock = socket(AF_INET, SOCK_STREAM/* use tcp */, 0)) < 0) {
		perror("Create socket error:");
		return 1;
	}

	printf("Socket created\n");
	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Connect error:");
		return 1;
	}

	printf("Connected to server %s:%d\n", addr, port);
	printf("Enter message: \n");
	while (fgets(msg, MAX_MSG_LENGTH, stdin)) {
		if (send(sock, msg, strnlen(msg, MAX_MSG_LENGTH), 0) < 0) {
			perror("Send error:");
			return 1;
		}
		int recv_len = 0;
		if ((recv_len = recv(sock, reply, MAX_MSG_LENGTH, 0)) < 0) {
			perror("Recv error:");
			return 1;
		}
		if (recv_len == 0){
			printf("Server disconnected, client quit.\n");
			break;
		}
		reply[recv_len] = '\0';
		printf("Server reply:\n%s", reply);
		printf("Enter message: \n");
	}
	if (send(sock, "", 0, 0) < 0) {
		perror("Send error:");
		return 1;
	}
	return 0;
}



/*Server implementation here*/
int server(uint16_t port)
{	int sock; //stores socket descripter 
	int new_sock;
	char buff[MAX_MSG_LENGTH]; 
	struct sockaddr_in server_addr; //declares server/client socket address structs
	int len;

	//specify address of this server
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET; //specifies that address family is IPv4 
	server_addr.sin_addr.s_addr = htons(INADDR_ANY); //accepts connections from ANY IP ADDRESS
	server_addr.sin_port = htons(port); //converts provided port # (port) to network byte order and sets in server_addr struct

	//creates TCP socket using socket fxn -> returns error message if fails (i.e. socket <0)
	if ((sock = socket(AF_INET, SOCK_STREAM/* use tcp */, 0)) < 0) { //SOCK_STREAM = reliable stream (for ip is TCP), when 3rd arg=0, OS decides
		perror("Create socket error:");
		return 1;
	}

	//must first assign sockel local ip/port number, cant just open/read/write data to socket 
	//must connect other end of socket to a remote machine w bind and connect
	printf("Socket created\n");

	//use bind function to bind socket to ip address
	if (bind(sock, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) { //if fails, use perror and return 1 
		perror("bind error:");
		return 1;
	}

	//listen for client -defines how many connections can be pending on a specified socket
	listen(sock, MAX_BACK_LOG);

	//&server_addr gives address of server
	while(1){
		if(new_sock = accept(sock, (struct sockaddr *)&server_addr, &len) < 0){
			perror("accept error:");
			exit(1);
		}
		while((len = recv(new_sock, buff, sizeof(buff), 0)) > 0){
			send(new_sock, buff, len, 0); //flags default=0
		}
		close(new_sock);
	}
	return 0;
}