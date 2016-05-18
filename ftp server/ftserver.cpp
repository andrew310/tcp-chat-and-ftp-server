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
 * sends a message, detects if it is \quit
 * returns -1, 0 or 1 which allows parent function "chat" to determine if user desires to quit
 */
int sendMessage(int connection, fd_set *writefds){
    char msg[MAX];
    int length;
    int quit = 1;
    struct timeval tv;
    int rv;

    // clear the set ahead of time
    FD_ZERO(writefds);

    // add our descriptors to the set
    FD_SET(connection, writefds);

        //SEND MESSAGES
        if (FD_ISSET(connection, writefds)) {
            printf("%s%s","Arnold Schwarzenegger","> ");
            fgets(msg, MAX, stdin);
            length = strlen(msg);
            msg[length-1] = '\n';

            if(strncmp(msg, "\\quit", 5) == 0){
                quit = 0;
            }
        }
        if (send(connection, msg, length, 0) == -1){
            quit = -1;
            perror("send");
        }

    return quit;
}

/*Function: sendMessage
 * receives an int, which is the connected socket
 * receives a message and prints it to screen
 * returns -1, 0 or 1 which allows parent function "chat" to determine if client quit
 */
char** getMessage(int connection){
    char buf[MAX];
    int numbytes;
    char* args;
    char** tokens = malloc(sizeof(char*) * 512);

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
        printf("%s",buf);
        args = strtok(buf, "\n");
        int i = 0;
        while (args != NULL) {
            strcpy(cmd_args[i], args);
            args = strtok(NULL, "\n");
            tokens[i] = arg;
            i++;
        }
    }
    return tokens;
}

/*Function: chat
 * receives an int, which is the host socket
 * messages are received and sent until one party disconnects
 */
void chat(int socket, fd_set *readfds, fd_set *writefds){
    int stopChat = 1;
    //note the 1 in while(1) is unrelated to stopChat
    while(1){
        if(getMessage(socket, readfds) < 1){
            printf("chat ended by client");
        }

        stopChat = sendMessage(socket, writefds);
        if (stopChat== 0){
            printf("chat ended by host");
            close(socket);
            exit(0);
        } else if (stopChat == -1){
            perror("Client connection lost");
            exit(0);
        } else{

        }
    }
    printf("chat ended");
    return;
}

MSG_SIZE = 500;

int main(int argc, char *argv[])
{
	int serverSocket, connectionSocket, dataSocket;  // socket file descriptors
	struct addrinfo options, *servinfo, *p;
	struct sockaddr_storage their_addr; // client information
	fd_set readfds,writefds; // flags will use
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN]; //THIS WILL HOLD INCOMING IP ADDRESS
	int rv;
	bool keepChatting;
    char msgBuffer[MSG_SIZE];

    //handle incorrect usage
	if (argc != 2) {
	    std::cerr << "Usage: " << argv[0] << "<port number>" << std::endl;
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

    sin_size = sizeof their_addr;
    connectionSocket = accept(serverSocket, (struct sockaddr *)&their_addr, &sin_size);
    //inet_ntop converts IPv4 and IPv6 addresses from binary to text form
    //see: http://man7.org/linux/man-pages/man3/inet_ntop.3.html
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
    printf("server: got connection from %s\n", s);




    //the main loop to accept incoming connection
	while(1) {
        //TODO: receive
        close(serverSocket); // child doesn't need the listener
        close(connectionSocket);  // parent doesn't need this


	}//end of while loop

	return 0;
}
