#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>

#define BUFLEN 256


typedef struct entries {
	double sold;
	char   first_name[12];
	char   last_name[12];
	char   password[8];
	int    failed_logins;
	int    card_number;
	int    locked;
	int    pin;
} Entry;

typedef struct session {
	Entry* entry;
	int    pid;
} Session;
	
void showEntry(Entry e) {
	printf("%s %s %d %d %s %.2lf\n",
			e.last_name,
			e.first_name,
			e.card_number,
			e.pin,
			e.password,
			e.sold);
}

void error(char *msg) {
	perror(msg);
	exit(-1);
}

void checkServerPort(char const *server_port_str, int *server_port) {
	
	int i, port_number = 0; 
	for (i = 0; i < strlen(server_port_str); i++) {
		int char_as_int = server_port_str[i];
		if (char_as_int < 48 || char_as_int > 57) {
			error("[Server]: Server port is invalid!");
		} else {
			port_number = port_number * 10 + (char_as_int - '0');
		}
	}
	*server_port = port_number;
}

void checkServerData(char const *server_datafile_name, FILE **user_fd) {
	/* This can be uncommented to only accept a namefile on input. */
	// if (strcmp(server_datafile_name, "users_data_file") != 0)
	// 	error("[Server]: Wrong file input!");
	(*user_fd) = fopen(server_datafile_name, "r");
	if ((*user_fd) == NULL) 
		error("[Server]: Error on opening input file.");
}
/* Reads all entries for all the users from file. */
Entry *getUserData(FILE *users_file, int entries) {
	/*
    indx: 0      1         2            3     4        5
 	      <nume> <prenume> <numar_card> <pin> <parola> <sold>
    max:  12chr  12chr     6chr         4chr  8chr     double
	*/
	Entry *userData = NULL;
	userData = (Entry *) malloc (entries * sizeof(Entry));

	int i;
	char buff[255];
	char aux_card[6];
	char aux_pin[4];
	for (i = 0; i < entries; i++) {
		
		fscanf(users_file, "%s", buff);
		strcpy(userData[i].last_name, buff);
		memset(buff, 0, sizeof(buff));
		
		fscanf(users_file, "%s", buff);
		strcpy(userData[i].first_name, buff);
		memset(buff, 0, sizeof(buff));
		
		fscanf(users_file, "%s", buff);
		strcpy(aux_card, buff);
		userData[i].card_number = atoi(aux_card);
		memset(buff, 0, sizeof(buff));
		memset(aux_card, 0, sizeof(aux_card));

		fscanf(users_file, "%s", buff);
		strcpy(aux_pin, buff);
		userData[i].pin = atoi(aux_pin);
		memset(buff, 0, sizeof(buff));
		memset(aux_pin, 0, sizeof(aux_pin));

		fscanf(users_file, "%s", buff);
		strcpy(userData[i].password, buff);
		memset(buff, 0, sizeof(buff));

		fscanf(users_file, "%s", buff);
		sscanf(buff, "%lf", &(userData[i].sold));
		memset(buff, 0, sizeof(buff));

		userData[i].locked = 0;
		userData[i].failed_logins = 0;
	}

	return userData;
}
/* Gets the pointer to an entry. */
Entry *getEntry(Entry *userData, int entries, Entry e) {
	int i;
	for (i = 0; i < entries; i++) {
		if (e.card_number == (userData[i]).card_number) {
			return &(userData[i]);
		}
	}
	return NULL;
}
/* Gets user from user data given the searched card number; stores the entry in 'user'. */
int getUserByCardNumber(Entry *userData, int entries, int searched_card, Entry* user) {

	int i;
	for (i = 0; i < entries; i++) {
		if (userData[i].card_number == searched_card) {
			(*user) = userData[i];
			return 1;
		}
	}
	return 0;
}
/* Returns the entry from the existing sessions based on the given client PID. */
Entry *getSessionEntry(Session* sessions, int active_sessions, int session_pid) {
	int i; 
	for (i = 0; i < active_sessions; i++) {
		if (sessions[i].pid == session_pid) {
			return sessions[i].entry;
		}
	}
	return NULL;
}
/* Returns 1 for already existing Session and 0 otherwise. */
int activeSession(Session* sessions, int active_sessions, int session_pid, int cardNumber) {
	int i;
	for (i = 0; i < active_sessions; i++) {
		if (sessions[i].pid == session_pid || sessions[i].entry->card_number == cardNumber){
			return 1;
		}
	}
	return 0;
}
/* Checks if PID is active in any session. */
int activePID(Session* sessions, int active_sessions, int session_pid) {
	int i;
	for (i = 0; i < active_sessions; i++) {
		if (sessions[i].pid == session_pid){
			printf("This session already exists!\n");
			return 1;
		}
	}
	return 0;
}
/* Adds new Sessions with its own Entry and a PID. */
void addSession(Session** sessions, Entry *userData, int entries, int* active_sessions, Entry new_entry ,int session_pid) {
	if ((*active_sessions) >= entries) {
		printf("Active sessions: %d\n", (*active_sessions));
		perror("[Server]: Too many sessions opened!");
	} else {
		(*sessions)[(*active_sessions)].pid = session_pid;
		(*sessions)[(*active_sessions)].entry = getEntry(userData, entries, new_entry);
		(*active_sessions)++;
		printf("Added new session! Active sessions: %d\n", (*active_sessions));
	}
}
/* Removes session given the client PID. */
void rmvSession(Session** sessions, int entries, int* active_sessions, int session_pid) {
	if ((*active_sessions) == 0) {
		perror("[Server]: There are no active sessions to close!");
	} else {
		int i, aux, found = 0;
		for (i = 0; i < (*active_sessions); i++) {
			if (((*sessions)[i]).pid == session_pid){
				aux = i;
				found = 1;
				break;
			}
		}
		if (found) {
			for (i = aux; i < (*active_sessions); i++) {
				(*sessions)[i] = (*sessions)[i + 1];
			}
		} else {
			perror("[Server]: Session not active.");
		}
		(*active_sessions)--;
		printf("Active sessions: %d\n", (*active_sessions));
	}
}

int main(int argc, char const *argv[]) {	

	if (argc != 3) {
		error("[Server]: Server port or user data file is missing!");
	}
	/* Setting database for the server. */
// ------------------------------------------------------------------ //
	int server_port;
	checkServerPort(argv[1], &server_port);
	FILE *users_file;
	checkServerData(argv[2], &users_file);
	// Getting number of entries...
	if (feof(users_file)) 
		error("[Server]: Error on reading entries number.");

	int entries = (int) (fgetc(users_file) - '0');
	Entry *userData = getUserData(users_file, entries);
	int session_pid;
	int active_sessions = 0;
	/* 
	Making room for the maximum number of sessions.
	Maximum number of sessions equals the numbers of entries.
	*/
	Session* sessions = (Session *) malloc (entries * sizeof(Session));

	/* Initialising connection */
// ------------------------------------------------------------------ //
	struct sockaddr_in client_addr;
	struct sockaddr_in serv_addr;
	unsigned int client_len;
	char buffer[BUFLEN];
	int newsockfd;
	int sockfd;
	int n;

	fd_set read_fds;	
	fd_set tmp_fds;	 
	int fdmax;

	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	/* Opening TCP socket. */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("[Server]: Error on opening socket.");
	
	/* Complete info about server address. */
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(server_port);

	/* Binding */
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
		error("[Server]: Error on binding.");

	int max_clients = entries; 
	listen(sockfd, max_clients);
	FD_SET(sockfd, &read_fds);
	fdmax = sockfd;
// ------------------------------------------------------------------ //
	/* Data required for the transfer money command.*/
// ------------------------------------------------------------------ //
	int pending_transfer; // Is any transfer running ? [1/0]
	int transfer_pid;	  // The PID of the client who wants to transfer money
	int card_reciever;	  // Card number of the client who will get the money
	double money_amount;  // The money amount that is transported
// ------------------------------------------------------------------ //
	/* 
	The infinite loop in which the server gets messages from clients and 
	sends responses back.
	*/
// ------------------------------------------------------------------ //
	while (1) {
		tmp_fds = read_fds;
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1) {
			error("[Server]: Error on select.");
		}
		int i;
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd) {
					/* New connection established. Server will now accept. */
					client_len = sizeof(client_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len)) == -1){ 
						error("[Server]: Error on accept.");
					} else {	
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax) 
							fdmax = newsockfd;
					}
					printf("New connect from: %s, port: %d, socket_client: %d\n ", inet_ntoa(client_addr.sin_addr),
					                                                               ntohs(client_addr.sin_port),
					                                                               newsockfd);
				} else {
					/* Recieving message on a socket => recv(). */
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
						if (n == 0) {
							/* Connection has been terminated. */
							printf("[Server]: Socket %d hung up.\n", i);
							rmvSession(&sessions, entries, &active_sessions, session_pid);
						} else {
							error("[Server]: Error in recv");
						}
						/* Close and remove socket. */
						close(i); 
						FD_CLR(i, &read_fds);
					} 
					
					else { 
																		
						char msg_error[] = "[Server]: Client not connected.\n";
						char command[40];
						Entry e;
						int valid_comand = 0;
						sscanf(buffer, "%d %s", &session_pid, command);
						/* In case of quit command from client. */
						if (strcmp(command, "quit") == 0) {
							if (pending_transfer && transfer_pid == session_pid) {
								pending_transfer = 0; 
							}
							rmvSession(&sessions, entries, &active_sessions, session_pid);
							close(i); 
							FD_CLR(i, &read_fds);
						}
						/* In case of waiting for [y/n] response from client for transferring money. */
						if (pending_transfer && transfer_pid == session_pid) {
							valid_comand = 1;
							if (strcmp(command, "y") == 0) {
								memset(buffer, 0, BUFLEN);
								strcpy(buffer, "IBANK> Transfer realizat cu succes");
								memset(command, 0, 40);
								getUserByCardNumber(userData, entries, card_reciever, &e);
								(getEntry(userData, entries, e))->sold =
								    (getEntry(userData, entries, e))->sold + money_amount;

								e = *getSessionEntry(sessions, active_sessions, session_pid);
								(getEntry(userData, entries, e))->sold =
								    (getEntry(userData, entries, e))->sold - money_amount;

							} else {
								memset(buffer, 0, BUFLEN);
								strcpy(buffer, "IBANK> -9 : Operatie anulata");
								memset(command, 0, 40);
							}
								pending_transfer = 0;
						}
						/* In case of login command from client. */
						if (strcmp(command, "login") == 0) {
							valid_comand = 1;
							int cardNumber, pin;
							sscanf(buffer, "%d %s %d %d", &session_pid, command, &cardNumber, &pin);
							int can_add;
							if (activeSession(sessions, active_sessions, session_pid, cardNumber)) {
								memset(buffer, 0, BUFLEN);
								strcpy(buffer, "IBANK> -2 : Sesiune deja deschisa");
								can_add = 0;
							} else {
								can_add = 1;
							}
							if (can_add) {
								if (getUserByCardNumber(userData, entries, cardNumber, &e)) {
									if (e.pin == pin) {
										addSession(&sessions, userData, entries, &active_sessions, e, session_pid);
										memset(buffer, 0, BUFLEN);
										strcpy(buffer, "IBANK> Welcome ");
										strcpy(buffer + strlen(buffer), e.last_name);
										strcpy(buffer + strlen(buffer), " ");
										strcpy(buffer + strlen(buffer), e.first_name);
									} else {
										memset(buffer, 0, BUFLEN);
										(getEntry(userData, entries, e))->failed_logins++;
										if ((getEntry(userData, entries, e))->failed_logins >= 3) {
											(getEntry(userData, entries, e))->locked = 1;
											strcpy(buffer, "IBANK> -5 : Card blocat");
										} else {
											strcpy(buffer, "IBANK> -3 : Pin gresit");
										}
									}
								} else {
									memset(buffer, 0, BUFLEN);
									strcpy(buffer, "IBANK> -4 : Numar card inexistent");								
								}
							}
						}
						/* In case of logout command from client. */
						int logout_client = 0;
						if (strcmp(command, "logout") == 0) {
							valid_comand = 1;
							memset(buffer, 0, BUFLEN);
							if (!activePID(sessions, active_sessions, session_pid)) {
								strcpy(buffer, " -1 : Clientul nu este autentificat");
							} else {
								strcpy(buffer, "IBANK> Clientul a fost deconectat");
								logout_client = 1;
							}
						}
						/* In case of listsold command from client. */
						if (strcmp(command, "listsold") == 0) {
							valid_comand = 1;
							memset(buffer, 0, BUFLEN);
							if (!activePID(sessions, active_sessions, session_pid)) {
								strcpy(buffer, " -1 : Clientul nu este autentificat");
							} else {
								strcpy(buffer, "IBANK> ");
								double req_sold = getSessionEntry(sessions, active_sessions, session_pid)->sold;
								snprintf(buffer + 7, BUFLEN - 7, "%.2lf", req_sold);
							}
						}
						/* In case of unlock command from client. */
						if (strcmp(command, "unlock") == 0) {
							valid_comand = 1;
							int card_no;
							sscanf(buffer, "%d %s %d\n", &session_pid, command, &card_no);
							memset(buffer, 0, BUFLEN);
							if (getUserByCardNumber(userData, entries, card_no, &e)) {
								(getEntry(userData, entries, e))->failed_logins = 0;
								(getEntry(userData, entries, e))->locked = 0;
								strcpy(buffer, "UNLOCK> Card deblocat");
							} else {
								strcpy(buffer, "UNLOCK> -4 : Numar card inexistent");
							}
						}
						/* In case of quit command from client. */
						if (strcmp(command, "transfer") == 0) {
							valid_comand = 1;
							int card_sendto;
							sscanf(buffer, "%d %s %d %lf", &session_pid, command, &card_sendto, &money_amount);
							memset(buffer, 0, BUFLEN);
							if (!activePID(sessions, active_sessions, session_pid)) {
								strcpy(buffer, " -1 : Clientul nu este autentificat");
							} else {
								if (money_amount > getSessionEntry(sessions, active_sessions, session_pid)->sold) {
									strcpy(buffer, "IBANK> -8 : Fonduri insuficiente");
								} else {
									if (getUserByCardNumber(userData, entries, card_sendto, &e)) {

										strcpy(buffer, "IBANK> Transfer ");
										char aux_money[50];
										memset(aux_money, 0, 50);
										snprintf(aux_money, 50, "%.2lf", money_amount);
										strcpy(buffer + strlen(buffer), aux_money);
										strcpy(buffer + strlen(buffer), " catre ");
										strcpy(buffer + strlen(buffer), e.last_name);
										strcpy(buffer + strlen(buffer), " ");
										strcpy(buffer + strlen(buffer), e.first_name);
										strcpy(buffer + strlen(buffer), "? [y/n]");
										pending_transfer = 1;
										transfer_pid = session_pid;
										card_reciever = card_sendto;
									} else {
										strcpy(buffer, "IBANK> -4 : Numar card inexistent");
									}

								}
							}
						}
						/* Test if the input from client was any of the above cases. */
						if (!valid_comand) {
							memset(buffer, 0, BUFLEN);
							strcpy(buffer, " -6 : Operatie esuata");
						}
						if (FD_ISSET(i, &read_fds) != 0) {
							send(i, buffer, strlen(buffer), 0);
							puts(buffer);
						}
						else {
							send(i, msg_error, strlen(msg_error), 0);
							puts(msg_error);
						}
						if (logout_client) {
							rmvSession(&sessions, entries, &active_sessions, session_pid);							
						}
					}
				}
			}
		}
 	}
// ------------------------------------------------------------------ //

	close(sockfd);

	return 0;
}
