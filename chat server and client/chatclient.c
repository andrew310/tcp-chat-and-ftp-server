//
//  chatclient.c
//  chatclient
//
//  Created by andrew on 4/19/16.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <assert.h>
# define PROMPT	"> "
#define MAXDATASIZE 500// max number of bytes we can get at once

char username[10];

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//function to set non blocking flag
void set_nonblock(int socket) {
    int flags;
    flags = fcntl(socket,F_GETFL,0);
    assert(flags != -1);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK); //set socket to non blocking
}


/*Function: sendMessage
 * receives an int, which is the connected socket
 * sends a message, detects if it is \quit
 * returns -1, 0 or 1 which allows parent function "chat" to determine if user desires to quit
 */
int sendMessage(int connection, fd_set *writefds){
    char msg[MAXDATASIZE], formattedMsg[MAXDATASIZE];
    int length;
    int quit = 1;

    // clear the set ahead of time
    FD_ZERO(writefds);

    // add our descriptors to the set
    FD_SET(connection, writefds);

    //SEND MESSAGES
    if (FD_ISSET(connection, writefds)) {
        printf("%s%s",username,PROMPT);
        fgets(msg, MAXDATASIZE, stdin);
        if(strncmp(msg, "\\quit", 5) == 0){
            quit = 0;
        }
    sprintf(formattedMsg, "\r%s%s%s\n", username, PROMPT, msg);
    }
    length = strlen(formattedMsg);

    if (send(connection, formattedMsg, length, 0) == -1){
        quit = -1;
        perror("send");
    }
    return quit;
}

/*Function: chat
 * receives an int, which is the host socket
 * messages are received and sent until one party disconnects
 */
void chat(int socket, fd_set *readfds, fd_set *writefds){
    char buf[MAXDATASIZE];
    int stopChat = 1;
    int numbytes;
    pthread_t thread;
    struct timeval tv;
    int rv;

    //RECEIVE MESSAGES
    while(1){
        // clear the set ahead of time
        FD_ZERO(readfds);

        // add our descriptors to the set
        FD_SET(socket, readfds);

        //will time out after this much time with no incoming message
        tv.tv_sec = 1000;
        tv.tv_usec = 500000;

        /*SEND*/
        stopChat = sendMessage(socket, writefds);
            if (stopChat== 0){
            printf("chat ended by host");
            close(socket);
            exit(0);
        }
        else if (stopChat == -1){
            perror("Client connection lost");
            exit(0);
        }

        /*RECEIVE*/
        //make ASYNCHRONOUS
        rv = select(socket+1, readfds, NULL, NULL, &tv);

        if (rv == -1) {
        perror("select"); // error occurred in select()
        } else if (rv == 0) {
            printf("Timeout occurred!  No data after 1000 seconds.\n");
        } else {
            // one or both of the descriptors have data
            if (FD_ISSET(socket, readfds)) {
                numbytes = recv(socket, buf, MAXDATASIZE-1, 0);
            }
        }

        //if connection lost and there is an error, break look and print error message
        if (numbytes == -1) {
            perror("recv");
            exit(1);
        }
        /* if connection lost but no error, break loop with no error
         * http://stackoverflow.com/questions/2416944/can-read-function-on-a-connected-socket-return-zero-bytes
         */
        else if (numbytes == 0) {
            printf("Host has left the chat.\n");
            exit(0);
        }
        //no error, print the message
        else {
            buf[numbytes] = '\0';
            printf("Arnold Schwarzenegger> %s \n",buf);
        }
    }
    //done chatting, close socket
	close(socket);
}


int main(int argc, char *argv[]){
	int sockfd;
	int rv;
	int length;
    struct addrinfo hints, *servinfo, *p;
	char s[INET6_ADDRSTRLEN];
	fd_set readfds,writefds; // flags will use

    //invalid command line input handler
	if (argc != 3) {
	    fprintf(stderr,"usage: client hostname portnum\n");
	    exit(1);
	}

    printf("Enter username: ");
    fgets(username, MAXDATASIZE, stdin);
    length = strlen(username);
    username[length-1] = '\0'; //replace newline with endstring

    //initialize hints to 0
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; //allow IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; //use TCP

    //get linked list of address structures
	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the addresses and find one we can connect to
	for(p = servinfo; p != NULL; p = p->ai_next) {
        //make socket
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
        //connect
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			printf("...trying to find another...\n");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
    set_nonblock(sockfd);

    //inet_ntop converts IPv4 and IPv6 addresses from binary to text form
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

    //pass socket and read/write flags to the chat function
    chat(sockfd, &readfds, &writefds);

	return 0;
}
