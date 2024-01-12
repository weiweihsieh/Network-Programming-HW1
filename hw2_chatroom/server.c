#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CLIENT 10
#define MAX 1024

struct sockaddr_in clientInfo[MAX_CLIENT];
socklen_t clientlen = sizeof(clientInfo[0]);


typedef struct {
    int connectfd;
    char name[MAX];

} client_use;

client_use user[MAX_CLIENT];

int clientNum = 0;


static void *doit(void *arg);
int name_judgement(char username[], int index, int receiver);
int find_name_index(char username[]);


int main()
{
    int listenfd, i;
    int index = 0;
    struct sockaddr_in serverInfo;

    for (i = 0; i < MAX_CLIENT; i++)
        bzero(&user[i], sizeof(user[i]));


    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serverInfo, sizeof(serverInfo));
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverInfo.sin_port = htons(7777);

    bind(listenfd, (struct sockaddr *) &serverInfo, sizeof(serverInfo));

    listen(listenfd, 1024);


    while (clientNum <= MAX_CLIENT) {
        user[clientNum].connectfd = accept(
            listenfd, (struct sockaddr *) &clientInfo[clientNum], &clientlen);

        clientNum++;
        index = clientNum - 1;

        pthread_t tid;

        pthread_create(&tid, NULL, &doit, &index);
    }
}

static void *doit(void *arg)
{
    int i, receiver;
    int index = *(int *) arg;
    char username[MAX], filename[MAX];
    char userlist[MAX] = {}, tmp[MAX] = {};
    char message_send[MAX], message_recv[MAX], namejudge[MAX];
    char name[30] = {};
    // char accept[3];

    // FILE *fp;
    // int numbytes;
    // char file[MAX], files[MAX];

    /*welcome message*/
    char welcome_message[MAX];
    bzero(welcome_message, sizeof(welcome_message));
    sprintf(welcome_message,
            "------------WELCOME-----------\nYour client number: "
            "%d\n------------------------------\n",
            index);
    send(user[index].connectfd, welcome_message, strlen(welcome_message), 0);

    /*receive user name*/
    while (1) {
        recv(user[index].connectfd, name, sizeof(name), 0);
        strcpy(user[index].name, name);

        bzero(namejudge, sizeof(namejudge));
        for (i = 0; i < MAX_CLIENT; i++) {
            if (strcmp(name, user[i].name) == 0 && i != index) {
                sprintf(namejudge,
                        "The user name is repeat! Please use others!\n");
                send(user[index].connectfd, namejudge, strlen(namejudge), 0);
                break;
            }
        }

        // the name does not exist
        if (i == MAX_CLIENT) {
            sprintf(namejudge, "correct");
            send(user[index].connectfd, namejudge, strlen(namejudge), 0);
            break;
        }
    }

    while (1) {
        bzero(message_recv, sizeof(message_recv));
        bzero(message_send, sizeof(message_send));

        recv(user[index].connectfd, message_recv, sizeof(message_recv), 0);


        if (strcmp("Q", message_recv) == 0) {  // quit
            sprintf(message_send, "%s is offline!\n", name);

            printf("%s", message_send);
            for (i = 0; i < clientNum; i++) {
                if (i == index || user[i].connectfd == 0)
                    continue;

                send(user[i].connectfd, message_send, sizeof(message_send), 0);
            }

            user[index].connectfd = -1;
            pthread_exit(0);
        } else if (strcmp("W", message_recv) == 0) {  // who is online
            bzero(userlist, sizeof(userlist));

            sprintf(userlist, "-------All online users-------\n");
            for (i = 0; i < clientNum; i++) {
                if (user[i].connectfd == 0 || user[i].connectfd == -1)
                    continue;

                sprintf(tmp, "%s\n", user[i].name);
                strcat(userlist, tmp);
            }

            strcat(userlist, "------------------------------\n");
            send(user[index].connectfd, userlist, strlen(userlist), 0);
        } else if (strcmp("D", message_recv) == 0) {  // designate a client
            bzero(username, sizeof(username));

            recv(user[index].connectfd, username, sizeof(username), 0);

            receiver = find_name_index(username);

            /*judge the user name is correct or not*/
            if (name_judgement(username, index, receiver) == -1) {
                continue;
            }

            recv(user[index].connectfd, message_recv, sizeof(message_recv), 0);

            sprintf(message_send, "*****%s: %s\n", name, message_recv);

            send(user[receiver].connectfd, message_send, strlen(message_send),
                 0);
        }
        /*else if((strcmp("F", message_recv) == 0))
        {

            bzero(file, sizeof(file));
            bzero(files, sizeof(files));
            bzero(username, sizeof(username));
            bzero(filename, sizeof(filename));
            bzero(accept, sizeof(accept));

                        recv(user[index].connectfd, username, sizeof(username),
        0);

                        receiver = find_name_index(username);

                        //judge the user name is correct or not
                        if(name_judgement(username, index, receiver) == -1)
                        {
                                continue;
                        }

            //filename
            recv(user[index].connectfd, filename, sizeof(filename), 0);


            send(user[receiver].connectfd, "file", 4, 0);

            //send(user[receiver].connectfd, user[index].name,
        strlen(user[index].name), 0);
            //send(user[receiver].connectfd, filename, strlen(filename), 0);

            //determine whether receiver wants the file or not
            //recv(user[receiver].connectfd, accept, sizeof(accept), 0);
            if(strcmp("n", accept) == 0)
            {
                send(user[index].connectfd, accept, strlen(accept), 0);
                continue;
            }
            else
            {
                send(user[index].connectfd, accept, strlen(accept), 0);
            }

            while(1)
            {
                bzero(file, sizeof(file));

                recv(user[index].connectfd, file, sizeof(file), 0);
        //MSG_DONTWAIT: nonblock when nothing send
                //printf("%d\n", numbytes);
                //printf("---%s", file);


                send(user[receiver].connectfd, file, strlen(file), 0);
                //send(user[receiver].connectfd, file, sizeof(file), 0);

                if(strcmp("EOF", file) == 0)
                {
                    //printf("break\n");
                    break;
                }

                //strcat(files, file);
                //write(user[receiver].connectfd, file, numbytes);

                //printf("%s", file);
            }


            //printf("%s", files);
            //fclose(fp);
            continue;
        }*/
        else {
            sprintf(message_send, "%s: %s\n", name, message_recv);
            printf("%s", message_send);


            for (i = 0; i < clientNum; i++) {
                if (i == index || user[i].connectfd == 0)
                    continue;

                send(user[i].connectfd, message_send, sizeof(message_send), 0);
            }
        }
    }
}

int find_name_index(char username[])
{
    int i;

    for (i = 0; i < clientNum; i++) {
        if (strcmp(username, user[i].name) == 0)
            break;
    }

    return i;
}


int name_judgement(char username[], int index, int receiver)
{
    char namejudge[MAX];

    /*judge the user name is correct or not*/
    if (receiver == clientNum || user[receiver].connectfd == -1) {
        sprintf(namejudge, "Can not find user '%s'\n", username);
        send(user[index].connectfd, namejudge, sizeof(namejudge), 0);

        return -1;
    } else if (receiver == index) {
        sprintf(namejudge, "Can not send to yourself!\n");
        send(user[index].connectfd, namejudge, sizeof(namejudge), 0);

        return -1;
    } else {
        sprintf(namejudge, "correct");
        send(user[index].connectfd, namejudge, sizeof(namejudge), 0);

        return 0;
    }
}
