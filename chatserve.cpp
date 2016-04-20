//
//  chatserve.cpp
//  chatserve
//
//  Created by andrew on 4/19/16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

int main(int argc, const char * argv[]) {
    // insert code here...
    int serverSocket, newSocket;
    socklen_t addSize;
    
    struct sockaddr_in serverAddress;
    struct sockaddr_storage clientAddress;
    
    char *msg = "hallo there matey!";
    int msgLen, bytes_sent;
    
    /*SERVER CONFIG*/
    //set up socket and port
    //parameters: internet domain, stream socket and protocol (0 = tcp)
    serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    
    //set server address family to internet
    serverAddress.sin_family = AF_INET;
    //set server address to any
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    //select a port number
    serverAddress.sin_port = htons(3012);
    
    //bind the address to the socket
    bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof serverAddress);
    
    //listen
    listen(serverSocket, 5);
    std::cout << "the magic happens on port 3012";
    
    //accept incoming connection
    addSize = sizeof clientAddress;
    newSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &addSize);
    
    //send message
    msgLen = strlen(msg);
    bytes_sent = send(newSocket, msg, msgLen, 0);
    
    
    return 0;
}
