#!/usr/bin/python
# Andrew Brown
# cs372-400 Project 2

import sys
from socket import *
import signal
import os

MSGLEN = 100

def handler(sig, frame):
    clientSocket.close()

def sendCommand(socket, msg):
    print "sending the goods"
    socket.send(msg)

def main():
    if(len(sys.argv) < 4):
        print "Usage: ./ftclient.py <hostname> <port number> <arg>"
        sys.exit(0)

    for arg in sys.argv:
        print arg

    hostName = sys.argv[1]
    hostPort = int(sys.argv[2]);
    print hostPort
    #assign client socket, use my ip
    clientSocket = socket(AF_INET, SOCK_STREAM)
    #separate socket for receiving data
    dataSocket = socket(AF_INET, SOCK_STREAM)
    context = sys.argv[3]

    #setup for signal interrupt to close socket
    signal.signal(signal.SIGINT, handler)

    clientSocket.connect((hostName, hostPort))

    sendCommand(clientSocket, context)


if __name__ == "__main__":
    main()
