#!/usr/bin/python
# Andrew Brown
# cs372-400 Project 2

import sys
from socket import *
import signal
import os

MSGLEN = 100

if(len(sys.argv) < 4):
    print "Usage: ./ftclient.py <hostname> <port number> <arg>"
    sys.exit(0)

for arg in sys.argv:
    print arg

def handler(sig, frame):
    clientSocket.close()
    
hostName = sys.argv[1]
hostPort = int(sys.argv[2]);
print hostPort
#assign client socket, use my ip
clientSocket = socket(AF_INET, SOCK_STREAM)
#separate socket for receiving data
dataSocket = socket(AF_INET, SOCK_STREAM)

#setup for signal interrupt to close socket
signal.signal(signal.SIGINT, handler)

clientSocket.connect((hostName, hostPort))