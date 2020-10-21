#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include "socket.h"

#ifndef PORT
    #define PORT 59998
#endif

#define LISTEN_SIZE 5
#define WELCOME_MSG "Welcome to CSC209 Twitter! Enter your username: "
#define SEND_MSG "send"
#define SHOW_MSG "show"
#define FOLLOW_MSG "follow"
#define UNFOLLOW_MSG "unfollow"
#define QUIT_MEG "quit"
#define ANNOUNCEMENT " has just joined.\r\n" 
#define BUF_SIZE 256
#define MSG_LIMIT 8
#define FOLLOW_LIMIT 5
#define DUPLICATED_USER_MSG1 "Username cannot be empty. Please try again: "
#define DUPLICATED_USER_MSG2 "This username has been used. Please try again: "
#define LONG_NAME "Your username is too long. Please select a short name.\r\n"
#define INVALID_COMMAND "Invalid command.\r\n"
#define EMPTY_FOLLOW "You can't follow an empty user.\n"
#define TOO_LONG_MESSAGE "Your message is longer than 140 characters.\r\n"
#define Unfollow_UNFOLLOWED "You have not followed this user.The server ingored your request.\r\n"
struct client {
    int fd;
	int msg_index;
    struct in_addr ipaddr;
    char username[BUF_SIZE];
    char message[MSG_LIMIT][BUF_SIZE];
    struct client *following[FOLLOW_LIMIT]; // Clients this user is following
    struct client *followers[FOLLOW_LIMIT]; // Clients who follow this user
    char inbuf[BUF_SIZE]; // Used to hold input from the client
    char *in_ptr; // A pointer into inbuf to help with partial reads
    struct client *next;
};

// Provided functions. 
void add_client(struct client **clients, int fd, struct in_addr addr);
void remove_client(struct client **clients, int fd);

// These are some of the function prototypes that we used in our solution 
// You are not required to write functions that match these prototypes, but
// you may find them helpful when thinking about operations in your program.

// Send the message in s to all clients in active_clients. 
void announce(struct client *active_clients, char *s){
	struct client *p;
    for (p = active_clients; p != NULL; p = p->next) {
        if (write(p->fd, s, strlen(s)) == -1) {
            fprintf(stderr, 
                "Write to client %s failed\n", inet_ntoa(p->ipaddr));
            remove_client(&p, p->fd);
        }
    }
}

// Move client c from new_clients list to active_clients list. 
void activate_client(struct client *c, 
    struct client **active_clients_ptr, struct client **new_clients_ptr){
		struct client *p;
		if ((*new_clients_ptr)->fd == c->fd){
			*new_clients_ptr = c->next;
		}else{
		    for (p = *new_clients_ptr; p != NULL; p = p->next) {
		        if (p->next == c) {
		            p->next = c->next;
		        }
		    }
		}
		
    	c->next = *active_clients_ptr;
		*active_clients_ptr = c;
    }
//find new line sign
int find_network_newline(const char *buf, int n) {
    int i;
    for(i = 0; i < n; i++){
        if(buf[i] == '\r' && buf[i+1] == '\n'){
            return (i+2);
        }
    }
    return -1;
}


// The set of socket descriptors for select to monitor.
// This is a global variable because we need to remove socket descriptors
// from allset when a write to a socket fails. 
fd_set allset;

/* 
 * Create a new client, initialize it, and add it to the head of the linked
 * list.
 */
void add_client(struct client **clients, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));
    p->fd = fd;
    p->ipaddr = addr;
    p->username[0] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *clients;
	p->msg_index = 0;

    // initialize messages to empty strings
    for (int i = 0; i < MSG_LIMIT; i++) {
        p->message[i][0] = '\0';
    }
	for (int i = 0; i < FOLLOW_LIMIT; i++){
		p->followers[i] = NULL;
		p->following[i] = NULL;
	}
	

    *clients = p;
}

/* 
 * Remove client from the linked list and close its socket.
 * Also, remove socket descriptor from allset.
 */
void remove_client(struct client **clients, int fd) {
    struct client **p;

    for (p = clients; *p && (*p)->fd != fd; p = &(*p)->next)
        ;

    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        // TODO: Remove the client from other clients' following/followers
        // lists
		for (int i = 0; i < FOLLOW_LIMIT; i++){
			if ((*p)->followers[i] != NULL){
				for (int j = 0; j < FOLLOW_LIMIT; j++){
					if ((*p)->followers[i]->following[j] != NULL){
						if (strcmp((*p)->followers[i]->following[j]->username, (*p)->username) == 0){
							(*p)->followers[i]->following[j] = NULL;
						}
					}
				}
			}
			if ((*p)->following[i] != NULL){
				for (int j = 0; j < FOLLOW_LIMIT; j++){
					if ((*p)->following[i]->followers[j] != NULL){
						if (strcmp((*p)->following[i]->followers[j]->username, (*p)->username) == 0){
							(*p)->following[i]->followers[j] = NULL;
						}
					}
				}
			}
		}

        // Remove the client
        struct client *t = (*p)->next;
	char *announcement = malloc(sizeof(char)*(BUF_SIZE + 18));
	if(announcement == NULL){
		perror("malloc");
		exit(1);
	}
	strcpy(announcement, "");
	snprintf(announcement, sizeof(char)*(BUF_SIZE + 18),"Goodbye %s\r\n",(*p)->username);
	announce(*clients, announcement);
	free(announcement);
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;
    } else {
        fprintf(stderr, 
            "Trying to remove fd %d, but I don't know about it\n", fd);
    }
}


int main (int argc, char **argv) {
    int clientfd, maxfd, nready;
    struct client *p, *t, *s;
    struct sockaddr_in q;
    fd_set rset;

    // If the server writes to a socket that has been closed, the SIGPIPE
    // signal is sent and the process is terminated. To prevent the server
    // from terminating, ignore the SIGPIPE signal. 
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // A list of active clients (who have already entered their names). 
    struct client *active_clients = NULL;

    // A list of clients who have not yet entered their names. This list is
    // kept separate from the list of active clients, because until a client
    // has entered their name, they should not issue commands or 
    // or receive announcements. 
    struct client *new_clients = NULL;

    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, LISTEN_SIZE);
	free(server);

    // Initialize allset and add listenfd to the set of file descriptors
    // passed into select 
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;

        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            exit(1);
        } else if (nready == 0) {
            continue;
        }

        // check if a new client is connecting
        if (FD_ISSET(listenfd, &rset)) {
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd, &q);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
			
            printf("Connection from %s\n", inet_ntoa(q.sin_addr));
            add_client(&new_clients, clientfd, q.sin_addr);
            char *greeting = WELCOME_MSG;
            if (write(clientfd, greeting, strlen(greeting)) == -1) {
                fprintf(stderr, 
                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
                remove_client(&new_clients, clientfd);
            }
        }

        // Check which other socket descriptors have something ready to read.
        // The reason we iterate over the rset descriptors at the top level and
        // search through the two lists of clients each time is that it is
        // possible that a client will be removed in the middle of one of the
        // operations. This is also why we call break after handling the input.
        // If a client has been removed, the loop variables may no longer be 
        // valid.
        int cur_fd, handled;
        for (cur_fd = 0; cur_fd <= maxfd; cur_fd++) {
            if (FD_ISSET(cur_fd, &rset)) {
                handled = 0;

                // Check if any new clients are entering their names
                for (p = new_clients; p != NULL; p = p->next) {
                    if (cur_fd == p->fd) {
                        // TODO: handle input from a new client who has not yet 
                        // entered an acceptable name
						
						//printf("Read username from the client\n");
						//read the username from the new_client
						handled = 1;
						bool username_acceptable = true;
						char *inbuf = malloc(sizeof(char)*(BUF_SIZE));
						if(inbuf == NULL){
							perror("malloc");
							exit(1);
						}
						int sig = read( cur_fd , inbuf, BUF_SIZE);
						if (sig == -1 || sig == 0){
			                fprintf(stderr,
			                    "Read from client %s failed\n", inet_ntoa(q.sin_addr));
							remove_client(&new_clients, cur_fd);
							free(inbuf);
							break;
						}
						inbuf[sig] = '\0';
						int len;
						len = strlen(inbuf);
						for (int i = 0; i < len; i++){
								if( inbuf[i] == '\n'){
									inbuf[i] = '\0';
								}
								if( inbuf[i] == '\r'){
									inbuf[i] = '\0';
								}
							}
						if (len > BUF_SIZE){
							char *msg=LONG_NAME;
				            if (write(cur_fd, msg, strlen(msg)) == -1) {
				                fprintf(stderr,
				                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
								remove_client(&new_clients, cur_fd);
								free(inbuf);
								break;
				            }
						free(inbuf);
						break;
						}
						//printf("Check username\n");
						// check if it is an acceptable name
						if (strcmp(inbuf, "") == 0){
							printf("Empty username.\n");
				            username_acceptable = false;
							char *unacceptable_username = DUPLICATED_USER_MSG1;
				            if (write(cur_fd, unacceptable_username, strlen(unacceptable_username)) == -1) {
				                fprintf(stderr,
				                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
								remove_client(&new_clients, cur_fd);
								free(inbuf);
								break;
				            }
						}else{
				            for (t = active_clients; t != NULL; t = t->next) {
				                if (strcmp(inbuf, t->username) == 0) {
				                    username_acceptable = false;
				                    break;
				                }
				            }

							if (username_acceptable){
								char *announcement = malloc(sizeof(char)*(BUF_SIZE + 18));
								strcpy(p->username, inbuf);
								strcpy (announcement, "");
								snprintf(announcement, sizeof(char)*(BUF_SIZE + 18), "%s has just joined.\n", p->username);
								activate_client(p, &active_clients, &new_clients);
								announce(active_clients, announcement);
								free(announcement);
								printf("Username successful.\n");
							}else{
								printf("Duplicated user.\n");
								char *unacceptable_username = DUPLICATED_USER_MSG2;
					            if (write(cur_fd, unacceptable_username, strlen(unacceptable_username)) == -1) {
					                fprintf(stderr,
					                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
					                remove_client(&new_clients, cur_fd);
									free(inbuf);
									break;
					            }
							}
						}
						free(inbuf);
                        break;
                    }
                }

                if (!handled) {
                    // Check if this socket descriptor is an active client
                    for (p = active_clients; p != NULL; p = p->next) {
                        if (cur_fd == p->fd) {
                            // TODO: handle input from an active client
							
							//read command from user input
							char *inbuf = malloc(sizeof(char)*(BUF_SIZE));
							if(inbuf == NULL){
								perror("malloc");
								exit(1);
							}
							bool show = false, quit = false;
							int sig = read( cur_fd , inbuf, BUF_SIZE);
							if (sig == -1 || sig == 0){
				                fprintf(stderr,
				                    "Read from client %s failed\n", inet_ntoa(q.sin_addr));
								remove_client(&active_clients, cur_fd);
								free(inbuf);
								break;
							}
							inbuf[sig] = '\0';
							int len;
							len = strlen(inbuf);
							if (len == 0){
								free(inbuf);
									break;
					            //}
								break;
							}
							for (int i = 0; i < len; i++){
								if( inbuf[i] == '\n'){
									inbuf[i] = '\0';
								}
								if( inbuf[i] == '\r'){
									inbuf[i] = '\0';
								}
							}
							//printf("%s\n",inbuf);
							
							len = strlen(inbuf);
							if (len == 0){
								free(inbuf);
								break;
							}
							
							//printf("Check if it is show or quit command.\n");
							if (strcmp(SHOW_MSG, inbuf) == 0){
								show = true;
							}else if(strcmp(QUIT_MEG, inbuf) == 0){
								quit = true;
							}
							char *ptr = strtok(inbuf, " ");
							//printf("Check command complete...\n");
						
							//follow command
							if (strcmp(ptr, FOLLOW_MSG) == 0){ 
								printf("follow command...\n");
								bool is_active_user = false, has_followed = false, following_limit_reached = true, followed_limit_reached = true;
								ptr = strtok(NULL, " ");
								if (ptr == NULL){
									char *follow_empty = EMPTY_FOLLOW;
						            if (write(cur_fd, follow_empty, strlen(follow_empty)) == -1) {
						                fprintf(stderr,
						                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
						                remove_client(&active_clients, cur_fd);
										break;
						            }
								}else{// Follow self is illegal
									if (strcmp(ptr, p->username) == 0){
							            if (write(cur_fd, "You cannot follow yourself. The server ingored your request.\n", strlen("You cannot follow yourself. The server ingored your request.\n")) == -1) {
							                fprintf(stderr,
							                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
							                remove_client(&active_clients, cur_fd);
											break;
							            }
									}else{//chech if the user is in the active_user list
										for (t = active_clients; t != NULL; t = t->next) {
											if (strcmp(ptr, t->username) == 0){
												s = t;
												is_active_user = true;
												//check if the follower limit has reached
												for (int i = 0; i < FOLLOW_LIMIT; i++){
													if (t->followers[i] == NULL){
														followed_limit_reached = false;
													}else{
														if (strcmp(p->username, t->followers[i]->username) == 0){
															if (!has_followed){
																has_followed = true;
													            if (write(cur_fd, "You have already followed the user. The server ingored your request.\n", strlen("You have already followed the user. The server ingored your request.\n")) == -1) {
													                fprintf(stderr,
													                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
													                remove_client(&active_clients, cur_fd);
																	break;
													            }
															}
														}
													}
												}
											}
										}
										if (!is_active_user){
								            if (write(cur_fd, "The user is not in the active user list. Please try another username.\n", strlen("The user is not in the active user list. Please try another username.\n")) == -1) {
								                fprintf(stderr,
								                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
								                remove_client(&active_clients, cur_fd);
												break;
								            }
										}else if (followed_limit_reached){// reach the follow limit
								            if (write(cur_fd, "The user has reached his/her followers limits. You cannot follow him/her for now.\n", strlen("The user has reached his/her followers limits. You cannot follow him/her for now.\n")) == -1) {
								                fprintf(stderr,
								                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
								                remove_client(&active_clients, cur_fd);
												break;
								            }
										}else if (!has_followed){// unfollow a client which hasn't been followed
											for (int i = 0; i < FOLLOW_LIMIT; i++){
												if (p->following[i] == NULL){
													following_limit_reached = false;
													p->following[i] = s;
													break;
												}
											}
											if (following_limit_reached){// reach follow limit
									            if (write(cur_fd, "You has reached your followers limits. You cannot follow others for now.\n", strlen("You has reached your followers limits. You cannot follow others for now.\n")) == -1) {
									                fprintf(stderr,
									                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
									                remove_client(&active_clients, cur_fd);
													break;
									            }
											}else{// Successfully follow the client
												for (int i = 0; i < FOLLOW_LIMIT; i++){
													if (s->followers[i] == NULL){
														s->followers[i] = p;
														char *msg = malloc(sizeof(char)*(BUF_SIZE + 30));
														if(msg == NULL){
															perror("malloc");
															exit(1);
														}
														strcpy (msg, "");
														snprintf(msg, sizeof(char)*(BUF_SIZE + 30), "You have followed user: %s\n", s->username);
											            if (write(cur_fd, msg, strlen(msg)) == -1) {
											                fprintf(stderr,
											                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
											                remove_client(&active_clients, cur_fd);
															break;
											            }
														free(msg);
														break;
													}
												}
											}
										}
									}
								}
							// unfollow command	
							}else if(strcmp(ptr, UNFOLLOW_MSG) == 0){ 
								printf("unfollow command...\n");
								bool is_active_user = false, had_followed = false;
								ptr = strtok(NULL, " ");
								if (ptr == NULL){
						            if (write(cur_fd, "You cannot unfollow an empty username. The server ingored your request.\n", strlen("You cannot unfollow an empty username. The server ingored your request.\n")) == -1) {
						                fprintf(stderr,
						                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
						                remove_client(&active_clients, cur_fd);
										break;
						            }
								}else{
									if (strcmp(ptr, p->username) == 0){
							            if (write(cur_fd, "You cannot unfollow yourself. The server ingored your request.\n", strlen("You cannot unfollow yourself. The server ingored your request.\n")) == -1) {
							                fprintf(stderr,
							                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
							                remove_client(&active_clients, cur_fd);
											break;
							            }
									}else{
										for (t = active_clients; t != NULL; t = t->next) {
											//chech if the user is in the active_user list
											if (strcmp(ptr, t->username) == 0){
												s = t;
												is_active_user = true;
											}
										}
										if (!is_active_user){
								            if (write(cur_fd, "The user is not in the active user list. Please try another username.\n", strlen("The user is not in the active user list. Please try another username.\n")) == -1) {
								                fprintf(stderr,
								                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
								                remove_client(&active_clients, cur_fd);
												break;
								            }
										}else {
											for (int i = 0; i < FOLLOW_LIMIT; i++){
												if (p->following[i] != NULL){
													if (strcmp(p->following[i]->username, ptr)==0){
														had_followed = true;
														p->following[i] = NULL;
														break;
													}
												}
											}

											if (had_followed){
												for (int i = 0; i < FOLLOW_LIMIT; i++){
													if (s->followers[i] != NULL){
														if (strcmp(s->followers[i]->username, p->username)==0){
															s->followers[i] = NULL;
												            if (write(cur_fd, "You are not following this user any more.\n", strlen("You are not following this user any more.\n")) == -1) {
												                fprintf(stderr,
												                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
												                remove_client(&active_clients, cur_fd);
																break;
												            }
															break;
														}
													}
												}
											}else{
												char *unfollow_unfollowed = Unfollow_UNFOLLOWED;
									            if (write(cur_fd, unfollow_unfollowed, strlen(unfollow_unfollowed)) == -1) {
									                fprintf(stderr,
									                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
									                remove_client(&active_clients, cur_fd);
													break;
									            }
											}
										}
									}
								}
							}else if(show){ // show command
								printf("show...\n");
								for (int i = 0; i < FOLLOW_LIMIT; i++){
									if (p->following[i]!=NULL){
										char *msg = malloc(sizeof(char)*(BUF_SIZE+20));
										if(msg ==NULL){
											perror("malloc");
											exit(1);
										}
										strcpy (msg, "");
										//snprintf(msg, sizeof(char)*(BUF_SIZE+1), "%s\n", p->following[i]->username);
							            if (write(cur_fd, msg, strlen(msg)) == -1) {
							                fprintf(stderr,
							                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
							                remove_client(&active_clients, cur_fd);
											break;
							            }
										for (int j = 0; j < MSG_LIMIT; j++){
											if(strlen( p->following[i]->message[j]) > 0) {
												strcpy (msg, "");
												snprintf(msg, sizeof(char)*(BUF_SIZE+20), "%s wrote: %s\n",p->following[i]->username, p->following[i]->message[j]);
									            if (write(cur_fd, msg, strlen(msg)) == -1) {
									                fprintf(stderr,
									                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
									                remove_client(&active_clients, cur_fd);
													break;
									            }
											}
										}
										free(msg);
									}
								}
							}else if(strcmp(ptr, SEND_MSG) == 0){ // send command
								printf("send command...\n");
								char *msg = malloc(sizeof(char)*(BUF_SIZE));
								if(msg == NULL){
											perror("malloc");
											exit(1);
										}
								char *msg_user = malloc(sizeof(char)*(BUF_SIZE));
								if(msg_user ==NULL){
											perror("malloc");
											exit(1);
										}
								bool msg_too_long = false;
								int msg_len = 0;
								ptr = strtok(NULL, " ");
								strcpy (msg, "");
								while(ptr != NULL)
								{
									strcat(msg, ptr);
									msg_len += strlen(ptr);
									strcat(msg, " ");
									msg_len += strlen(ptr)+1;
									if (msg_len > BUF_SIZE){
										msg_too_long = true;
										break;
									}
									ptr = strtok(NULL, " ");
								}
								if (msg_too_long){// too long message
									char *long_message = TOO_LONG_MESSAGE;
									if(long_message ==NULL){
											perror("malloc");
											exit(1);
										}
						            if (write(cur_fd, long_message, strlen(long_message)) == -1) {
						                fprintf(stderr,
						                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
						                remove_client(&active_clients, cur_fd);
										break;
						            }
								}else{
									strcpy(p->message[p->msg_index++],msg);
									if (p->msg_index == MSG_LIMIT){
										p->msg_index = 0;
									}
									for (int i = 0; i < FOLLOW_LIMIT; i++){
										if (p->followers[i] != NULL){
								            if (write(p->followers[i]->fd, p->username, strlen(p->username)) == -1) {
								                fprintf(stderr,
								                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
								                remove_client(&active_clients, cur_fd);
												break;
								            }
								            if (write(p->followers[i]->fd, " says:", strlen("says: ")) == -1) {
								                fprintf(stderr,
								                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
								                remove_client(&active_clients, cur_fd);
												break;
								            }
								            if (write(p->followers[i]->fd, msg, strlen(msg)) == -1) {
								                fprintf(stderr,
								                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
								                remove_client(&active_clients, cur_fd);
												break;
								            }
								            if (write(p->followers[i]->fd, "\n", strlen("\n")) == -1) {
								                fprintf(stderr,
								                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
								                remove_client(&active_clients, cur_fd);
												break;
								            }
										}
									}
								}
								free(msg);
								free(msg_user);
							}else if(quit){ // quit command
								//printf("Quit command...\n");
								remove_client(&active_clients, cur_fd);
							}else{
								//printf("Invalid command...\n");
								char *invalid_message = INVALID_COMMAND;
					            if (write(cur_fd, invalid_message, strlen(invalid_message)) == -1) {
					                fprintf(stderr,
					                    "Write to client %s failed\n", inet_ntoa(q.sin_addr));
					                remove_client(&active_clients, cur_fd);
									break;
					            }
							}
							
							free(inbuf);
                            break;
                        }
                    }
                }
            }
        }
    }
    return 0;
}
