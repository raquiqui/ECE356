#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //socket api -use this to create socket
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

//2 fxns server and client are declared
int server(uint16_t port); //takes port number as arg 
//set up server to listen on this port
int client(const char * addr, uint16_t port); //takes IP/port number s args
//creates client that connects to server at specified addres/port

#define MAX_MSG_LENGTH (1300) //max length of message 
#define MAX_BACK_LOG (5)	  //max length of server's listen queue

int main(int argc, char ** argv)
{	//checks if correct number of command-line args is given
	if (argc < 3 || (argv[1][0] == 'c' && argc < 4)) {
		printf("usage: myprog c <port> <address> or myprog s <port>\n");
		return 0;
	}

	//converts provided port number to an integer
	uint32_t number = atoi(argv[2]);

	//validates port number range
	if (number < 1024 || number > 65535) {
		fprintf(stderr, "port number should be larger than 1023 and less than 65536\n");
		return 0;
	}

	//converts port number to 16 bit integer
	uint16_t port = atoi(argv[2]);
	
	//checks command type and calls approp. fxn 
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

/*Client implementation complete?*/
int client(const char * addr, uint16_t port)
{
	int sock; //stores socket descriptor
	struct sockaddr_in server_addr;  //declares a structure
	//declares char arrays to store message to be sent and the server reply
	char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH];  

	//creates TCP socket using socket fxn -> returns error message if fails (i.e. socket <0)
	if ((sock = socket(AF_INET, SOCK_STREAM/* use tcp */, 0)) < 0) { //SOCK_STREAM = reliable stream (for ip is TCP), when 3rd arg=0, OS decides
		perror("Create socket error:");
		return 1;
	}

	//must first assign sockel local ip/port number, cant just open/read/write data to socket 
	//must connect other end of socket to a remote machine w bind and connect
	printf("Socket created\n");
	server_addr.sin_addr.s_addr = inet_addr(addr); //converts IP address (addr) to binary and sets it in server_addr structure
	server_addr.sin_family = AF_INET; //specifies that address family is IPv4 
	server_addr.sin_port = htons(port); //converts provided port # (port) to network byte order and sets in server_addr struct

	//invokes int connect(int socket, struct sockaddr *address, int addr_len)
	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { //if fails, use perror and return 1 
		perror("Connect error:");
		return 1;
	}

	printf("Connected to server %s:%d\n", addr, port); //prints indication of connection
	printf("Enter message: \n"); //prompts user to enter message
	
	//loop to send/recieve messages
	while (fgets(msg, MAX_MSG_LENGTH, stdin)) { 

		//send message to server
		if (send(sock, msg, strnlen(msg, MAX_MSG_LENGTH), 0) < 0) {
			perror("Send error:");
			return 1;
		}
		int recv_len = 0;

		//recieves reply from server
		if ((recv_len = recv(sock, reply, MAX_MSG_LENGTH, 0)) < 0) {
			perror("Recv error:");
			return 1;
		}

		//checks if server has disconnected
		if (recv_len == 0){
			printf("Server disconnected, client quit.\n");
			break;
		}

		//null-terminate the recieved message
		reply[recv_len] = '\0';
		printf("Server reply:\n%s", reply); //print server reply
		printf("Enter message: \n");
	}

	//sends empty message to signal at end of communication
	if (send(sock, "", 0, 0) < 0) {
		perror("Send error:");
		return 1;
	}
	return 0;
}


/*Server implementation here*/
int server(uint16_t port)
{	int sock; //stores socket descripter 
	struct sockaddr_in server_addr; //declares a struct -WHAT DOES THIS MEAN

	//structs??
	struct in_addr { //addr must be of structure sockaddr_in
		u_int_t s_addr
	}
	struct sockaddr_in {
		short sin_family;
		u_short sin_port;
		struct in_addr sin_addr;
		char sin_zero[8]; 
	}

	char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH]; //declares char arrays to store message to be sent and the server reply

	//specify address of this server
	server_addr.sin_family = AF_INET; //specifies that address family is IPv4 
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //accepts connections from ANY IP ADDRESS
	server_addr.sin_port = htons(port); //converts provided port # (port) to network byte order and sets in server_addr struct



	//creates TCP socket using socket fxn -> returns error message if fails (i.e. socket <0)
	if ((sock = socket(AF_INET, SOCK_STREAM/* use tcp */, 0)) < 0) { //SOCK_STREAM = reliable stream (for ip is TCP), when 3rd arg=0, OS decides
		perror("Create socket error:");
		return 1;
	}

	//must first assign sockel local ip/port number, cant just open/read/write data to socket 
	//must connect other end of socket to a remote machine w bind and connect
	printf("Socket created\n");
	

	/*allow program to run again even if old connections in TIME_WAIT -> need to avoid seeing "address already
	in use" errors when killing and restarting daemon frequently*/
	int n = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&n, sizeof(n)) < 0){
		perror("SO_REUSEADDR");
		close(sock);
		return -1;
	}
	fcntl(s, F_SETFD, 1); 

	//use bind function to bind socket to ip address
	if (bind(int sock, struct sockaddr *address, int addr_len) < 0) { //if fails, use perror and return 1 
		perror("bind error:");
		return 1;
	}

	//listen for client -defines how many connections can be pending on a specified socket
	if (listen(int sock, int backlog) < 0) { //if fails, use perror and return 1 
		perror("listen error:");
		return 1;
	}

	//invokes int accept (accepts connection)
	if (accept(int sock, struct sockaddr *address, int *addr_len) < 0) { //if fails, use perror and return 1 
		perror("accept error:");
		return 1;
	}
	/*blocking operation
	-doesnt return until a remote participant has established a connection 
	-when it does, returns a new socket that correponds to the new estalished connection and 
	address argument contains remote participant address*/

	//invokes int connect(int socket, struct sockaddr *address, int addr_len)
	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { //if fails, use perror and return 1 
		perror("Connect error:");
		return 1;
	}


	int send(int sock, char *msg, int msg_len, int flags) //flags default=0

	int recv(int sock, char *buff, int buff_len, int flags) //flags default=0

	return sock;
}

/*read line of input from file descriptor and return it*/
static char* 
readline(int sock){
	char *buf = NULL, *nbuf;
	int buf_pos=0, buf_len=0;
	int i, n;

	for(;;){
		//ensure there is room in buffer
		if(buf_pos == buf_len){
			buf_len = buf_len ? buf_len <<1 :4;
			nbuf = realloc(buf, buf_len);

			if(!nbuf){
				free(buf);
				return NULL;
			}
			buf = nbuf;
		}

		//read some data into buffer
		n = read(s, buf + buf_pos, buf_len - buf_pos);
		if (n <= 0){
			if(n<0)
				perror("read");
			else
				fprintf(stderr, "read:EOF\n");
			free(buf);
			return NULL;
		}

		/*look for end of a line and return if we got it. be generous in what is considered end of line*/
		for(i=buf_pos; i<buf_pos+n; i++){
			if(buf[i] == '\0' || buf[i] == '\r' || buf[i]=='\n'){
				buf[i]='\0';
				return buf;
			}
			buf_pos += n;
		}

	}
}

