#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1000
#define MAX_TEXT_SIZE 500


int main(int argc,char *argv[]) {
    char message[MAX_BUFFER_SIZE]; // message to be sent to server or received from server
    char text[MAX_TEXT_SIZE]; // text to be sent to server
    int opt; // option selected by user
    char name[20], surname[20], phoneNumber[20]; // client's name, surname and phone number
    int id; // client's id
    int temp_id; // id of the user to be added, deleted, messaged or chatted with
    int isExit = 0,isDelete,num; // if isExit = 1, client will exit
    int send_status,receive_status; // status of send and receive functions
    if(argc != 2) { // if number of arguments is not 2, exit
        printf("Invalid number of arguments\n");
        return 0;
    }

    id = atoi(argv[1]); // convert id from string to integer

    int client_socket = socket(AF_INET,SOCK_STREAM,0); // create a socket
    if(client_socket == -1) { // if socket could not be created, exit
        printf("Socket could not be created\n");
        return 0;
    }
    //printf("Socket created\n");

    struct sockaddr_in server_address; // create a socket address
    memset(&server_address,0,sizeof(server_address)); // set all bytes of server_address to 0
    server_address.sin_family = AF_INET; // set address family to AF_INET
    server_address.sin_port = htons(9002); // set port number to 9002
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // set ip address to

    
    int connect_status = connect(client_socket,(struct sockaddr *)&server_address,sizeof(server_address)); 
    // connect to the server
    // if connection could not be established, exit
    if(connect_status == -1) {
        printf("Connection failed\n");
        return 0;
    }
    printf("Connected\n"); // if connection is established, print "Connected"

    char id_string[10]; // id to be sent to server as a string
    sprintf(id_string,"n %d ",id); // convert id to string
    send_status = send(client_socket,id_string,strlen(id_string),0); // send id to server
    if(send_status == -1) {
        printf("Send failed\n");
        return 0;
    }
   // printf("Id sent\n");

    memset(message,0,MAX_BUFFER_SIZE); // set all bytes of message to 0
    receive_status = recv(client_socket,message,sizeof(message)-1,0); // receive message from server
    if(receive_status == -1) { // if receive failed, exit
        printf("Receive failed\n");
        return 0;
    }
    if(receive_status == 0) { // if server disconnected, exit
        printf("Server disconnected\n");
        return 0;
    }
    int status; // status of the client
    sscanf(message,"%d",&status); // convert message to integer
    if(status == -1) { // if status is -1, that means client was not registered before
        // so client will register to the system
        printf("You are registering to the system\n"); 
        printf("Enter your name: "); 
        scanf("%s",name); // get name from user
        printf("Enter your surname: ");
        scanf("%s",surname); // get surname from user
        printf("Enter your phone number: ");
        scanf("%s",phoneNumber); // get phone number from user
        sprintf(message,"%s %s %s",name,surname,phoneNumber); // convert name, surname and phone number to string
        send_status = send(client_socket,message,strlen(message),0); // send name, surname and phone number to server
        if(send_status == -1) { // if send failed, exit
            printf("Send failed\n");
            return 0;
        }
        printf("Registration message sent\n\n"); // if send is successful, print "Registration message sent"
    }


    while(!isExit){ // while client does not want to exit
        memset(message,0,MAX_BUFFER_SIZE); // set all bytes of message to 0 before each iteration
        // print options
        printf("\n\nSelect an option: \n" 
            "1. List Contacts \n"
            "2. Add User : \n"
            "3. Delete User : \n"
            "4. Send Message : \n"
            "5. Check Messages : \n"
            "6. Show Chat With a User : \n"
            "7. Exit : \n");
        scanf("%d",&opt); // get option from user
        if(opt == 1) { // if option is 1, client wants to list contacts
            sprintf(message,"l"); // convert option to string as "l" (list)
            send_status = send(client_socket,message,strlen(message),0); // send l to server
            if(send_status == -1) { // if send failed, exit
                printf("Send failed\n");
                return 0;
            }
            
            memset(message,0,MAX_BUFFER_SIZE); // set all bytes of message to 0 before receiving message from server
            receive_status = recv(client_socket,message,sizeof(message)-1,0); // receive message from server
            // check if receive failed or server disconnected
            if(receive_status == -1) {
                printf("Receive failed\n");
                return 0;
            }   
            else if(receive_status == 0) {
                printf("Server disconnected\n");
                return 0;
            }
            else {
                if(strcmp(message,"-1") == 0) { // if message is -1, that means client has no contacts
                    printf("You have no contact\n"); // print "You have no contact"
                }
                else {
                    // if message is not -1, that means client has contacts
                    // print contacts that comes from server
                    printf("     ID     -     PHONE-NUMBER     -     NAME     -     SURNAME     \n");
                    printf("%s",message);
                }
            }
        }

        else if(opt == 2) { // if option is 2, client wants to add a user
            printf("Enter id: "); scanf("%d",&temp_id);  // get id of the user to be added from user
            // check if id is equal to client's id. If it is, get another id from user
            while(id == temp_id) { 
                printf("Invalid ID : You can not add yourself\n");
                printf("Enter id: "); scanf("%d",&temp_id); // get id of the user to be added from user
            }
            sprintf(message,"a %d ",temp_id); // convert option and id to string as "a num" (add id_to_be_added)
            send_status = send(client_socket,message,strlen(message),0); // send "a id" to server
            if(send_status == -1) { // if send failed, exit
                printf("Send failed\n");
                return 0;
            }

            memset(message,0,MAX_BUFFER_SIZE); 
            receive_status = recv(client_socket,message,sizeof(message)-1,0); // receive message from server
            // check if receive failed or server disconnected
            if(receive_status == -1) {
                printf("Receive failed\n");
                return 0;
            }   
            else if(receive_status == 0) {
                printf("Server disconnected\n");
                return 0;
            }
            else {
                if(strcmp(message,"0") == 0) { // if message is 0, that means user that client wants to add does not exist
                    printf("User does not exist\n");
                }
                else if(strcmp(message,"-1") == 0) { // if message is -1, that means user that client wants to add is already added
                    printf("User has already been added\n");
                } 
                else { // if message is not 0 or -1, that means user that client wants to add is not added before. 
                    //So client adds user correctly
                    printf("User added\n"); 
                }
            }
        }

        else if(opt == 3) { // if option is 3, client wants to delete a user
            printf("Enter id: "); scanf("%d",&temp_id); // get id of the user to be deleted from user
            // check if id is equal to client's id. If it is, get another id from user
            while(id == temp_id) { 
                printf("Invalid ID : You can not delete yourself\n");
                printf("Enter id: "); scanf("%d",&temp_id); 
            }
            sprintf(message,"d %d ",temp_id); // convert option and id to string as "d num" (delete id_to_be_deleted)
            send_status = send(client_socket,message,strlen(message),0); // send "d id" to server
            if(send_status == -1) { // if send failed, exit
                printf("Send failed\n");
                return 0;
            }

            memset(message,0,MAX_BUFFER_SIZE);
            receive_status = recv(client_socket,message,sizeof(message)-1,0); 
            // check if receive failed or server disconnected
            if(receive_status == -1) {
                printf("Receive failed\n");
                return 0;
            }   
            else if(receive_status == 0) {
                printf("Server disconnected\n");
                return 0;
            }
            else {
                if(strcmp(message,"0") == 0) { // if message is 0, that means user that client wants to delete does not exist
                    printf("User does not exist in system\n");
                }
                else if(strcmp(message,"-1") == 0) { // if message is -1, that means user that client wants to delete is not added or deleted before
                    printf("User does not exist in your contact list\n");
                }
                else { // if message is not 0 or -1, that means user that client wants to delete is added before.
                    printf("User deleted\n");
                }
            }
        }

        else if(opt == 4) { // if option is 4, client wants to send a message
            printf("Enter id: "); scanf("%d",&temp_id); getchar(); // get id of the user to be messaged from user
            // getchar() is used to get rid of '\n' character in stdin
            // check if id is equal to client's id. If it is, get another id from user
            while(id == temp_id) { 
                printf("Invalid ID : You can not message yourself\n");
                printf("Enter id: "); scanf("%d",&temp_id); getchar(); 
            }
            printf("Enter message: ");
            fgets(text,MAX_TEXT_SIZE,stdin); // get message from user
            sprintf(message,"s %d %s",temp_id,text); // convert option, id and message to string as "s id text" (send id_receiver message_to_be_sent)
            send_status = send(client_socket,message,strlen(message),0); // send "s id text" to server
            if(send_status == -1) { // if send failed, exit
                printf("Send failed\n");
                return 0;
            }
            memset(message,0,MAX_BUFFER_SIZE); 
            // receive message from server
            receive_status = recv(client_socket,message,sizeof(message)-1,0);

            // check if receive failed or server disconnected
            // if message is 1, that means message is delivered successfully
            if(strcmp(message,"1") == 0) { // 
                printf("Your message has been delivered successfully\n");
            }

            // if message is 0, that means user that client wants to message does not exist or is not added before
            else if(strcmp(message,"0") == 0) {
                printf("User does not exist in your contact list\n");
            }
        }


        else if(opt == 5) { // if option is 5, client wants to check messages
            sprintf(message,"c"); // convert option to string as "c" (check)
            send_status = send(client_socket,message,strlen(message),0); // send "c" to server
            if(send_status == -1) {
                printf("Send failed\n");
                return 0;
            }
            memset(message,0,MAX_BUFFER_SIZE);
            receive_status = recv(client_socket,message,sizeof(message)-1,0);
            // check if receive failed or server disconnected
            if(receive_status == -1) {
                printf("Receive failed\n");
                return 0;
            }   
            else if(receive_status == 0) {
                printf("Server disconnected\n");
                return 0;
            }
            else {
                if(strcmp(message,"-1") == 0) { // if message is -1, that means client has no unread messages
                    printf("You have no unread messages\n");
                }
                else { // if message is not -1, that means client has unread messages
                    printf("\n%s\n",message); // print user ids that sent messages to client
                    // get id of the user whose messages will be read from user
                    printf("Whose messages do you want to read? : "); scanf("%d",&temp_id);  
                    memset(message,0,MAX_BUFFER_SIZE);
                    sprintf(message,"%d ",temp_id); // convert id to string
                    send_status = send(client_socket,message,strlen(message),0); // send id to server
                    if(send_status == -1) {
                        printf("Send failed\n");
                        return 0;
                    }
                    memset(message,0,MAX_BUFFER_SIZE); 
                    receive_status = recv(client_socket,message,sizeof(message)-1,0); // receive message from server
                    // check if receive failed or server disconnected
                    if(receive_status == -1) {
                        printf("Receive failed\n");
                        return 0;
                    }   
                    else if(receive_status == 0) {
                        printf("Server disconnected\n");
                        return 0;
                    }

                    if(strcmp(message,"-1") == 0) {  // if message is -1, that means client has no messages from user whose id is temp_id
                        printf("You have no messages from this user\n");
                    }
                    else { // if message is not -1, that means client has messages from user whose id is temp_id
                        printf("Messages from User %d\n",temp_id);
                        printf("\n%s\n",message);
                    }
                }
            }
        }
        else if(opt == 6){ // if option is 6, client wants to show a chat and delete a message from a chat
            printf("Enter id: "); scanf("%d",&temp_id); // get id of the user to be chatted with from user
            // check if id is equal to client's id. If it is, get another id from user
            while(id == temp_id) {
                printf("Invalid ID : You can not chat with yourself\n");
                printf("Enter id: "); scanf("%d",&temp_id);
            }

            sprintf(message,"m %d ",temp_id); // message to be sent to server as "m id" (message id_receiver)
            send_status = send(client_socket,message,strlen(message),0); // send "m id" to server
            if(send_status == -1) { 
                printf("Send failed\n");
                return 0;
            }

            memset(message,0,MAX_BUFFER_SIZE);
            receive_status = recv(client_socket,message,sizeof(message)-1,0);
            // check if receive failed or server disconnected
            if(receive_status == -1) {
                printf("Receive failed\n");
                return 0;
            }   
            else if(receive_status == 0) {
                printf("Server disconnected\n");
                return 0;
            }
            else {
                if(strcmp(message,"-1") == 0) { // if message is -1, that means client has no chat with user whose id is temp_id
                    printf("You don't have chat with User %d\n",temp_id);
                } 
                else if(strcmp(message,"0") == 0) { // if message is 0, that means user whose id is temp_id does not exist or is not added before
                    printf("User does not exist in your contact list\n");
                }
                else { // if message is not -1 or 0, that means client has chat with user whose id is temp_id
                    printf("Messages from User %d\n",temp_id);
                    printf("\n%s\n",message);
                    printf("\nDo you want to delete a message from this chat? (1/0 -> y/n) : "); scanf("%d",&isDelete);
                    if(isDelete == 1) {
                        // send "1" to server to indicate that client wants to delete a message from this chat
                        sprintf(message,"1");
                        send_status = send(client_socket,message,strlen(message),0);
                        if(send_status == -1) {
                            printf("Send failed\n");
                            return 0;
                        }
                        printf("Enter message num to be deleted: "); scanf("%d",&num); // get message num to be deleted from user
                        sprintf(message,"%d",num); // convert message num to string
                        send_status = send(client_socket,message,strlen(message),0); // send message num to server
                        if(send_status == -1) {
                            printf("Send failed\n");
                            return 0;
                        }
                        memset(message,0,MAX_BUFFER_SIZE);
                        receive_status = recv(client_socket,message,sizeof(message)-1,0); // receive message from server
                        // check if receive failed or server disconnected
                        if(receive_status == -1) {
                            printf("Receive failed\n");
                            return 0;
                        }   
                        else if(receive_status == 0) {
                            printf("Server disconnected\n");
                            return 0;
                        }
                        else {
                            if(strcmp(message,"0") == 0) { // if message is -1, that means message num is invalid
                                printf("Invalid message num\n");
                            }
                            else { // if message is not -1, that means message num is valid
                                printf("Message deleted\n");
                            }
                        }
                    }
                    else {
                        sprintf(message,"0");
                        send_status = send(client_socket,message,strlen(message),0);
                        if(send_status == -1) {
                            printf("Send failed\n");
                            return 0;
                        }

                    }
                }
            }
        }

        else if(opt == 7) { // if option is 7, client wants to exit
            sprintf(message,"e"); // convert option to string as "e" (exit)
            send_status = send(client_socket,message,strlen(message),0); // send "e" to server
            if(send_status == -1) { // if send failed, exit
                printf("Send failed\n");
                return 0;
            }
            printf("Exiting\n");
            isExit = 1;
        }

        else { // if option is not 1,2,3,4,5,6 or 7, that means option is invalid
            printf("Invalid option\n");
        }
    }

    close(client_socket);

    return 0;


}
