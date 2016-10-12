# Socket-Programming
File Transfer using TCP/UPD

OVERVIEW
The application consists of two parts (i) Server Part and (ii) Client Part running possibly on different machines. Basically, the server hosts a number of files that are located in some folder called "data". A client can communicate with the server and request the server transfer the file to the client (i.e., download file). However, instead of receiving the file in whole, the client requests chunks of the file in parallel with multiple threads.

Running Instructions:

Always make sure the server is up and running.

Server Side:
1. In order to compile the file, run the following command:
	g++ server.cpp -lpthread
2. To run the file, use the following commmand:
	./a.out 3333 4444

Client Side:
1. In order to compile the file, run the following command:
	g++ client.cpp -o client
2. To run the file, use the following commmand:
	./client 3333 4444
3. User will be asked to enter a command(lookup or download or quit)
4. If user enters 'lookup' command, the user will be prompted to enter the filename
5. User enters the filename
6. User will be able to see the byte size of the file sent by the server and '0' if the file does not exist
7. User will again be prompted for the command
8. If user enters 'download' command, the code will use the same filename used for the lookup  and the bytesize sent by the server
   At client side, user will be able to see the 'chunk requests' sent from the client to the server
9. If a lookup was not done before download, then an error will be thrown
10. The above will continue till the user enters a 'quit' command


