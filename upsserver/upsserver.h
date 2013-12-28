/*
 * File:   upsserver.h
 * Author: clary
 *
 * Created on 11. prosinec 2013, 6:57
 */

#ifndef UPSSERVER_H
#define	UPSSERVER_H

typedef struct {
	int id;
	char login[16];
	int logged;
} Client;

int get_socket();
int bind_socket(int, int);
int listen_on_socket(int);
int get_configured_socket(int);
void add_client(int);
void remove_client(int);
void send_text_to_clients(char*, int);
void close_clients();
void close_client(int);
void parse(char*, int, int);
void check_login(char*, int, int);
void check_logout(char*, int)
void check_all_msg(char*, int, int);
void check_ping(char*, int);
void check_users(char*, int);
void login(char*, int , int);
void send_user_list(int);
void check_priv_msg(char*, int , int);
void send_private_message(char*, int, int);
void trace_log(char*, char*);

#endif	/* UPSSERVER_H */

