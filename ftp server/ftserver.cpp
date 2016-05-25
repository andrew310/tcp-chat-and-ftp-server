//
//  ftserver.cpp
//  file transfer server
//
//  Created by andrew on 5/9/16.
// references: Beej's guide

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //for forks
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> //for addrinfo
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <cassert>
#include <sys/select.h>


#define BACKLOG 10	 // how many pending connections queue will hold
#define MAX 500 //max message size



void sigchld_handler(int s)
{
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*Function: serverSetup
 * Parameters: int for socket, addrinfo structs
 * makes a socket and binds it
 * returns socket
 * a lot of help from: https://beej.us/guide/bgnet/output/html/multipage/index.html
 */
int serverSetup(int serverSocket, struct addrinfo *p, struct sigaction sa, struct addrinfo *servinfo, int yes){
    	/* remember getaddrinfo returned list of address structures
    	 * we need to loop through them until we can bind one
    	 */
	for(p = servinfo; p != NULL; p = p->ai_next) {
            /*MAKE SOCKET */
		if ((serverSocket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
        //allows us to reuse the socket
		if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
        /*BIND SOCKET*/
        unlink("127.0.0.1");
		if (bind(serverSocket, p->ai_addr, p->ai_addrlen) == -1) {
			close(serverSocket);
			perror("server: bind");
			continue;
		}
		break;
	} //END OF LOOP

	/* clean up memory associated with servinfo struct
	 * we don't need it since it was just a list of available options
	 */
	freeaddrinfo(servinfo);

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

    /*LISTEN*/
	if (listen(serverSocket, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
    //return socket
    return serverSocket;
}//END OF SERVERSETUP


/*Function: sendMessage
 * receives an int, which is the connected socket
 * sends a message
 */
void sendMessage(char* msg, int length, int connection){

    if (send(connection, msg, length, 0) == -1){
        perror("send");
    }

}

/*Function: sendMessage
 * receives an int, which is the connected socket
 * receives command argument(s) from client and prints to screen
 * returns tokenized array containing commands
 */
char** getMessage(int connection){
    char buf[MAX];
    int numbytes;
    char* arg = NULL;
    char** tokens = (char**)malloc(sizeof(char*) *6);

    printf("waiting for incoming commands on port number: %d\n", connection);
    //waits for message from client
    numbytes = recv(connection, buf, MAX-1, 0);

    //if connection lost and there is an error, break looP and print error message
    if (numbytes == -1) {
        perror("recv");
        exit(1);
    }
    /* if connection lost but no error, break loop with no error
     * http://stackoverflow.com/questions/2416944/can-read-function-on-a-connected-socket-return-zero-bytes
     */
    else if (numbytes == 0) {
        printf("Connection lost.\n");
        exit(0);
    }
    //no error, print the message
    else {
        //buf[numbytes] = '\0';
        printf("Message: %s\n",buf);
        arg = strtok(buf, "\n");
		arg = strtok(buf, " ");
		printf("arg: %s\n", arg);
        int i = 0;
        while (arg != NULL) {
            arg = strtok(NULL, " ");
            tokens[i] = arg;
            i++;
        }
    }
    return tokens;
}

/*Function: execCMd
 * receives tokenized array containing arguments, int for client fd, int for data fd
 * returns executes commands based on the arguments
 */
void execCmd(char** args, int connectionFd, char* ip){
	printf("received arg: %s\n", args[0]);

	//set up data socket client
	int sockfd, numbytes;
	char buf[MAX];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	char* dataPort;


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	//these if statements are to find where the port # for the data connection are in the command array
	if(strncmp(args[0], "-1", 2) == 0){
		dataPort = args[1];
		printf("dataport: %s\n", dataPort);
	}

	else if(strncmp(args[0], "-g", 2) == 0){
		dataPort = args[2];
		printf("dataport: %s\n", dataPort);
	}

	//we are passing in "ip" which is the address information for the client connection
	//basically using it and the port number we rcvd to create a new connection
	if ((rv = getaddrinfo(ip, dataPort, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		//make connection
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

	if (p == NULL) {
		fprintf(stderr, "ftserver: failed to connect on data socket\n");
		exit(1);
	}
	//get string form of address so we can print it
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	s, sizeof s);
	printf("ftserver: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

		break;
	}

    //if user wants to see list of files
    if(strncmp(args[0], "-1", 2) == 0){
				//ref: http://stackoverflow.com/questions/671461/how-can-i-execute-external-commands-in-c-linux
        FILE *dir = popen("ls", "r");
        char buff[500];
        int length;
        fread(&buff[0], sizeof buff[0], 500, dir);
        int i;
        while(buff[i] != 0){
            i++;
        }
        length = i;
		printf("sending...\n");

        sendMessage(buff, length, sockfd);
    }

	//if user wants to download a file
	//read file into byte array
	//ref: http://www.codecodex.com/wiki/Read_a_file_into_a_byte_array
	//ref2: http://stackoverflow.com/questions/5594042/c-send-file-to-socket
	else if (strncmp(args[0], "-g", 2) == 0 && args[1] != NULL) {
		printf("opening file: %s\n", args[1]);
		FILE *fl = fopen(args[1], "r");
		if (fl == NULL) {
				perror("error opening file");
		}//TODO: add current directory information
		fseek(fl, 0, SEEK_END);
		long len = ftell(fl);
		char *ret = (char*)malloc(len);
		fseek(fl, 0, SEEK_SET);
		printf("sending the goods...\n");
		size_t nbytes = 0;
		while ((nbytes = fread(ret, 1, 500, fl)) > 0) {
			send(sockfd, ret, nbytes, 0);
		}
		fclose(fl);
		printf("done sending the goods\n");

	}
	//invalid command received
	else{
		char errmsg[] = "ftserver: invalid command";
		sendMessage(errmsg, 15, sockfd);
	}
	//we dont need this connection anymore
	close(sockfd);
}//end of execCmd


int main(int argc, char *argv[])
{
	int serverSocket, connectionSocket, dataSocket, dataFd;  // socket file descriptors
	struct addrinfo options, *servinfo, *p;
	struct sockaddr_storage their_addr; // client information
	fd_set readfds,writefds; // flags will use
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN]; //THIS WILL HOLD INCOMING IP ADDRESS
	int rv;
	bool keepChatting;
  	char msgBuffer[MAX];
	char** tokens = NULL;

    //handle incorrect usage
	if (argc != 2) {
	    perror("Usage: arg <port number>");
	    exit(1);
	}

    /* We are initiating the addrinfo members ai_family, ai_socktype and ai_flags below to the int "0" passed to the memset function.
     * all memset does is for the sizeof, fill the memory referenced by options with the int we pass, which is 0
     */
	memset(&options, 0, sizeof options);
	//now that we have initiated, specify what we want
	options.ai_family = AF_UNSPEC; //allows for IPv4 or IPv6
	options.ai_socktype = SOCK_STREAM; //sock stream means TCP
	options.ai_flags = AI_PASSIVE; // wildcard IP, fills in my IP

    //getaddrinfo returns a list of address structures
    //we pass in the OPTIONS specified above, and results are in the servinfo
    rv = getaddrinfo(NULL, argv[1], &options, &servinfo);
    //if getaddrinfo didn't return errorless
	if (rv != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

    //call to serverSetup function after which serverSocket will contain the socket number
    serverSocket = serverSetup(serverSocket, p, sa, servinfo, yes);
	printf ("The magic happens on port: \"%s\".\n",argv[1]);

    //reap dead children processes
	sa.sa_handler = sigchld_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

  //the main loop to accept incoming requests
	while(1) {

        sin_size = sizeof their_addr;
        connectionSocket = accept(serverSocket, (struct sockaddr *)&their_addr, &sin_size);
        //inet_ntop converts IPv4 and IPv6 addresses from binary to text form
        //see: http://man7.org/linux/man-pages/man3/inet_ntop.3.html
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

		if (!fork()) { // this is the child process
			close(serverSocket); // child doesn't need the listener
      		tokens = getMessage(connectionSocket);
			printf("token: %s\n", tokens[0]);
			execCmd(tokens, connectionSocket, s);
			close(dataSocket);
			exit(0);
		}
		close(connectionSocket);  // parent doesn't need this


	}//end of while loop

	return 0;
}
