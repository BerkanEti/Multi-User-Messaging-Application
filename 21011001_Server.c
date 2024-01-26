#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>

#define MAX_BUFFER_SIZE 1000
#define MAX_ID_DIGITS 10
#define MAX_COMMAND_SIZE 100

/*
@brief client struct
@param id client id
@param name client name
@param surname client surname
@param phoneNumber client phone number
*/
typedef struct client {
    int id;
    char name[20];
    char surname[20];
    char phoneNumber[20];
}client;


void createClientsFolder();
void createClientIDFolder(client );
void client_handler(int );
int checkClientIDFolder(int );
client getClientInfo(int );
char* sendContactList(int );
int addUserToContactList(int , int );
int deleteUserFromContactList(int ,int );
int sendMessage(int , int , char* );
char* checkUnreadMessages(int );
char* getMessagesFromUnreads(int , int );
char* getMessagesFromClients(int , int );
int deleteMessage(int , int , int );

/*
@brief create client folder if it does not exist

@return
*/
void createClientsFolder() { 
    char command[MAX_COMMAND_SIZE]; // command for system function
    sprintf(command,"mkdir -p clients"); // create clients folder if it does not exist
    system(command); // execute command
}

/*
@brief check client's id folder if it exists

@param id client id

@return 1 if it exists, 0 if it does not exist
*/
int checkClientIDFolder(int id) {
    char command[MAX_COMMAND_SIZE];
    sprintf(command,"ls clients/%d",id); // create command for system function. It will check client's id folder
    int status = system(command);  // execute command
    if(status == 0) { // if it exists
        return 1;
    }
    return 0; // if it does not exist
}


/*
@brief get client info from client_info.csv

@param id client id

@return client struct that contains client info
*/
client getClientInfo(int id) {
    client c; // client struct
    char command[MAX_COMMAND_SIZE]; 
    sprintf(command,"cat clients/%d/client_info.csv",id); // create command for system function. It will read client_info.csv
    FILE *fp = popen(command,"r"); // execute command
    c.id = id; // set client id
    fscanf(fp,"%[^,],%[^,],%s",c.name,c.surname,c.phoneNumber); // get client info from client_info.csv
    pclose(fp);
    return c; 
}

/*
@brief get contact list from contact_list.csv

@param id client id

@return contact list as string. If contact list is empty, return -1
*/
char* sendContactList(int id) {
    FILE *fp; 
    client c;
    char command[MAX_COMMAND_SIZE];;
    sprintf(command,"clients/%d/contact_list.csv",id); // create command for system function. It will read contact_list.csv
    fp = fopen(command,"r");
    char *contact_list = (char *)malloc(sizeof(char)*MAX_BUFFER_SIZE); 
    memset(contact_list,0,MAX_BUFFER_SIZE);

    char line[MAX_BUFFER_SIZE];
    // read contact_list.csv and add it to contact_list string
    while(fgets(line,MAX_BUFFER_SIZE,fp) != NULL) {
        sscanf(line,"%d,%[^,],%[^,],%s",&c.id,c.name,c.surname,c.phoneNumber);
        sprintf(contact_list,"%s\n%8d %18s %18s %18s",contact_list,c.id,c.phoneNumber,c.name,c.surname);
    }

    // if contact_list is empty, add -1 to the contact_list string
    if(strlen(contact_list) == 0) {
        sprintf(contact_list,"-1"); 
    }
    fclose(fp);
    return contact_list;

}

/*
@brief add user to contact list

@param id client id
@param temp_id user id that will be added to contact list

@return 1 if user is added, 0 if user does not exist, -1 if user is already in contact list
*/
int addUserToContactList(int id, int temp_id) {
    int wasAdded = 0; // if user is already in contact list, wasAdded will be 1
    if(checkClientIDFolder(temp_id) == 0) {
        return 0;
    }
    client c = getClientInfo(temp_id); // get user info
    client temp;
    FILE *fp;
    char command[MAX_COMMAND_SIZE];
    char line[MAX_BUFFER_SIZE];

    sprintf(command,"clients/%d/contact_list.csv",id);
    fp = fopen(command,"r");

    // check if user is already in contact list
    while(fgets(line,MAX_BUFFER_SIZE,fp) != NULL && wasAdded == 0) {
        sscanf(line,"%d,%[^,],%[^,],%s",&temp.id,temp.name,temp.surname,temp.phoneNumber);
        if(temp.id == c.id) {
            wasAdded = 1;
        }    
    }

    fclose(fp);
    if(wasAdded) { // if user is already in contact list
        return -1;
    }

    fp = fopen(command,"a");

    fprintf(fp,"%d,%s,%s,%s\n",c.id,c.name,c.surname,c.phoneNumber); // add user to contact list
    fclose(fp); 

    return 1;
}

/*
@brief delete user from contact list

@param id client id
@param temp_id user id that will be deleted from contact list

@return 1 if user is deleted, 0 if user does not exist, -1 if user is not in contact list
*/
int deleteUserFromContactList(int id,int temp_id) {
    int isExist = 0; // if user is not in contact list, isExist will be 0
    if(checkClientIDFolder(temp_id) == 0) { // check if user exists
        return 0;
    }
    client c = getClientInfo(temp_id); // get user info
    client* contact_list = (client *)malloc(sizeof(client)); // contact list
    client temp;
    FILE *fp;
    int currentSize = 0;
    char command[MAX_COMMAND_SIZE];
    char line[MAX_BUFFER_SIZE];

    sprintf(command,"clients/%d/contact_list.csv",id); 
    fp = fopen(command,"r");

    // check if user is in contact list. 
    // Also add users to contact list array except user that will be deleted
    while(fgets(line,MAX_BUFFER_SIZE,fp) != NULL) {
        // get user info
        sscanf(line,"%d,%[^,],%[^,],%s",&temp.id,temp.name,temp.surname,temp.phoneNumber);
        if(temp.id == c.id) {
            isExist = 1;
        }
        else { // add user to contact list array
            currentSize++;
            contact_list = (client *)realloc(contact_list,sizeof(client)*currentSize);
            contact_list[currentSize-1] = temp;
        }

    }

    fclose(fp);
    if(!isExist) { // if user is not in contact list
        return -1;
    }

    fp = fopen(command,"w");
    // write contact list to contact_list.csv
    for(int i=0;i<currentSize;i++) {
        fprintf(fp,"%d,%s,%s,%s\n",contact_list[i].id,contact_list[i].name,contact_list[i].surname,contact_list[i].phoneNumber);
    }

    fclose(fp);
    return 1;
}

/*
@brief save message to messages folder and unread_messages.txt

@param id client id
@param temp_id user id that will be sent message
@param message message that will be sent

@return 1 if message is sent, 0 if user does not exist in contact list or system.
*/
int sendMessage(int id, int temp_id, char* message) { 
    int isExist = 0; // if user does not exist in contact list, isExist will be 0
    int counter = 0;
    if(checkClientIDFolder(temp_id) == 0) { // check if user exists
        return 0;
    }
    client c = getClientInfo(temp_id); // get user info
    client temp;
    FILE *fp;
    FILE *fp2;
    char command[MAX_COMMAND_SIZE];
    char line[MAX_BUFFER_SIZE];

    sprintf(command,"clients/%d/contact_list.csv",id);
    fp = fopen(command,"r");

    // check if user exists in contact list
    while(fgets(line,MAX_BUFFER_SIZE,fp) != NULL && isExist == 0) {
        // get user info
        sscanf(line,"%d,%[^,],%[^,],%s",&temp.id,temp.name,temp.surname,temp.phoneNumber);
        if(temp.id == c.id) {
            isExist = 1;
        }
    }

    fclose(fp);
    if(!isExist) { // if user does not exist in contact list
        return 0;
    }
    

    sprintf(command,"clients/%d/messages/%d.txt",id,c.id); // create command for system function. It will save message to messages folder
    fp2 = fopen(command,"r");
    if(fp2 != NULL) {
         while(fgets(line,MAX_BUFFER_SIZE,fp2) != NULL) {
            if(strlen(line) != 1) {
                counter++;
            }
        }
        fclose(fp2);
    }
    

    fp = fopen(command,"a");
    fprintf(fp,"%d - You : %s\n",counter+1,message); // save message to messages folder. print as you because id is client's id
    fclose(fp);

    sprintf(command,"clients/%d/messages/%d.txt",c.id,id); 


    fp2 = fopen(command,"r");

    if(fp2 != NULL) {
        counter = 0;
         while(fgets(line,MAX_BUFFER_SIZE,fp2) != NULL) {
            if(strlen(line) != 1) {
                counter++;
            }

        }
        fclose(fp2);
    }
    

    
    fp = fopen(command,"a");
    fprintf(fp,"%d - User %d : %s\n",counter+1,id,message); // save message to messages folder. print as user because id is user's id
    fclose(fp);

    sprintf(command,"clients/%d/unread_messages.txt",c.id);
    fp = fopen(command,"a"); // save message to unread_messages.txt in receiver user's folder
    fprintf(fp,"%d : %s\n",id,message); 
    fclose(fp);

    return 1;
}

/*
@brief check unread messages

@param id client id

@return unread messages as string. If there is no unread message, return -1
*/
char* checkUnreadMessages(int id) {
    FILE *fp;
    char command[MAX_COMMAND_SIZE]; 
    char line[MAX_BUFFER_SIZE];
    char* unreadedMessages = (char *)malloc(sizeof(char)*MAX_BUFFER_SIZE);
    memset(unreadedMessages,0,MAX_BUFFER_SIZE);
    int tempID,i; // tempID is user id that sent message
    int* unreadedIds = (int *)malloc(sizeof(int)); // unreadedIds is array that contains user ids that sent message
    int currentSize = 0, isExist;
   // sprintf(unreadedMessages,"");
    sprintf(command,"clients/%d/unread_messages.txt",id);
    fp = fopen(command,"r");
    
    // get user ids that sent message and save them to unreadedIds array
    while(fgets(line,MAX_BUFFER_SIZE,fp) != NULL) {
        tempID = 0; 
        isExist = 0; // if user id is already in unreadedIds array, isExist will be 1
        if(strlen(line) != 1) { // if line is not empty
            for(i=0;line[i] != ' ';i++) { // get user id from line to tempID
            tempID = tempID*10 + (line[i]-'0');
            }
            i=0;
            while(i<currentSize && isExist == 0) { // check if user id is already in unreadedIds array
                if(unreadedIds[i] == tempID) { // if user id is already in unreadedIds array
                    isExist = 1;
                }
                i++;
            }
            if(isExist == 0) { // if user id is not already in unreadedIds array
                currentSize++; // increase currentSize 
                unreadedIds = (int *)realloc(unreadedIds,sizeof(int)*currentSize); // reallocate unreadedIds array
                unreadedIds[currentSize-1] = tempID; // add user id to unreadedIds array
                // add user id to unreadedMessages string
                sprintf(unreadedMessages,"%s\n You have new messages from User %d : ",unreadedMessages,tempID); 
            } 
        } 
    }

    fclose(fp);
    if(strlen(unreadedMessages) == 0) { // if there is no unread message return -1
        return "-1";
    }

    return unreadedMessages;

}

/*
@brief get messages from unread_messages.txt and delete them from unread_messages.txt

@param id client id

@return messages as string. If there is no unread message, return -1
*/
char* getMessagesFromUnreads(int id, int temp_id) {
    FILE *fp;
    char command[MAX_COMMAND_SIZE]; 
    char line[MAX_BUFFER_SIZE];
    char* unreadedMessages = (char *)malloc(sizeof(char)*MAX_BUFFER_SIZE); // unreadedMessages is string that contains unread messages
    // newMessages is string that contains messages except unread messages
    char* newMessages = (char *)malloc(sizeof(char)*MAX_BUFFER_SIZE);
    memset(unreadedMessages,0,MAX_BUFFER_SIZE);memset(newMessages,0,MAX_BUFFER_SIZE);
    int tempID,i;
   // sprintf(unreadedMessages,"");
    //sprintf(newMessages,"");
    sprintf(command,"clients/%d/unread_messages.txt",id);  // create command for system function. It will read unread_messages.txt
    fp = fopen(command,"r");
    
    // get messages from unread_messages.txt which is sent by temp_id
    while(fgets(line,MAX_BUFFER_SIZE,fp) != NULL) {
        tempID = 0; 
        if(strlen(line) != 1) { // if line is not empty
            for(i=0;line[i] != ' ';i++) {
            tempID = tempID*10 + (line[i]-'0');
            }
            if(tempID == temp_id) { // if line is sent by temp_id
                sprintf(unreadedMessages,"%s\n%s",unreadedMessages,line);
            }
            else { // if line is not sent by temp_id
                sprintf(newMessages,"%s\n%s",newMessages,line);
            }
        } 
    }

    fclose(fp);
    if(strlen(unreadedMessages) == 0) { // if there is no unread message return -1
        return "-1";
    }

    fp = fopen(command,"w");

    // delete messages from unread_messages.txt.
    // add messages except unread messages to newMessages string. 
    // strtok function is used to delete \n character from the end of the line
    strtok(newMessages,"\n"); 
    while(newMessages != NULL) {
        fprintf(fp,"%s\n",newMessages);
        newMessages = strtok(NULL, "\n");
    }
    fclose(fp);

    return unreadedMessages;
}

/*
@brief get messages from messages folder which is sent by temp_id. 
       Also delete messages from unread_messages.txt

@param id client id
@param temp_id user id that sent message

@return messages as string. If there is no message, return -1. If user does not exist, return 0
*/
char* getMessagesFromClients(int id, int temp_id) {
    FILE *fp;
    char command[MAX_COMMAND_SIZE];
    char line[MAX_BUFFER_SIZE];
    char* clientMessages = (char *)malloc(sizeof(char)*MAX_BUFFER_SIZE);
    char* newMessages = (char *)malloc(sizeof(char)*MAX_BUFFER_SIZE);
    memset(clientMessages,0,MAX_BUFFER_SIZE);memset(newMessages,0,MAX_BUFFER_SIZE);
    int tempID,i;
    int isExist;
   // sprintf(clientMessages,"");
   // sprintf(newMessages,"");

    if(checkClientIDFolder(temp_id) == 0) {
        return "0";
    }

    sprintf(command,"clients/%d/messages/%d.txt",id,temp_id);
    fp = fopen(command,"r+");
    if(fp == NULL) {
        return "-1";
    }

    // get messages from messages folder
    while(fgets(line,MAX_BUFFER_SIZE,fp) != NULL) {
        if(strlen(line) != 1){
            sprintf(clientMessages,"%s\n%s",clientMessages,line);
        }
        
    }

    fclose(fp);

    if(strlen(clientMessages) == 0) { // if there is no message return -1
        return "-1";
    }


    // delete messages from unread_messages.txt
    sprintf(command,"clients/%d/unread_messages.txt",id);
    fp = fopen(command,"r");
    while(fgets(line,MAX_BUFFER_SIZE,fp) != NULL) {
        tempID = 0;
        isExist = 0;
        if(strlen(line) != 1) {
            for(i=0;line[i] != ' ';i++) {
            tempID = tempID*10 + (line[i]-'0');
            }
            if(tempID != temp_id) {
                sprintf(newMessages,"%s\n%s",newMessages,line);
            }
        } 
    }
    fclose(fp);

    fp = fopen(command,"w");
    strtok(newMessages,"\n");
    while(newMessages != NULL) {
        fprintf(fp,"%s\n",newMessages);
        newMessages = strtok(NULL, "\n");
    }
    fclose(fp);

    return clientMessages;
}

/*
@brief delete message from messages folder

@param id client id
@param temp_id user id that sent message
@param num number of message that will be deleted

@return
*/
int deleteMessage(int id, int temp_id, int num) {
    FILE *fp;
    char command[MAX_COMMAND_SIZE];
    char line[MAX_BUFFER_SIZE];
    char currentMessage[MAX_BUFFER_SIZE];
    char* clientMessages = (char *)malloc(sizeof(char)*MAX_BUFFER_SIZE);
    char* newMessages = (char *)malloc(sizeof(char)*MAX_BUFFER_SIZE);
    memset(clientMessages,0,MAX_BUFFER_SIZE);memset(newMessages,0,MAX_BUFFER_SIZE);
    int tempID,i;
    int counter = 0;
    int isDeleted = 0;
   // sprintf(clientMessages,"");
   // sprintf(newMessages,"");

    sprintf(command,"clients/%d/messages/%d.txt",id,temp_id);
    fp = fopen(command,"r+");

    // get messages from messages folder
    while(fgets(line,MAX_BUFFER_SIZE,fp) != NULL) {
        if(strlen(line) != 1){
            counter++;
            if(counter != num) {
                // take currentMessage : (line = "num(%d) - message" and get only message)
                sscanf(line,"%*d - %[^\n]",currentMessage);
                sprintf(clientMessages,"%s\n%s",clientMessages,currentMessage);
            } 
            else {
                isDeleted = 1;
            }
        }
    }

    fclose(fp);

    if(isDeleted == 0) { // if message is not deleted
        return 0;
    }

    counter = 0;
    fopen(command,"w");
    strtok(clientMessages,"\n");
    sprintf(clientMessages,"%s",clientMessages+1);
    while(clientMessages != NULL && strlen(clientMessages) != 0) {
        counter++;
        fprintf(fp,"%d - %s\n",counter,clientMessages);
        clientMessages = strtok(NULL, "\n");
    }
    fclose(fp);

    return 1;
}

/*
@brief create client id folder and subfolders

@param c client struct

@return
*/
void createClientIDFolder(client c) {
    int id = c.id;
    char command[MAX_COMMAND_SIZE];
    // create client id folder
    sprintf(command,"mkdir -p clients/%d",id);
    system(command);
    // create subfolders
    sprintf(command,"touch clients/%d/contact_list.csv",id);
    system(command);
    sprintf(command,"touch clients/%d/client_info.csv",id);
    system(command);
    sprintf(command,"mkdir -p clients/%d/messages",id);
    system(command);
    sprintf(command,"echo %s,%s,%s > clients/%d/client_info.csv",c.name,c.surname,c.phoneNumber,id);
    system(command);
    sprintf(command,"touch clients/%d/unread_messages.txt",id);
    system(command);
}

/*
@brief handle client and receive messages from client or send messages to client

@param client_socket client socket

@return
*/
void client_handler(int client_socket) {
    char message[MAX_BUFFER_SIZE]; // message that will be received from client
    char id_string[MAX_ID_DIGITS]; // id_string that contains client id as string
    char unreadedMessages[MAX_BUFFER_SIZE]; // unreadedMessages that contains unread messages
    char contact_list[MAX_BUFFER_SIZE]; // contact_list that contains contact list
    client c; // client struct
    char command; // command that will be received from client
    int i; 
    int id,tempID,status,num; // id is client id that handling now, tempID is id that command will be executed on it, status is status of command, num is number of unread messages
    int receive_status,send_status; 
    int isExit = 0;

    while(!isExit){
        memset(message,0,MAX_BUFFER_SIZE);
        receive_status = recv(client_socket,message,sizeof(message)-1,0);
        if(receive_status == -1) {
            printf("Receive failed\n");
            return;
        }
        if(receive_status == 0) {
            printf("Client disconnected\n");
            return;
        }
        // get first letter of the message
        command = message[0];

        if(command == 'n') { // if client is new client add client to clients folder
            for(i=2;message[i] != ' ';i++) {
                id_string[i-2] = message[i];
            }
            id_string[i-2] = '\0';
            id = atoi(id_string); // get client id from message and convert it to integer
            status = checkClientIDFolder(id);

            if(status) { // if client exists
                memset(message,0,MAX_BUFFER_SIZE);
                sprintf(message,"%d",id);
                send_status = send(client_socket,message,strlen(message),0);
                if(send_status == -1) {
                    printf("Send failed\n");
                    return;
                }

                c = getClientInfo(id);
            }

            else { // if client does not exist
                memset(message,0,MAX_BUFFER_SIZE);
                sprintf(message,"-1");
                send_status = send(client_socket,message,strlen(message),0);
                if(send_status == -1) {
                    printf("Send failed\n");
                    return;
                }
                memset(message,0,MAX_BUFFER_SIZE);
                receive_status = recv(client_socket,message,sizeof(message)-1,0);
                if(receive_status == -1) {
                    printf("Receive failed\n");
                    return;
                }
                if(receive_status == 0) {
                    printf("Client disconnected\n");
                    return;
                }
                sscanf(message,"%s %s %s",c.name,c.surname,c.phoneNumber);
                c.id = id;
                createClientIDFolder(c);
            }
            printf("Client %d connected\n",id); 
        }

        if(command == 'l') { // if client request for contact list
            printf("Client %d requested contact list\n",id);
            strcpy(contact_list,sendContactList(id)); // get contact list
            send_status = send(client_socket,contact_list,strlen(contact_list),0); // send contact list to client
            if(send_status == -1) { 
                printf("Send failed\n");
                return;
            }
        }

        else if(command == 'a') { // if client request for add user
            printf("Client %d requested add user\n",id);
            memset(id_string,0,MAX_ID_DIGITS);
            for(i=2;message[i] != ' ';i++) {
                id_string[i-2] = message[i];
            }
            id_string[i-2] = '\0';
            tempID = atoi(id_string); // get user id that will be added to contact list
            status = addUserToContactList(id,tempID); // add user to contact list
            if(status == 1) { // if user is added
                send_status = send(client_socket,"1",1,0);
            }

            else if(status == -1) { // if user is already in contact list
                send_status = send(client_socket,"-1",2,0);
            }
            else { // if user does not exist
                send_status = send(client_socket,"0",1,0);
            }

            if(send_status == -1) {
                printf("Send failed\n");
                return;
            }
        }

        else if(command == 'd') { // if client request for delete user
            printf("Client %d requested delete user\n",id);
            memset(id_string,0,MAX_ID_DIGITS);
            for(i=2;message[i] != ' ';i++) {
                id_string[i-2] = message[i];
            }
            id_string[i-2] = '\0';
            tempID = atoi(id_string); // get user id that will be deleted from contact list
            status = deleteUserFromContactList(id,tempID); // delete user from contact list
            if(status == 1) { // if user is deleted
                send_status = send(client_socket,"1",1,0);
            }

            else if(status == -1) { // if user is not in contact list
                send_status = send(client_socket,"-1",2,0);
            }
            else { // if user does not exist
                send_status = send(client_socket,"0",1,0);
            }
            if(send_status == -1) {
                printf("Send failed\n");
                return;
            }

        }

        else if(command == 's') { // if client request for send message
            printf("Client %d requested send message\n",id);
            memset(id_string,0,MAX_ID_DIGITS);
            for(i=2;message[i] != ' ';i++) {
                id_string[i-2] = message[i];
            }
            id_string[i-2] = '\0';
            tempID = atoi(id_string); // get user id that will be sent message
            status = sendMessage(id,tempID,message+i+1); 
            if(status == 1) { // if message is sent
                send_status = send(client_socket,"1",1,0);
            }
            else { // if user does not exist
                send_status = send(client_socket,"0",1,0);
            }
            if(send_status == -1) {
                printf("Send failed\n");
                return;
            }
        }
 
        else if(command == 'c') { // if client request for check unread messages
            printf("Client %d requested check unread messages\n",id);
            strcpy(unreadedMessages,checkUnreadMessages(id)); // get user ids that sent message
            send_status = send(client_socket,unreadedMessages,strlen(unreadedMessages),0); // send user ids that sent message to client
            if(send_status == -1) {
                printf("Send failed\n");
                return;
            }

            if(strcmp(unreadedMessages,"-1") != 0) { // if there are unread message(s)
                receive_status = recv(client_socket,message,sizeof(message)-1,0);  // get user id that will be read messages
                if(receive_status == -1) {
                    printf("Receive failed\n");
                    return;
                }

                memset(id_string,0,MAX_ID_DIGITS);
                for(i=0;message[i] != ' ';i++) {
                    id_string[i] = message[i];
                }
                id_string[i] = '\0';
                tempID = atoi(id_string); // get user id that will be read messages
                strcpy(unreadedMessages,getMessagesFromUnreads(id,tempID)); // get messages from unread_messages.txt and delete them from unread_messages.txt
                send_status = send(client_socket,unreadedMessages,strlen(unreadedMessages),0);
                if(send_status == -1) {
                    printf("Send failed\n");
                    return;
                }
            }
        }

        else if(command == 'm') { // if client request for read chat history
            printf("Client %d requested read messages\n",id);
            memset(id_string,0,MAX_ID_DIGITS);
            for(i=2;message[i] != ' ';i++) {
                id_string[i-2] = message[i];
            }
            id_string[i-2] = '\0';
            tempID = atoi(id_string); // get user id that will be read messages
            strcpy(unreadedMessages,getMessagesFromClients(id,tempID));
            send_status = send(client_socket,unreadedMessages,strlen(unreadedMessages),0); // send messages to client
            if(send_status == -1) {
                printf("Send failed\n");
                return;
            }

            if(strcmp(unreadedMessages,"-1") != 0 && strcmp(unreadedMessages,"0") != 0) { // if there are messages
                memset(message,0,MAX_BUFFER_SIZE);
                receive_status = recv(client_socket,message,sizeof(message)-1,0);
                if(receive_status == -1) {
                    printf("Receive failed\n");
                    return;
                }
                if(receive_status == 0) {
                    printf("Client disconnected\n");
                    return;
                }


                if(strcmp(message,"1") == 0) { // if client request for delete message(s)
                    receive_status = recv(client_socket,message,sizeof(message)-1,0);
                    if(receive_status == -1) {
                        printf("Receive failed\n");
                        return;
                    }
                    if(receive_status == 0) {
                        printf("Client disconnected\n");
                        return;
                    }
                    sscanf(message,"%s",id_string);
                    num = atoi(id_string); // get number of unread messages

                    status = deleteMessage(id,tempID,num); // delete message from messages folder
                    if(status == 1) {
                        send_status = send(client_socket,"1",1,0);
                    }
                    else {
                        send_status = send(client_socket,"0",1,0);
                    }

                    if(send_status == -1) {
                        printf("Send failed\n");
                        return;
                    }
                }
            }
            
            
        }

        else if(command == 'e') { // if client request for exit
            printf("Client %d disconnected\n",id);
            isExit = 1;
        }
    }

}

int main() { 
    int server_socket = socket(AF_INET,SOCK_STREAM,0);  // create socket
    createClientsFolder(); // create clients folder if it does not exist
    if(server_socket == -1) { // check if socket is created
        printf("Socket could not be created\n");
        return 0;
    }
    printf("Socket created\n");

    struct sockaddr_in server_address; // server address
    server_address.sin_family = AF_INET; // set address family
    server_address.sin_port = htons(9002); // set port
    server_address.sin_addr.s_addr = INADDR_ANY; // set ip address


    int bind_status = bind(server_socket,(struct sockaddr *)&server_address,sizeof(server_address)); // bind socket
    if(bind_status == -1) {
        printf("Socket could not be bound\n");
        return 0;
    }
    printf("Socket bound\n");

    int listen_status = listen(server_socket,5); // listen socket for clients 
    if(listen_status == -1) {
        printf("Listen failed\n");
        return 0;
    }

    pthread_t thread; // Client handler will be executed in thread. Because server will handle multiple clients
    while(1) {
        struct sockaddr_in client_address; // client address
        socklen_t client_address_size = sizeof(client_address); // client address size
        uintptr_t  client_socket = accept(server_socket,(struct sockaddr *)&client_address,&client_address_size); // accept client
        if(client_socket == -1) {
            printf("Accept failed\n");
            return 0;
        }
        printf("Client accepted\n");

        // create thread for client handler
        if(pthread_create(&thread,NULL,(void *)client_handler,(void *)client_socket) < 0) {
            printf("Thread could not be created\n");
            return 0;
        }
    }

    close(server_socket); // close socket
    printf("Socket closed\n");

    return 0;
}