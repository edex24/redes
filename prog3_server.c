/* prog3_server.c - code for example server program that uses TCP */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>


//each participant will contain this info
struct a_participant{
	int sd;
	char username[11];
	bool is_active;
};

#define QLEN 6 /* size of request queue */
int visits = 0; /* counts client connections */
int checkUsername(char* username, struct a_participant participants[]);



/*------------------------------------------------------------------------
* Program: prog3_server
*
* Purpose: allocate a socket and then repeatedly execute the following:
* (1) wait for the next connection from a client
* (2) send a short message to the client
* (3) close the connection
* (4) go back to step (1)
*
* Syntax: ./prog3_server participant_port observer_port
*
* port - protocol port number to use
*
*------------------------------------------------------------------------
*/

int main(int argc, char **argv) {
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad,sad2; /* structure to hold server's address */
	struct sockaddr_in cad; /* structure to hold client's address */
	int participant_sd, observer_sd, sd1,sd2; /* socket descriptors */
	int alen; /* length of address */
	int optval = 1; /* boolean value when we set socket option */
	char buf[1000]; /* buffer for string the server sends */
	uint16_t participant_port;
	uint16_t observer_port;
	int max_participants = 255;
	int max_observers = 255;
	int i = 0;
	int opt = true;
	char yes = 'Y';
	char no = 'N';

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server participant_port observer_port\n");
		exit(EXIT_FAILURE);
	}

	participant_port = atoi(argv[1]); /* convert argument to binary */
	observer_port = atoi(argv[2]); /* convert argument to binary */



  //***************** Participant Setup *******************************

	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */
	sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */


	/* test for illegal value of port listening to participants*/
	if (participant_port > 0) {
		sad.sin_port = htons((u_short)participant_port);
	} else { /* print error message and exit */
		fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}


	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket for participant */
	participant_sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (participant_sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}


	/* Allow reuse of port - avoid "Bind failed" issues */
	if( setsockopt(participant_sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}

	/* Bind a local address to the socket */
	if (bind(participant_sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	/* Specify size of request queue */
	if (listen(participant_sd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}


	//***************** Observer Setup ********************************


	memset((char *)&sad,0,sizeof(sad2)); /* clear sockaddr structure */
	sad2.sin_family = AF_INET; /* set family to Internet */
	sad2.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */


	/* test for illegal value of port listening to observer*/
	if (observer_port > 0) { /* test for illegal value */
		sad2.sin_port = htons((u_short)observer_port);
	} else { /* print error message and exit */
		fprintf(stderr,"Error: Bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket for observer */
	observer_sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (participant_sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}


	/* Allow reuse of port - avoid "Bind failed" issues */
	if( setsockopt(observer_sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}

	/* Bind a local address to the socket */
	if (bind(observer_sd, (struct sockaddr *)&sad2, sizeof(sad2)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	/* Specify size of request queue */
	if (listen(observer_sd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}

	printf("%s\n", "Server is running....");



	//******* Variables for connection ******************************


	int max_sd = 0;
	int sd = 0; /*curr sd */
	int activity; /* return value of select function */
	int active_participant_count = 0; /*active participants */
	int active_observer_count = 0; /* active observer */
	struct a_participant participants[max_participants];
	int participant_sockets[max_participants]; /* all participant sds */
	int observer_sockets[max_observers]; /* all observer sds */
	int valread; /*select function*/
	char*message = "hello";
	//set of socket descriptors
  	fd_set readfds;
  	FD_ZERO(&readfds);
  	int p_index=0;




    //initializing participant and observer sd's to 0 (not connected)
    for(i=0; i< max_participants; i++){
    	participant_sockets[i] = 0;
    }

    for(i=0; i < max_observers; i++){
    	observer_sockets[i] = 0;
    }

    //initiate participant array
    memset(participants, '\0', sizeof(participants));



	//*********** Connection Loop ************************************
	while(1){


		printf("List of usernames: \n");
		for(i=0; i < max_participants;i++){
			if(participants[i].is_active){
				printf("Participant #%d username : %s\n", i, participants[i].username);
			}
		}

		alen = sizeof(cad);
		//printf("%s\n", "waiting for participants and observers...");
		max_sd = 0;

		//clear the socket set
		FD_ZERO(&readfds);

    	//add master sockets to read set
		FD_SET(observer_sd, &readfds);
		FD_SET(participant_sd, &readfds);

		if(observer_sd > participant_sd){
			max_sd = observer_sd;
		}else{
			max_sd = participant_sd;
		}

		//add sockets for participants and observers
		for(i=0; i< max_participants; i++){
			sd = participants[i].sd;
			if(sd > 0 && participants[i].is_active){
				FD_SET(sd,&readfds);
			}
			if(sd > max_sd){
				max_sd = sd;
			}

		}
		for(i=0; i<max_observers; i++){
			sd = observer_sockets[i];
			if(sd > 0){
				FD_SET(observer_sockets[i],&readfds);
			}
			if(sd > max_sd){
				max_sd = sd;
			}

		}



	//wait for an activity on one of the sockets , timeout is NULL , no wait timeout
    activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

    if ((activity < 0)) {
        printf("select error/n");
        exit(EXIT_FAILURE);
    }else{
        	//observer trying to connect
	        if(FD_ISSET(observer_sd,&readfds)){
	        	if((sd1 = accept(observer_sd,(struct sockaddr *)&cad, &alen))<0){
	        		fprintf(stderr, "Error in accepting an observer connection\n");
	        		exit(EXIT_FAILURE);
	        	}

	        	//check if already reached max num of observers
	        	if(active_observer_count == max_observers){
	        		//send 'N' to participant
	        		send(sd1,&no,sizeof(no),0);
	        		close(sd1);
	        	}else{
	        		//send 'Y' to participant
	        		send(sd1,&yes,sizeof(yes),0);
	        		active_observer_count++;
	        		printf("New observer connected\n");
	        	}


	        	//now add this new sd
	        	for(i=0;i<max_observers;i++){
	        		if(observer_sockets[i] <= 0){
	        			observer_sockets[i] = sd1;
	        			printf("Adding new observer fd to sockets\n");
	        			break;
	        		}
	        	}
	        }



		//participant is trying to connect
		if(FD_ISSET(participant_sd,&readfds)){
			if((sd2 = accept(participant_sd,(struct sockaddr *)&sad,(socklen_t*)&alen))< 0){
				fprintf(stderr, "Error accpeting participant connection\n");
				exit(EXIT_FAILURE);
			}
			printf("%s\n", "Participant trying to connect...");
			if(active_participant_count == max_participants){
				send(sd2,&no,sizeof(no),0);
				close(sd2);
				fprintf(stderr, "Reached max num of participants\n");

			}else{
				send(sd2,&yes,sizeof(yes),0);
				active_participant_count++;
				printf("%s\n", "New participant connected");


				//check their username validity before adding sd to list

				int valid_username = 0;
				char username[1000];
				uint8_t username_len;
        int conteo=0;

				while(valid_username != 1){
					conteo ++;
					if(recv(sd2,username, sizeof(username),MSG_WAITALL) <=0){
						fprintf(stderr, "Error receving username\n");
						exit(EXIT_FAILURE);
					}

					valid_username = checkUsername(username,participants);
					fprintf(stderr, "username: %s\n", username);
					fprintf(stderr, "username len: %d\n", username_len);
					printf("checkUsername returns: %d\n", valid_username);

					if(valid_username == 1){
						if(send(sd2,&yes,sizeof(yes),0) <= 0){
							fprintf(stderr, "Error replying Y to username\n");
							exit(EXIT_FAILURE);
						}
					}else{

						printf("Sending participant 'N'\n");
						if(send(sd2,&no,sizeof(no),0) <= 0){
							fprintf(stderr, "Error replying N to username\n");
							exit(EXIT_FAILURE);
						}

					}
				}

				//create a new participant to add into array
				struct a_participant new_participant;
				new_participant.sd = sd2;
				strcpy(new_participant.username,username);
				new_participant.is_active = true;
				participants[p_index] = new_participant;
				p_index++;


				//once valid add their sd to socket list
				for(i=0; sizeof(participants); i++){
					if(participant_sockets[i] == 0){
						participant_sockets[i] = sd2;
						fprintf(stderr, "Adding participant %d\n", participant_sockets[i]);
						fprintf(stderr, "With username: %s\n", participants[p_index-1].username);
						break;
					}
				}

		}
	}
	//participant trying to close connection


	}//activity loop




    }//end while
    return 0;
	}//end main



//validate username
int checkUsername(char* username, struct a_participant participants[]){
	int username_index = 0;
	char l = username[username_index];
	char name[11];
	int i;
	int len = strlen(username);

	if(len > 10 || len < 1){
		return 0;
	}

	for(i = 0; i<sizeof(participants); i++){
		strcpy(name, participants[i].username);
		//if this username has already been used in this game, return 0
		if(strcmp(name, username)==0)
		return 0;
	}

	while(username[username_index]!='\0' && username_index <= strlen(username)){
		l = username[username_index];
		//if the letter is not a number
		if(!(48<=l && 57>= l)){
			//and if the letter is not a capitol letter
			if(!(65<=l && 90>= l)){
				//and if the letter is not a lowercase letter
				if(!(97<= l && 122 >= l)){
					//and if the letter is not a whitespace or underscore
					if(!(l == 32 || l == 95))
					return 0;
				}
			}
		}
		username_index++;
	}

	//the username is valid return 1

	//printf("username %s is valid.\n", username);
	return 1;
}
