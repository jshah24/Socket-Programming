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
#define MAX 200
#define WAIT 10
#define BUFFERLENGTH 1024
using namespace std;


/*---------Took reference from Beej's Guide to Network Programming-----*/ 
long int file_lookup(char* filename)
{
	char final_filename[100];
	cout<<"INFO: Server Checking if file exits inside the data folder"<<endl;
	FILE *fread;
	long int file_bytesize=0;
	strcpy(final_filename,".//data//");
    strcat(final_filename,filename);
	fread=fopen(final_filename,"r");
	if(fread==NULL)
	  {
		cout<<"Error:Could not open the file"<<endl;
     }
	else
     {	
		fseek(fread,0,SEEK_END);		
		file_bytesize=ftell(fread);		
     }
	return(file_bytesize);
}

void *udp_creation(void *serverportnumber_udp1)
{	
	/*-------------------------Declaration for UDP soket-----------------------*/
	int server_udp_socket,bindserver_udp,recv_filename, udpport;
	struct sockaddr_in server_udp, client_udp;	
	char b[1000];
	char receive_filename_buffer[BUFFERLENGTH];
	char send_bytesize_buffer[20];
	long int bytesize;
	int bytesize_send;
	
	/*---------------Creating server UDP socket----------------*/
	server_udp_socket=socket(AF_INET,SOCK_DGRAM,0);
	if(server_udp_socket<0)
	  {
			cout<<"Error:Server failed to create UDP socket"<<endl;
			exit(1);
	  }
	else 
     {
		cout<<"Server's UDP Socket created successfully"<<endl;
	  }
	  
	/*--------------Server UDP socket address--------*/

	udpport= *(int*)serverportnumber_udp1;	
	memset(&server_udp,0,sizeof(server_udp));
	server_udp.sin_family=AF_INET;
	server_udp.sin_addr.s_addr=htonl(INADDR_ANY);
	server_udp.sin_port=udpport;
	
	/*--------------Binding server UDP socket---------*/
	socklen_t serverudp = sizeof(server_udp);
	bindserver_udp=bind(server_udp_socket,(struct sockaddr *)&server_udp, serverudp);

	if(bindserver_udp<0)
	  {
		cout<<"Error:Server Binding failed for UDP"<<endl;
		exit(1);
	  }
	else
	  {
		cout<<"INFO: Server Binding is successful for UDP"<<endl;		
	  }
	  
	/*--------------Client UDP socket address--------*/
	memset(&client_udp,0,sizeof(client_udp));
	 
	/*----------------Server is Receiving filename from client over UDP---------------*/
	socklen_t clientlen = sizeof(client_udp);
	while(1){	    
		recv_filename = recvfrom(server_udp_socket,receive_filename_buffer,BUFFERLENGTH,0,(struct sockaddr *)&client_udp,&clientlen);		
		if(recv_filename<0)
		  {
			cout<<"Error:Server failed to receive the file name from client"<<endl;
			exit(1);
		  }
		else
		{			
			receive_filename_buffer[recv_filename] = '\0';
			bytesize = file_lookup(receive_filename_buffer);		
			sprintf(send_bytesize_buffer,"%d",bytesize);	//Converting bytesize from int to string for 'sendto' function
			bytesize_send=sendto(server_udp_socket,send_bytesize_buffer,BUFFERLENGTH,0,(struct sockaddr *)&client_udp,sizeof(client_udp));
			if(bytesize_send<0)
			{
				cout<<"Error: Server cannot send bytesize to the client over UDP"<<endl;				
				exit(1);
			}
			else
			{
				cout<<"INFO: Server is successful in sending the file byte size to the client. The file bytesize is: "<<send_bytesize_buffer<<endl;
			}		 
		}
	}
	/*-----------------------Closing client's UDP socket---------------------------------------------------*/
	close(server_udp_socket);	
}

void *file_transfer(void *new_server_tcp_socket){
	/*---------------------------Declaration---------------------------*/
	int new_server_tcp_sock = *(int*) new_server_tcp_socket;
    char recieve_chunkdetail_buffer[BUFFERLENGTH];
	char filename[50];
	char final_filename[100];
	int chunk_number, bytes_to_transfer=499, starting_offset;
	FILE *ffile;
	int send_filechunk_length;
	char newbuff[1000];
	/*-----------Server Recieving the chunk number from the client----------*/
	if(recv(new_server_tcp_sock,&chunk_number,sizeof(chunk_number),0)<0)
	{
		cout<<"Error: Error at server in receiving the details from the client, for file transfer"<<endl;
		exit(1);
	}
	
	/*-----------Server Recieving the file name from the client----------*/
	if(recv(new_server_tcp_sock,filename,sizeof(filename),0)<0)
	{
		cout<<"Error: Error at server in receiving the details from the client, for file transfer"<<endl;
		exit(1);
	}
	
	
		starting_offset = (chunk_number-1) * (bytes_to_transfer+1);
		cout<<"INFO: Server received the chunk "<<chunk_number<<" request from the client for 500 bytes in "<<filename<<" at offset "<<starting_offset<<endl;
		strcpy(final_filename,".//data//");
		strcat(final_filename,filename);
		ffile = fopen(final_filename, "r");
		
		fseek(ffile, starting_offset, SEEK_SET );
		fread(newbuff,sizeof(char), (bytes_to_transfer+1),ffile);
		rewind(ffile);
		if(send(new_server_tcp_sock,newbuff,sizeof(newbuff),0)<0){
			cout<<"Error on the server while sending the chunk data"<<endl;
		}
			
		fclose(ffile);
		close(new_server_tcp_sock);
	pthread_exit(NULL);
}

void *tcp_creation(void *serverportnumber_tcp)
{
	/*-------------------------Declaration for 	TCP soket-----------------------*/
	int server_tcp_socket,bindserver_tcp,new_server_tcp_socket, tcpport;
	struct sockaddr_in server_tcp, client_tcp;	
	pthread_t threadid_new_tcp;
	int client_tcp_length;
	
/*---------------Creating server TCP socket----------------*/
	server_tcp_socket=socket(AF_INET,SOCK_STREAM,0);
    if(server_tcp_socket<0)
	{
		cout<<"Error:Server failed to create TCP socket"<<endl;
			exit(1);
	}
	else
	{
		cout<<"Server's TCP Socket created successfully"<<endl;
	}
	
/*--------------Server TCP socket address--------*/
	tcpport= *(int*)serverportnumber_tcp;
	memset(&server_tcp,0,sizeof(server_tcp));
	server_tcp.sin_family=AF_INET;
	server_tcp.sin_addr.s_addr=htonl(INADDR_ANY);
	server_tcp.sin_port=tcpport;
	
	/*-----------Client TCP socket address---------*/
	memset(&client_tcp,0,sizeof(client_tcp));
	client_tcp_length = sizeof(client_tcp);
	
	/*--------------Binding server TCP socket---------*/
	socklen_t servertcp=sizeof(server_tcp);
	bindserver_tcp=bind(server_tcp_socket,(struct sockaddr *)&server_tcp,servertcp);
	if(bindserver_tcp<0)
	{
		cout<<"Error:Server Binding failed for TCP"<<endl;
		exit(1);
	}
	else
	  {
		cout<<"INFO: Server Binding is successful for TCP"<<endl;
		cout<<"Binding happening on: "<<server_tcp.sin_port<<endl;
	  }
/*--------------------------Server Listening ---------------------------*/

	listen(server_tcp_socket,WAIT);
	

/*-------------------------Server Accepts the incoming connection-----------*/
	while(1)
	{
		new_server_tcp_socket=accept(server_tcp_socket,(struct sockaddr*)&client_tcp,(socklen_t*)&client_tcp_length);
		if(new_server_tcp_socket<0)
		  {
			cout<<"Error:Server failed to accept the incoming client request"<<endl;
			continue;
		  }
		else
		  {
			cout<<"INFO: Server accepted the incoming request"<<endl;
		  }
		pthread_create(&threadid_new_tcp,NULL,file_transfer,(void*)&new_server_tcp_socket);
	}
}
int main(int argc,char **argv)
{
	int serverportnumber_udp,serverportnumber_tcp;
	pthread_t threadid_udp, threadid_tcp;
	
	if(argc!=3)
	  {
		cout<<"Error"<<endl;
		exit(1);
	  }
	else{
		cout<<"INFO: The UDP and TCP port numbers for the server are: "<<argv[1]<< " and " << argv[2]<<endl;			
	}
	serverportnumber_udp=atoi(argv[1]);
	serverportnumber_tcp=atoi(argv[2]);
	pthread_create(&threadid_udp,NULL,udp_creation,(void*) &serverportnumber_udp);
	pthread_create(&threadid_tcp,NULL,tcp_creation,(void*) &serverportnumber_tcp);
	pthread_join(threadid_udp,NULL);
	pthread_join(threadid_tcp, NULL);
	return 0;
}
