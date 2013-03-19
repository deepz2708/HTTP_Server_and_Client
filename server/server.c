/*****************************************************
*		Name: Deepanshu Goyal						 *
*		Roll No.: 10010118							 *
*		File Name: Server.c							 *
*		Networks Assignment 2						 *
******************************************************/
#include <stdio.h>							//Include Libraries
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include<signal.h>
#include <sys/socket.h>
#include <sys/types.h> 



void error(const char *msg)					//prints the error message and exit
{
	perror(msg);
	exit(1);
}

int validate_input_string(int argc);		// checks if given input string is in correct format or not
int get_func(int, char*);					// get request from client
int put_func(int, char*);					// put data from client in a file

/************start of main function******************/
int main(int argc, char *argv[])									// start of main function
{
	struct sockaddr_in serv_addr, cli_addr;							// variable declarations
	socklen_t clilen;
	char buffer[512], method[4];
	int sockfd, newsockfd, portno, valid_string, invalid, n;

	valid_string = validate_input_string(argc);						// check to see if input string is valid or not
	if(valid_string == 0)											// if not valid then exit
		exit(1);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);						// check connecting server
	if (sockfd < 0) 												// if not then error
		error("OPENING SOCKET ERROR");

	int reuseYES=1;													// Code to ensure binding port is free next time
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reuseYES,sizeof(int)) == -1) {
		perror("setsockopt: Could not clear port that was being used earlier.");
		exit(1);
	}

	memset((char *) &serv_addr, 0, sizeof(serv_addr));					// initialize the server_addr to 0
	portno = atoi(argv[1]);											// port no changes into integer

	serv_addr.sin_family = AF_INET;									// declare the addresses
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) // bind the socket and if binding is < 0 then error
		error("BINDING ERROR");

	listen(sockfd,5);												// listen to the client
	while( 1 )														// while true i.e till the client doesnt terminate
	{
		int temp, len;
		clilen = sizeof(cli_addr);									// define the client address length
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);	// accept the client connection

		if (newsockfd < 0) 											// if < 0 then error
			error("ACCEPTING ERROR");

		memset(buffer,0,512);											// initialize the buffer to 0

		int end = 0,si=0;											// declare variables
		char inbuf;
		memset(&inbuf,0,1);											// initialize the inbuf to 0
		while(end!=4)												// while the end int is not 4 i.e. EOF \r\n\r\n
		{
			n = recv(newsockfd,&inbuf,1,0);							// read the socket recieve the character
			if (n < 0) 												// if < 0 then error
				error("READING SOCKET ERROR");
			buffer[si] = inbuf;si++;								// increase the buffer position by 1
			switch(end)												// switch case is used to check the EOF
			{
				case 0:												// for first \r
						if(inbuf=='\r')
						end=1;
						break;
				case 1:												// for first \n after \r
						if(inbuf=='\n')
							end=2;
						else
							end=0;
						break;
				case 2: 											// for \r\n\r
						if(inbuf=='\r')
							end=3;
						else
							end=0;
						break;
				case 3:												// for \r\n\r\n
						if(inbuf=='\n')
							end=4;
						else
							end=0;
						break;
			}
			memset(&inbuf,0,1);										// initialize the inbuf to 0
		}

		int pos=0,i=0;												// declare the variables
		char filename[250],version[10],hostname[250];
		memset(method,0,4);											// initialize all array to 0
		memset(filename,0,250);
		memset(version,0,10);
		memset(hostname,0,250);
		invalid = 0;												// initialize invalid to 0
		while(buffer[pos]==' ' || buffer[pos]=='\t') pos++;			// initial spaces and tab are counted and increase the position in buffer
		if(buffer[pos]=='\0') 										// if terminate there then it is bad request
		{
			printf("BAD REQUEST\n");
			invalid=1;												// if bad request then invalid to 1
		}
		else														// if not terminate then increase the buffer
		{
			method[0]=buffer[pos];pos++;
			method[1]=buffer[pos];pos++;
			method[2]=buffer[pos];pos++;

			while(buffer[pos]==' ' || buffer[pos]=='\t') pos++;
			if(buffer[pos]=='\0') 
			{
				printf("BAD REQUEST\n");
				invalid=1;
			}
			else
			{
				i=0;
				pos++;
				while(buffer[pos]!=' ' && buffer[pos]!='\t')
				{
					filename[i]=buffer[pos];
					pos++;
					i++;
				}
				while(buffer[pos]==' ' || buffer[pos]=='\t') pos++;
				if(buffer[pos]=='\0') 
				{
					printf("BAD REQUEST\n");
					invalid=1;
				}
				else
				{
					i=0;
					while(buffer[pos]!=' ' && buffer[pos]!='\t' && buffer[pos]!='\r' && buffer[pos]!='\n')
					{
						version[i]=buffer[pos];
						pos++;
						i++;
					}
					while(buffer[pos]==' ' || buffer[pos]=='\t') pos++;
					if(buffer[pos]=='\r' && buffer[pos+1]=='\n')
					{
						pos++;pos++;
						while(buffer[pos]==' ' || buffer[pos]=='\t') pos++;
						if(buffer[pos]=='\0') 
						{
							printf("BAD REQUEST\n");
							invalid=1;
						}
						else
						{
							if(buffer[pos]=='h'&&buffer[++pos]=='o'&&buffer[++pos]=='s'&&buffer[++pos]=='t'&&buffer[++pos]==':')
							{
								pos++;
								while(buffer[pos]==' ' || buffer[pos]=='\t') pos++;
								i=0;
								while(buffer[pos]!=' ' && buffer[pos]!='\t'&& buffer[pos]!='\r' && buffer[pos]!='\n')
								{
									hostname[i]=buffer[pos];
									pos++;
									i++;
								}
								if(buffer[pos]!='\r'||buffer[pos+1]!='\n'||buffer[pos+2]!='\r'||buffer[pos+3]!='\n')
								{
									printf("BAD REQUEST\n");
									invalid=1;
								}
							}
							else
							{
								printf("BAD REQUEST\n");
								invalid=1;
							}
						}
					}
					else
					{
						printf("BAD REQUEST\n");
						invalid=1;
					}
				}
			}
		}
		if(invalid==1)														// if invalid string then check the connection
		{
			n = send(newsockfd,"400 Bad Request",strlen("400 Bad Request"),0);	// send the bad request
			if (n < 0) 														// if request is not sending then writing socket error
				error("WRITING SOCKET ERROR");
		}
		else																// if not invalid then go in the put or get function
		{
		printf("method --> %s\n",method);					// print the current status of server i.e. what method, filename, version or hostname is used
		printf("filename --> %s\n",filename);
		printf("version --> %s\n",version);
		printf("hostname --> %s\n",hostname);
			if(!strcmp(method,"PUT"))										// if in method put is there then call put function 
			{
				put_func(newsockfd, filename);								// calling put function

			}
			else if(!strcmp(method,"GET"))									// if in method get is there then call get function
			{
				get_func(newsockfd, filename);								// calling get function
			}
			else
			{
				n = send(newsockfd,"400 Bad Request",strlen("400 Bad Request"),0); // check the connection
				if (n < 0) 													// if error then print error and exit
					error("WRITING SOCKET ERROR");
			}
		}
		close(newsockfd);													// close the connection
		printf("\n");
	}
	close(sockfd);
	return 0; 
}
/************end of main function******************/


/************start of put function******************/
int put_func(int newsockfd, char *filename)										// put function
{
	char input_buffer;															// varible declaration
	int reading_status;
	
	reading_status = recv(newsockfd, &input_buffer, 1,0);						// check the reading status
	if (reading_status < 0)														// if error then print error
		error("READING SOCKET ERROR");

	if( input_buffer == '1' )													// if something is there in input buffer then
	{
		FILE *fp;																// file pointer
		fp = fopen(filename, "w");												// append mode to write the data which is put by client
		char character_input;													// declare character input
		memset(&character_input , 0, 1);										// initialize caracter input to 0
		reading_status = recv(newsockfd, &character_input , 1,0);				// check the recieving data
		do																		// do while character input is not EOF
		{
			
				memset(&character_input , 0, 1);										// initialize character input to 0
				reading_status = recv(newsockfd, &character_input , 1,0);			// check the recieving data
				if (reading_status < 0)												// if error then print error
					error("READING SOCKET ERROR");
				if( character_input != EOF )										// if not EOF then
				{		
					printf("%c", character_input);									// print the character
					fputc(character_input, fp);										// print in file too
				}
			
		}while(character_input != EOF);
		//fputc(EOF, fp);															// put the eof
		fclose(fp);																// close the connection

		reading_status = send(newsockfd,"200 OK File Created\n",strlen("200 OK File Created\n"),0); // send the data to confirm the file receive
		if (reading_status < 0) 												// if error then print error
			error("WRITING SOCKET ERROR");	

	}
	else if( input_buffer == '0' )												// if error then print error
	{
		reading_status = send(newsockfd,"File Not Created\n",strlen("File Not Created\n"),0);	// send the status that file can't be created
		if (reading_status < 0) 												// print the error if any
			error("WRITING SOCKET ERROR");	

	}
}
/************end of put function******************/



/************start of get function******************/
int get_func(int newsockfd, char *filename)									// get function
{
	int writing_status;														// to get writing status
	FILE *fp;																// file pointer to open the file
	fp = fopen(filename,"r");												// open file in read mode
	if(fp)																	// if succeessful 
	{
		writing_status = send(newsockfd,"200 OK\n",strlen("200 OK\n"),0);	// send the data to client
		if (writing_status < 0) 											// if error then print error
			error("WRITING SOCKET ERROR");
		char character_input;												// character_input declaration

		character_input = fgetc(fp);										// read character from file
		while( character_input != EOF )										// while character_input is not EOF the read character by character
		{
			writing_status = send(newsockfd, &character_input, 1,0);		// send the data
			if (writing_status < 0) 										// if error then print error
				error("WRITING SOCKET ERROR");
			character_input = fgetc(fp);									// read next character

		}
		fclose(fp);															// close the file
	}
	else																	// else (for not successful open the file)
	{
		writing_status = send(newsockfd,"404 NOT FOUND\n",strlen("404 NOT FOUND\n"),0);	//send the data
		if (writing_status < 0) 											// if error then print error
			error("WRITING SOCKET ERROR");
	}
	return 1;
}
/************end of get function******************/



/************start of validate_input_query function******************/
int validate_input_string(int argc)										//validate input string function
{
	if (argc < 2) 														// if no. of argument is < 2 then command syntax is wrong
	{
		fprintf(stderr,"ERROR, NO PORT IS PROVIDED\n");
		return 0;
	}
	return 1;

}
/************end of validate_input_query function******************/
