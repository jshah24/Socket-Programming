#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#define SERVER_UDPPORT 3333
#define BUFFERLENGTH 1024
#define BYTES_TO_TRANSFER 500
#define SERVER_TCPPORT 4444

using namespace std;
int main(int argc,char **argv)
{	
	char command[20];
	int switchtype;
	
	/*-------------------------Declaration for UDP soket-----------------------*/
		  
	struct sockaddr_in server_udpaddress;
	int client_udp_socket;
	char filename[50];
	char receive_bytesize_buffer[BUFFERLENGTH]="";
	int recieve_bytesize;
	int file_bytesize;
	
	
		/*--------------Declaration of TCP---------------------*/
	int client_tcp_socket;
	int client_tcp_sockets_for_chunks[100];
	int file_desc_max;
	fd_set master;    // master file descriptor list
	fd_set read_fds;  // temp file descriptor list for select()
	FD_ZERO(&master);    // clear the master and temp sets
	FD_ZERO(&read_fds);
	struct timeval timeout;
	int select_output; 
	FILE *file_download;   
	char data_from_server[500];
	
	if(argc!=3)
		  {
			 cout<<"ERROR:Wrong arguments passed"<<endl;
			 exit(1);
		  
		  }
	do{
	cout<<"Please enter the command"<<endl;
	cin>>command;
	
	if(strcmp(command,"lookup")==0)
			switchtype = 1;
		if(strcmp(command,"download")==0)
			switchtype=2;
		if(strcmp(command,"quit")==0)		
			switchtype=3;
	switch(switchtype){
		case 1:
		{
			/*-----------------------------Server's UDP port address------------------------------*/	
			memset(&server_udpaddress,0,sizeof(server_udpaddress));
			server_udpaddress.sin_family=AF_INET;
			server_udpaddress.sin_addr.s_addr = inet_addr("127.0.0.1");
			server_udpaddress.sin_port=SERVER_UDPPORT;
			
			/*---------------------Creating Client UDP socket--------------------------------*/
		 
			 client_udp_socket=socket(AF_INET,SOCK_DGRAM,0);
				if(client_udp_socket<0)
				  {
						cout<<"Error:Client failed to create UDP socket"<<endl;
						exit(1);
				  }
				else
				{ 
					cout<<"Client's UDP Socket created successfully"<<endl;
				}

			/*-------Sending filename to Server using client UDP socket on the servr address mentioned above-----*/
			cout<<"Hello Client: Please enter the filename whose bytesize is to be found out"<<endl;
			cin>>filename;
			socklen_t serverlen = sizeof(server_udpaddress);
			if (sendto(client_udp_socket, filename, BUFFERLENGTH, 0, (const struct sockaddr *)&server_udpaddress, serverlen)<0)
			{
				cout<<"ERROR:An error has occurred while the client was sending file name to the server"<<endl;
				exit(1);
			}
			else
			{		
				cout<<"INFO: The file name has been successfully sent from the client to the server"<<endl;
			}
			
			/*----------------------Client is receiving bytesize of the file from server---------------------------------------------*/
			
			if(recieve_bytesize = recvfrom(client_udp_socket,receive_bytesize_buffer,BUFFERLENGTH,0,(struct sockaddr *)&server_udpaddress,&serverlen)<0)
			{
				cout<<"ERROR:An error has occurred while the client was receiving byte size from the server"<<endl;
			}
			else
			{		
				file_bytesize = atoi(receive_bytesize_buffer);	
				cout<<"INFO: The file byte size received by the client is: "<<file_bytesize<<endl;
			}
			
			/*-----------------------Closing client's UDP socket---------------------------------------------------*/
			close(client_udp_socket);
			break;
		}
	

		case 2:
		{
			if(file_bytesize != 0){
				int i, number_of_chunks, starting_offset=0;
				/*----calculating the number of chunks----*/
				number_of_chunks = file_bytesize/BYTES_TO_TRANSFER;
				number_of_chunks = number_of_chunks + ((file_bytesize%BYTES_TO_TRANSFER>0)?1:0);
				
				/*--------------Creating 'k' TCP sockets at the client-----------*/
				for(i=0; i<number_of_chunks; i++){
					struct sockaddr_in server_tcpaddress;
					/*---------Assigning server TCP address---------*/
					memset(&server_tcpaddress,0,sizeof(server_tcpaddress));
					server_tcpaddress.sin_family=AF_INET;
					server_tcpaddress.sin_addr.s_addr = inet_addr("127.0.0.1");
					server_tcpaddress.sin_port=SERVER_TCPPORT;
					
					client_tcp_sockets_for_chunks[i] = socket(AF_INET,SOCK_STREAM,0);
					if(client_tcp_sockets_for_chunks[i]<0){
						cout<<"Error: TCP socket creation not successful at the client"<<endl;
					}
					cout<<"TCP socket successfully created at the client"<<endl;
					
					if(connect(client_tcp_sockets_for_chunks[i] , (struct sockaddr *)&server_tcpaddress , sizeof(server_tcpaddress)) < 0)
					{
						cout<<"Error: Connection error at client"<<endl;
						exit(1);
					}
					int val = i+1;
					if( send(client_tcp_sockets_for_chunks[i] , &val , sizeof(i) , 0) < 0)
					{
						cout<<"Error: Error at client in sending the chunk number"<<endl;
						exit(1);
					}
					
					if( send(client_tcp_sockets_for_chunks[i] , filename , sizeof(filename) , 0) < 0)
					{
						cout<<"Error: Error at client in sending the file name"<<endl;
					}
					
					cout<<"INFO: Chunk request "<<i+1<<" is sent "<<filename<<" "<<(i*500)<<" "<<BYTES_TO_TRANSFER<<endl;
					sleep(1);
				}
				
				FD_SET(client_tcp_sockets_for_chunks[number_of_chunks-1], &master);// keep track of the biggest file descriptor
				file_desc_max = client_tcp_sockets_for_chunks[number_of_chunks-1]; // so far, it's this one
				for(i=0;i<number_of_chunks;i++)  
				{
						FD_SET(client_tcp_sockets_for_chunks[i], &read_fds);
				}
				
				timeout.tv_sec  = 3 * 60;
				timeout.tv_usec = 0;
				if (select_output=select(file_desc_max+1, &read_fds, NULL, NULL, &timeout) < 0) 
				{
					cout<<"Error: Error on client while doing select"<<endl;
					exit(1);
				}
				
				file_download = fopen(filename, "w");
				if(file_download == NULL)
				{
					cout<<"File does not exists"<<endl;
					exit(1);
				}
				
				for(i=0;i<number_of_chunks;i++)  
				{
					if(FD_ISSET(client_tcp_sockets_for_chunks[i], &read_fds))
					{
						if(recv(client_tcp_sockets_for_chunks[i], data_from_server, sizeof(data_from_server), 0)<0)
						{
							cout<<"Error: Error on client while recieveing chunk data from the server"<<endl;
						}
						fputs(data_from_server, file_download);
					}
				}
			}
			else{
				cout<<"Error: Cannot download as the file does not exists"<<endl;
			}
			break;
		}
	}
	}while(switchtype != 3);
}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	