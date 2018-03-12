/* demo_client.c - code for example client program that uses TCP */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

/*------------------------------------------------------------------------
* Program: demo_client
*
* Purpose: allocate a socket, connect to a server, and print all output
*
* Syntax: ./demo_client server_address server_port
*
* server_address - name of a computer on which server is executing
* server_port    - protocol port number server is using
*
*------------------------------------------------------------------------
*/
int main( int argc, char **argv) {
	struct hostent *ptrh; /* pointer to a host table entry */
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold an IP address */
	int sd; /* socket descriptor */
	uint16_t port; /* protocol port number */
	char *host; /* pointer to host name */
	int n; /* number of characters read */
	char buf[1000]; /* buffer for data from the server */
	int participant_num;
	char message[1024];

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./client server_address server_port\n");
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[2]); /* convert to binary */

	if (port > 0) /* test for legal value */
		sad.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"Error: bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	host = argv[1]; /* if host argument specified */

	/* Convert host name to equivalent IP address and copy to sad. */
	ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		fprintf(stderr,"Error: Invalid host: %s\n", host);
		exit(EXIT_FAILURE);
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Map TCP transport protocol name to protocol number. */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket. */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Connect the socket to the specified server. */
	if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"connect failed\n");
		exit(EXIT_FAILURE);
	}

	char participant_username[1000];
	char valid_username = 'N';
	uint8_t username_len;
	int conteo=0;
	printf("Participant %d connected...\n",sd);

	while(valid_username != 'Y'){
		conteo ++;
		printf("Please enter username:\n");
		fgets(participant_username,sizeof(participant_username),stdin);
		username_len = strlen(participant_username) -1;

		if(participant_username[username_len]=='\n'){
			participant_username[username_len] = '\0';
		}
		fprintf(stderr, "Username len: %d\n", username_len);

		int send_username = send(sd, &participant_username, sizeof(participant_username),0);


		if(send_username <= 0 ){
			fprintf(stderr, "Error sending username info to server\n");
			exit(EXIT_FAILURE);
		}
		printf("Username sent: %s\n", participant_username);
		int received_validity = recv(sd,&valid_username, sizeof(valid_username),0);
     printf("valor del server: %c \n", valid_username);
		if(received_validity<=0){
			fprintf(stderr, "Error receiving validity \n");
			exit(EXIT_FAILURE);
		}
		printf("username validity from server: %c \n", valid_username);


	}
printf("repetido: %d \n", conteo);
	//main chat loop
	while(1){
		printf("%s\n", "Enter message:");
		fgets(message,1024,stdin);
		printf("User entered: %s\n", message);

		if(send(sd,&message, sizeof(message),0) <= 0){
			fprintf(stderr, "%s\n", "Error sending participant message");
			exit(EXIT_FAILURE);
		}
	}

	close(sd);

	exit(EXIT_SUCCESS);
}
