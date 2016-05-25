created by Andrew Brown
CS372 - 400
Spring 2016

INSTRUCTIONS

use this line to compile the server:

$ gcc -o ftserver ftserver.cpp


then run using:

$ ftserver 30050

Any port will do.

TO USE THE CLIENT:

No compilation needed. Place in any directory on any flip server. 

run using this:


$ python ftclient.py <hostname> <host port> <command> [filname] <data port>

EXAMPLE:

$ python ftclient.py flip1 30050 -1 30025

in this case you will receive the contents of the directory where ftserver is running.


ANOTHER EXAMPLE:

$ python ftclient.py flip1 30050 -g alice.txt 30025

in this case you will receive the file you specified.

