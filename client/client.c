/*****************************************************
*		Name: Deepanshu Goyal						 *
*		Roll No.: 10010118							 *
*		File Name: Client.c							 *
*		Networks Assignment 2						 *
******************************************************/
#include <stdio.h>									//Include Libraries
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>

void error_msg(const char *msg)						//prints the error message and exit
{
	perror(msg);
	exit(0);
}

int put_func(int ,char* );							// data put on server
int get_func(int ,char* );							// request from server
int validate_input_query(int argc, char *argv);		// checks if given input string is in correct format or not

/************start of main function******************/
int main(int argc, char *argv[])								// start of main function
{	
	int sockfd, portno,valid_string, write_to_socket, size;		// variable declarations
	struct sockaddr_in serv_addr;								// inbuilt structure declaration of socket programming
	struct hostent *server;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	
	valid_string = validate_input_query(argc, argv[3]);			// check if input string is valid or not
	if( valid_string == 0)										// if not then exit
		exit(1);
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; 								// use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) //get the address of the host from the host name using getaddrinfo
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}

	portno = atoi(argv[2]);										// convert the port no into integer
		
	
	for(p = servinfo; p != NULL; p = p->ai_next) 				// servinfo now points to a linked list of 1 or more struct addrinfos
	{															// loop through all the results and connect to the first we can

		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) //allocate socket for the request
		{
			perror("socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) 		//connect to the socket
		{
			close(sockfd);
			perror("connect");
			continue;
		}
		size = 40 + strlen(argv[1]) + strlen(argv[4]) + strlen(argv[3]);// take the size of sending request
		char request[size];												// declare the request
		memset(request,0, size);										// initialize it to 0
		strcat(request,argv[3]);										// concatenate the various string to form a proper format of request 
		strcat(request," /");
		strcat(request,argv[4]);
		strcat(request," HTTP/1.1\r\nhost: ");
		strcat(request,argv[1]);
		strcat(request,"\r\n\r\n");
		printf("%s\n",request);											// print the request function
		write_to_socket = send(sockfd,request,strlen(request),0);		// send the request
		if (write_to_socket == -1) 										// if error then print error
			error_msg("SOCKET WRITING ERROR");
		
		
		if(!strcmp(argv[3],"PUT"))										// if put is used the use put function
		{
			put_func(sockfd, argv[4]);
		}
		else															// else use get function
		{
			get_func(sockfd, argv[4]);
		}
		close(sockfd);													//close the socket
		printf("\n");

		break; 															// if we get here, we must have connected successfully
	}
	if (p == NULL) 
	{
		fprintf(stderr, "failed to connect\n");							// looped off the end of the list with no connection
		exit(2);
	}

	freeaddrinfo(servinfo); 											// free the linked list

	return 0;
}
/************end of main function******************/


/************start of put function******************/
int put_func(int sockfd, char *arg)										// put function: file is to be put onto the server
{
	char input_buffer[256];												// character array to put the data into chunks
	int writing_status;													// writing status at the server
	memset(input_buffer,0,256);											// initialize the buffer to 0
	FILE *fp;															// file pointer which is to be send
	fp = fopen(arg,"r");												// open file in reading mode
	if(fp)																// if successful then
	{
		writing_status = send(sockfd, "1\0", 2,0);						// check the connection
		if (writing_status < 0) 										// if error print error
			error_msg("WRITING SOCKET ERROR");

		char character_input ;											// read character
		character_input = '\0';	
		do																// read till eof
		{
			
				character_input = fgetc(fp);								// read the character
				writing_status = send(sockfd, &character_input, 1,0);		// send it to server
				if (writing_status < 0) 									// if error print error
					error_msg("WRITING SOCKET ERROR");
			
		}while(character_input != EOF);
		fclose(fp);
	}
	else																// if not successful to open the file
	{	
		writing_status = send(sockfd, "0\0", 2,0);						// check status
		if (writing_status < 0) 										// if error print error
			error_msg("WRITING SOCKET ERROR");
		printf("Unable to open file\n");
	}

	writing_status = recv(sockfd,input_buffer,255,0);					// now recieve the status
	if (writing_status < 0)												// actually a reading status
		error_msg("READING SOCKET ERROR");						
	printf("%s\n",input_buffer);										// print what is being uploaded
	return 1;
}
/************end of put function******************/


/************start of get function******************/
int get_func(int sockfd, char* filename)							// fetch data from server function
{
	int reading_status;												// int to get reading_status
	char input_buffer;												// char to input buffer
	memset(&input_buffer,0,1);										// initialize buffer to zero
	FILE *fp;
	fp = fopen(filename, "w");
	reading_status = recv(sockfd,&input_buffer, 1,0);				// get data from server
	if (reading_status < 0)											// if error then print
		error_msg("READING SOCKET ERROR");
	while(reading_status!=0)										// while reading status != 0 means end of file
	{
		printf("%c",input_buffer);									// print the character
		fprintf(fp,"%c",input_buffer);
		reading_status = recv(sockfd,&input_buffer, 1,0);			// fetch another one
		if (reading_status < 0)										// if error then print
			error_msg("READING SOCKET ERROR");
	}
	fclose(fp);
	return 1;
}
/************end of get function******************/


/************start of validate_input_query function******************/
int validate_input_query(int argc, char *argv)						//validate input string
{
	if (argc < 5) 													// if no. of argument is < 5 then command syntax is wrong  
	{
		fprintf(stderr,"CORRECT COMMAND: ./client hostname port GET/PUT filename\n");	// print sample command syntax
		return 0;
	}

	if( strcmp( argv, "GET") && strcmp( argv, "PUT") ) 				// if get and put is missing
	{
		fprintf(stderr, "WRONG SYNTAX: GET/PUT SHOULD BE USED\n");	// print to put get & put
		return 0;
	}
	return 1;
	
}
/************end of validate_input_query function******************/
