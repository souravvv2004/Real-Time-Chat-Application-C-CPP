#include <stdio.h>
#include <stdlib.h>
#include <string.h> // bzero function
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h> // Select , fd_set
#include <netinet/in.h> //ntohs , ntohl , htons , htonl 
#include <arpa/inet.h> // inet_aton , inet_ntoa


/**
 * @brief Client module for chat application
 * 
 * */



/**
 * @brief Custom Error module for printing error message through STD ERROR Channel 
 * @param msg : Message string containing error
 * */
void error(const char* msg)
{
    perror(msg);
    exit(1);
}


/**
 * @brief Main driver code for the function
 * @param argc number of argument pass while execution
 * @param argv charr array storing the passed arguments
 * 
 * 
 * */
int main(int argc, char* argv[])
{
   
	// If Argument is less than 3 , return 
	// arg 0 : exe file name , arg 1 : ip address of server , arg 2 : port number of server
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }


	// Calling ascii to integer for converting portno
	
    int portno = atoi(argv[2]);

    // create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("socket failed");

    // setup server address struct
    struct sockaddr_in serv_addr;
	memset(&serv_addr, 0 , sizeof(serv_addr));  // intializing with zero 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(portno); 

	// converting 1289/32 bit ip address string into network byte order
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0)
        error("invalid IP address");
	
		char name[50] = {0};
		
		printf("Enter Name: ");
		fgets(name,sizeof(name),stdin);
		
		int name_len = strlen(name);
		
		if(name_len>0 && name[name_len-1] == '\n')
		{
				name[name_len - 1]='\0';
				
		
		
		}

    // connecting  to server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        error("connect failed");



    printf("Connected to server.\n");
    printf("Type a message and press Enter to send.\n");

    fd_set readfd;
    
    
    char buffer[256];

    for (;;)
    {
        // Resetting all values and set sockfd and keyboard input
        
        FD_ZERO(&readfd);
        FD_SET(sockfd, &readfd);
        FD_SET(STDIN_FILENO, &readfd);

        int maxfd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;

        int activity = select(maxfd + 1, &readfd, NULL, NULL, NULL);
        if (activity < 0)
            error("select failed");

        // socket has data - server sent something
        if (FD_ISSET(sockfd, &readfd))
        {
            memset(buffer, 0 ,  sizeof(buffer));
            int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

            if (n < 0)
            {
                error("recv failed");
            }
            else if (n == 0)
            {
                printf("Server disconnected.\n");
                break;
            }
            else
            {
                printf("%s", buffer);
                if (buffer[strlen(buffer) - 1] != '\n')
                    printf("\n");
            }
        }

        // stdin has data - user typed something
        if (FD_ISSET(STDIN_FILENO, &readfd))
        {
            memset(buffer,0, sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);
			
			
			char sendBuffer[310] = {0};
            snprintf(sendBuffer, sizeof(sendBuffer), "%s: %s", name, buffer);

            int n = send(sockfd, sendBuffer, strlen(sendBuffer), 0);
            if (n < 0)
                error("send failed");
        }
    }

    // cleanup
    close(sockfd);

    return 0;
}
