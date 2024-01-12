#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXCLEN 100
#define MAXLINE 8192


int main()
{
    int listenfd, connectfd, sockfd, n;
    int i, j, maxindex, maxfd, nready;
    int client[FD_SETSIZE];
    socklen_t clientlen;
    pid_t pid;
    fd_set readset, allset;
    struct sockaddr_in serverInfo, clientInfo;
    char tmp[MAXLINE], message[MAXLINE], buffer[MAXLINE], clen[MAXCLEN],
        htmlfile[MAXLINE], c;
    FILE *fp;
    int image[100000];


    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serverInfo, sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverInfo.sin_port = htons(7777);


    bind(listenfd, (struct sockaddr *) &serverInfo, sizeof(serverInfo));

    listen(listenfd, 1024);

    maxfd = listenfd;
    maxindex = -1;
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);  // add fd into set


    for (;;) {
        readset = allset;
        nready = select(maxfd + 1, &readset, NULL, NULL, NULL);

        /*initialization*/
        j = n = 0;
        bzero(&tmp, sizeof(tmp));
        bzero(&buffer, sizeof(buffer));
        bzero(&htmlfile, sizeof(htmlfile));

        // test whether fd in the set
        if (FD_ISSET(listenfd, &readset)) {
            clientlen = sizeof(clientInfo);
            connectfd =
                accept(listenfd, (struct sockaddr *) &clientInfo, &clientlen);
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = connectfd;
                    break;
                }
            }

            if (i == FD_SETSIZE)
                fprintf(stderr, "Too many clients!");

            FD_SET(connectfd, &allset);
            if (connectfd > maxfd)
                maxfd = connectfd;
            if (i > maxindex)
                maxindex = i;

            if (--nready <= 0)
                continue;
        }

        // check all clients
        for (i = 0; i <= maxindex; i++) {
            if ((sockfd = client[i]) < 0)
                continue;

            if (FD_ISSET(sockfd, &readset)) {
                /*request message of browser */
                recv(sockfd, buffer, MAXLINE, 0);
                printf("\n-----Request message-----\n%s", buffer);

                // get which html file should be open
                for (int k = 0; i < strlen(buffer); k++) {
                    if (buffer[k] == '/') {
                        if (buffer[k + 1] == ' ') {
                            strcpy(htmlfile, "welcome.html");
                            break;
                        }

                        k++;

                        int m = 0;
                        while (buffer[k] != ' ') {
                            htmlfile[m++] = buffer[k++];
                        }

                        break;
                    }
                }

                // printf("open html: %s\n", htmlfile);


                /*jpg file (not complete, only for HSIEH.jpg)*/
                /*if(strcmp(htmlfile, "HSIEH.jpg") == 0)
                {
                        int fd;
                        fd = open(htmlfile, O_RDONLY);

                        sprintf(message, "HTTP/1.1 200 OK\r\nContent-Type:
    image/jpeg\r\n\r\n"); read(fd, image, 100000);

                        send(sockfd, message, strlen(message), 0);
                        write(sockfd, image, sizeof(image));

        close(fd);

                }

    /*html file*/
                sprintf(message,
                        "HTTP/1.1 200 OK\r\nContent-Type: "
                        "text/html\r\nContent-Length: ");
                fp = fopen(htmlfile, "r");
                if (!fp) {
                    fprintf(stderr, "ERROR! Can not find this html!\n");
                    continue;
                }


                while ((c = fgetc(fp)) != EOF) {
                    tmp[n++] = c;
                }

                fclose(fp);

                sprintf(clen, "%d\r\n\r\n", (int) strlen(tmp));
                strcat(message, clen);
                strcat(message, tmp);

                send(sockfd, message, strlen(message), 0);

                printf("send successful!\n");

                close(sockfd);
                FD_CLR(sockfd, &allset);
                client[i] = -1;

                if (--nready <= 0)
                    break;
            }
        }
    }
}
