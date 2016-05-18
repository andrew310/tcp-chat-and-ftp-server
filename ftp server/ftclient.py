#!/usr/bin/python
# Andrew Brown
# cs372-400 Project 2

import sys
from socket import *

MSGLEN = 100

if(len(sys.argv) < 4):
    print "Usage: ./ftclient.py <hostname> <port number> <arg>"
    sys.exit(0)
    
    
hostName = sys.argv[1]
hostPort = int(sys.argv[2]);
#assign client socket, use my ip
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName, serverPort))

