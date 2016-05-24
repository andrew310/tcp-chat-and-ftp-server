#!/usr/bin/python
# Andrew Brown
# cs372-400 Project 2

import sys
from socket import *
import signal
import os

MSGLEN = 100

def sendCommand(socket, msg):
    print "ftclient: sending the command to " + sys.argv[1]
    socket.send(msg)

def getResponse(socket):
    response = ""
    try:
        response = socket.recv(500)
    except: pass
    return response

def main():

    context = ""
    if(len(sys.argv) < 5):
        print "Usage: ./ftclient.py <hostname> <port number> <arg> <data port number>"
        sys.exit(0)
        #basically just building a string out of the argv just like how it would appear in c
    elif(len(sys.argv) == 5):
        context = " ".join([sys.argv[2], sys.argv[3], sys.argv[4]])
    elif(len(sys.argv) == 6):
        context = " ".join([sys.argv[2], sys.argv[3], sys.argv[4], sys.argv[5]])

    #print len(sys.argv)

    #server socket information
    hostName = sys.argv[1]
    hostPort = int(sys.argv[2]);
    #assign client socket, use my ip
    clientSocket = socket(AF_INET, SOCK_STREAM)
    #separate socket for receiving data
    dataSocket = socket(AF_INET, SOCK_STREAM)

    #close sockets safely when we ctrl+c
    def handler(sig, frame):
        clientSocket.close()
    #setup for signal interrupt to close socket
    signal.signal(signal.SIGINT, handler)

    if sys.argv[3] == '-1':
        #print context + " hi "
        #send on the dataSocket so it will be received here
        clientSocket.connect((hostName, hostPort))
        sendCommand(clientSocket, context)

        while 1:
            res = clientSocket.recv(500)
            if not res:
                break
            print res


    elif sys.argv[3] == '-g':
        #connect
        clientSocket.connect((hostName, hostPort))
        #dataSocket.connect((hostName, hostPort))
        sendCommand(clientSocket, context)

        file = open(sys.argv[4] + ".received", "a")
        while 1:
            res = clientSocket.recv(500)

            if not res:
                break

            file.write(res)

            file.close



if __name__ == "__main__":
    main()
