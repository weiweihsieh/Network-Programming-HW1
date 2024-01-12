#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX 1024

int sockfd;

static void *doit(void *arg);
int name_judgement(char username[]);  // judge the name is correct or not

int main()
{
    struct sockaddr_in serverInfo;
    char message[MAX], messages[MAX], username[MAX], file[MAX], filename[MAX],
        namesjudge[MAX];
    int numbytes;
    FILE *fp;
    // char accept[3];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serverInfo, sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverInfo.sin_port = htons(7777);

    connect(sockfd, (struct sockaddr *) &serverInfo, sizeof(serverInfo));

    /*receive welcome message*/
    char welcome_message[MAX] = {};
    recv(sockfd, welcome_message, sizeof(welcome_message), 0);
    printf("%s", welcome_message);

    /*enter name*/
    char name[30] = {};

    while (1) {
        printf("Enter your name: ");

        bzero(namesjudge, sizeof(namesjudge));

        fgets(name, MAX, stdin);
        name[strlen(name) - 1] = '\0';  // delete '\n'

        send(sockfd, name, strlen(name), 0);


        recv(sockfd, namesjudge, sizeof(namesjudge), 0);

        if (strcmp("correct", namesjudge) == 0)
            break;
        else {
            printf("%s", namesjudge);
        }
    }

    pthread_t tid;
    pthread_create(&tid, NULL, &doit, NULL);

    while (1) {
        bzero(message, sizeof(message));

        printf("%s: ", name);
        fgets(message, MAX, stdin);
        message[strlen(message) - 1] = '\0';  // delete '\n'

        send(sockfd, message, strlen(message), 0);

        if (strcmp("Q", message) == 0) {
            printf("You are offline!\nSee you!\n");
            return 0;
        } else if (strcmp("D", message) == 0) {
            pthread_cancel(tid);  // do not want to use thread to receive
                                  // message

            printf("Enter user name: ");
            fgets(username, MAX, stdin);
            username[strlen(username) - 1] = '\0';

            if (name_judgement(username) ==
                -1)  // judge the name is correct or not
            {
                pthread_create(&tid, NULL, &doit, name);
                continue;
            }

            printf("*****%s: ", name);
            fgets(messages, MAX, stdin);
            messages[strlen(messages) - 1] = '\0';  // delete '\n'

            send(sockfd, messages, strlen(messages), 0);

            pthread_create(&tid, NULL, &doit, NULL);
        }
        /*else if((strcmp("F", message) == 0))
        {
            pthread_cancel(tid); //do not want to use thread to receive message

            bzero(username, sizeof(username));
            bzero(filename, sizeof(filename));
            bzero(accept, sizeof(accept));

            printf("Enter user name: ");
                        fgets(username, MAX, stdin);
                        username[strlen(username)-1] = '\0';

            if(name_judgement(username) == -1) //judge the name is correct or
        not
                        {
                                pthread_create(&tid, NULL, &doit, name);
                                continue;
                        }


            printf("Enter file name: ");
            fgets(filename, MAX, stdin);
            filename[strlen(filename)-1] = '\0';

            fp = fopen(filename, "r");
            if(!fp)
            {
                printf("The file does not exist!\n");
                pthread_create(&tid, NULL, &doit, name);
                continue;
            }


            send(sockfd, filename, strlen(filename), 0);

            //whether receiver accepts the file or not
            recv(sockfd, accept, sizeof(accept), 0);
            if(strcmp("n", accept) == 0 || strcmp("N", accept) == 0)
            {
                printf("%s rejects your file!\n", username);
                pthread_create(&tid, NULL, &doit, name);
                continue;
            }
            else
            {
                printf("%s accepts!\n", username);
            }


            //send the file
            while(fgets(file, MAX, fp) != NULL)
            {
                send(sockfd, file, strlen(file), 0);
            }

            send(sockfd, "EOF", 3, 0);

            printf("Send file successful!\n");

            fclose(fp);

            pthread_create(&tid, NULL, &doit, name);
        }*/
    }
}

static void *doit(void *arg)
{
    char rmessage[MAX] = {};

    char sendername[MAX], accept[3];

    char file[MAX], filename[MAX];
    int numbytes;
    FILE *fp;

    while (1) {
        bzero(rmessage, sizeof(rmessage));
        // bzero(sendername, sizeof(sendername));
        // bzero(filename, sizeof(filename));
        // bzero(accept, sizeof(accept));

        recv(sockfd, rmessage, sizeof(rmessage), 0);

        /*if(strcmp("file", rmessage) == 0)
        {
            //recv(sockfd, sendername, sizeof(sendername), 0);
            //recv(sockfd, filename, sizeof(filename), 0);


            //printf("***file! Accept?(Y/N)***\n");

            //fgets(accept, 3, stdin);

            //accept[strlen(accept)-1] = '\0';

            //printf("--%s", accept);
            if(strcmp("n", accept) == 0 || strcmp("N", accept) == 0)
            {
                send(sockfd, "n", 1, 0);
                continue;
            }
            else
            {
                send(sockfd, accept, strlen(accept), 0);
            }


            fp = fopen("test.txt", "w");
            printf("hihi\n");
            if(!fp)
            {
                fprintf(stderr, "ERROR!");
                exit(-1);
            }


            while(1)
            {
                bzero(file, sizeof(file));

                recv(sockfd, file, sizeof(file), 0);


                printf("---%s\n", file);

                if(strcmp("EOF", file) == 0)
                {
                    printf("break\n");
                    break;
                }

                printf("----%s", file);

                fprintf(fp, "%s", file);
                //fwrite(file, sizeof(char), numbytes, fp);
            }

            printf("--%s", file);
            fclose(fp);

            continue;


        }*/


        printf("\r%s", rmessage);  // the use of '\r'
    }
}

int name_judgement(char username[])  // judge the name is correct or not
{
    char namejudge[MAX];

    // send user name to server
    send(sockfd, username, strlen(username), 0);

    // receive the result from server
    recv(sockfd, namejudge, sizeof(namejudge), 0);

    if (strcmp("correct", namejudge) != 0) {
        printf("%s", namejudge);

        return -1;
    }

    return 0;
}
