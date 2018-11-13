#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define MAX 100
#define MAXLINE 8192

void sig_chld(int signo);

int main(int argc, char *argv[])
{
	int listenfd, connectfd, n, i, j;
	socklen_t clientlen;
	pid_t pid;
	struct sockaddr_in serverInfo, clientInfo;
	char tmp[MAXLINE], clen[MAX],  message[MAXLINE], buffer[MAXLINE], c, htmlfile[MAX];
	FILE *fp;
	int image[100000];	

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&serverInfo, sizeof(serverInfo));
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverInfo.sin_port = htons(7777);

	bind(listenfd, (struct sockaddr *)&serverInfo, sizeof(serverInfo));
	
	listen(listenfd, 1024);
	
	signal(SIGCHLD, sig_chld);
	
	
	for( ; ; )
	{
		clientlen = sizeof(clientInfo);
		connectfd = accept(listenfd, (struct sockaddr *)&clientInfo, &clientlen);
		
		/*initialization*/
		j = n = 0;
		bzero(&tmp, sizeof(tmp));
		bzero(&buffer, sizeof(buffer));			
		bzero(&htmlfile, sizeof(htmlfile));		

		
		/*fork*/
		if((pid = fork()) < 0)
		{
			fprintf(stderr, "Fork Error!");
		}
		else if(pid == 0) //child
        	{        
			close(listenfd);
			
			/*request message of browser */
			recv(connectfd, buffer, MAXLINE, 0);		
			printf("\n-----Request message-----\n%s", buffer);
			
			for(i=0; i<strlen(buffer); i++) //get which html file should be open 
			{
				if(buffer[i] == '/')
				{	
					if(buffer[i+1] == ' ')
					{
						strcpy(htmlfile, "welcome.html");
						break;
					}
					
					i++;
					
					while(buffer[i] != ' ')
					{
						htmlfile[j++] = buffer[i++];
					}					
					
					break;
				}
					
			}			
				
						
			printf("open html: %s\n", htmlfile);
			
			/*jpg file (not complete, only for HSIEH.jpg)*/
			if(strcmp(htmlfile, "HSIEH.jpg") == 0)
			{
				int fd;
				fd = open(htmlfile, O_RDONLY);
					
				sprintf(message, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\n\r\n");	
				read(fd, image, 100000);
				
				send(connectfd, message, strlen(message), 0);
				write(connectfd, image, sizeof(image));
				
				exit(0);
			} 


			/*html file*/
			sprintf(message, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ");	
			
			fp = fopen(htmlfile, "r");
			if(!fp)
			{
				fprintf(stderr, "ERROR! Can not find this html!\n");
				exit(1);
			}
			
				
			while((c = fgetc(fp)) != EOF)
			{
				tmp[n++] = c;
			}
			
			fclose(fp);				

			sprintf(clen, "%d\r\n\r\n", (int)strlen(tmp));
			strcat(message, clen);
			
			strcat(message, tmp);
			
			/*for(n=0; n<strlen(message); n++)
				printf("%c", message[n]); */
			
			send(connectfd, message, strlen(message), 0);
			
			printf("\nSend message successful!\n");
			

			exit(0); //important
		}
		else //parent
		{
			waitpid(-1, NULL, WNOHANG);
			close(connectfd);
		}
	}	
}

void sig_chld(int signo)
{
	pid_t pid;
	int stat;
	
	pid = waitpid(0, &stat, WNOHANG);

	return;
}
