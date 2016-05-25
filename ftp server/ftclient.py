#!/usr/bin/python
# Andrew Brown
# cs372-400 Project 2

import sys
from socket import *
import signal
import fcntl, os
import SocketServer


#function: sendCommand
#takes socket file descriptor and msg string
#sends to specified socket
def sendCommand(socket, msg):
    print "ftclient: sending the command to " + sys.argv[1]
    socket.send(msg)

#function: getResponse
#takes socket file descriptors
#waits for message on socket
def getResponse(socket):
    response = ""
    try:
        response = socket.recv(500)
    except: pass
    return response

def main():

    argsCount = len(sys.argv)
    context = ""
    if(len(sys.argv) < 5):
        print "Usage: ./ftclient.py <hostname> <port number> <arg> <data port number>"
        sys.exit(0)

    if (sys.argv[3] == '-g' and argsCount < 6):
        print "Usage: ./ftclient.py <hostname> <port number> <arg> <data port number>"
        sys.exit(0)

    #basically just building a string out of the argv just like how it would appear in c
    elif(len(sys.argv) == 5):
        context = " ".join([sys.argv[2], sys.argv[3], sys.argv[4]])
    elif(len(sys.argv) == 6):
        context = " ".join([sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5], "\n"])


    #print len(sys.argv)

    #server socket information
    hostName = sys.argv[1]
    hostPort = int(sys.argv[2]);
    #assign client socket, use my ip
    clientSocket = socket(AF_INET, SOCK_STREAM)

    #connect
    clientSocket.connect((hostName, hostPort))
    sendCommand(clientSocket, context)

    #separate socket for receiving data
    host = ''
    port = int(sys.argv[argsCount-1])
    backlog = 5
    dataSocket = socket(AF_INET, SOCK_STREAM)
    dataSocket.bind((host, port))

    #close sockets safely when we ctrl+c
    def handler(sig, frame):
        clientSocket.close()
    #setup for signal interrupt to close socket
    signal.signal(signal.SIGINT, handler)

    dataSocket.listen(backlog)
    print "listening on data socket"

    response = clientSocket.recv(500)
    clientSocket.close()
    print response
    if "not exist" in response:
        sys.exit(0)

    elif "invalid" in response:
        sys.exit(0)

    #handles the file transfer
    if sys.argv[3] == '-g':

        file = open(sys.argv[4] + ".downloaded", "a")

        #accept incoming connections on data socket
        #ref: http://ilab.cs.byu.edu/python/socket/echoserver.html
        while 1:
            client, address = dataSocket.accept()
            while 1:
                res = client.recv(1024)
                if not res:
                    break
                file.write(res)
            break

        file.close
        client.close()
        print "received " + sys.argv[4] + '.download'

    #ls requests
    else:

        #accept incoming connections on data socket
        #ref: http://ilab.cs.byu.edu/python/socket/echoserver.html
        while 1:
            client, address = dataSocket.accept()
            res = client.recv(1024)
            if not res:
                break
            print res
            client.close()
            break

if __name__ == "__main__":
    main()
