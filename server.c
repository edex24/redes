#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

int main(){
  //create Socket
  int network_socket;
  network_socket = socket (AF_INET, SOCK_STREAM, 0);

  //specify an address for the Socket
  struct sockaddr_in server_address;
  server_address.sin_family=AF_INET;
  server_address.sin_port=htons(8080);
  server_address.sin_addr.s_addr=INADDR_ANY;

  //conecta
  int connection_status=connect (network_socket,(struct sockaddr *)&server_address,sizeof(server_address));
  //conect regresa un integer y se puede usar pasa saber si se conecto
  if(connection_status==-1){
    printf("Hubo un error al conectar el socket\n");
  }

  //recibir data del server+
  char server_response[256];
  recv(network_socket,&server_response, sizeof(server_response),0);

  //print our servers response
  printf("The server sent the dat %s\n",server_response );

  //cerrar Socket
  close (network_socket);

  return 0;
}
