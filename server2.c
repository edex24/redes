#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

int main(){

//char server_message[256]=("you haver reached the server!");
char message[1000];
//crete the server socket
int server_socket;
server_socket = socket (AF_INET, SOCK_STREAM, 0);
if (server_socket == -1)
    {
        printf("Could not create socket \n");
    }
    printf("Sockcet created \n");

//define the server address
struct sockaddr_in server_address;
server_address.sin_family=AF_INET;
server_address.sin_port=htons(8080);
server_address.sin_addr.s_addr=INADDR_ANY;

//bind the socket to our specified ip and port
//bind (server_socket,(struct sockaddr *)&server_address,sizeof(server_address));
if( bind(server_socket,(struct sockaddr *)&server_address , sizeof(server_address)) < 0)
    {
        //print the error message
        printf("Bind Failed \n");
        return 1;
    }
    printf("Bind Success \n");

//Listen for connection
listen(server_socket,5);
printf("Waiting for incoming connections...\n");

//aceptando sockets
int client_socket;
client_socket= accept(server_socket,NULL,NULL);
if (client_socket < 0)
    {
        printf("accept Failed \n");
        return 1;
    }
    printf("Connection accepted \n");

printf("Enter message : \n");
scanf("%s" , message);

//Enviando mensaje
send(client_socket,message , strlen(message) ,0);

//close the socket
close(server_socket);

  return 0;
}
