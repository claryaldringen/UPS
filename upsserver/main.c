/*
 * File:   main.c
 * Author: clary
 *
 * Created on 4. prosinec 2013, 6:19
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#include <time.h>
#include "upsserver.h"

#define BUFSIZE 1000

Client clients[10];
int client_count = 0;

/*
 *
 */
int main(int argc, char** argv) {

	struct timeval tv;
	struct sockaddr_in client_info;
	int max,main_socket;
	int port = 6200;
	fd_set socket_set;

	main_socket = get_configured_socket(port);

	FD_ZERO(&socket_set);
	FD_SET(main_socket, &socket_set);
	max = main_socket;

  tv.tv_sec = 180;
  tv.tv_usec = 0;

	while (select(max + 1, &socket_set, NULL, NULL, &tv) > 0)
	{
		if (FD_ISSET(main_socket, &socket_set))
		{
			socklen_t addrlen = sizeof(client_info);
			add_client(accept(main_socket, (struct sockaddr*)&client_info, &addrlen));
		}
		for(int i = 0; i < client_count; i++)
		{
			if (FD_ISSET(clients[i].id, &socket_set))
			{
				char buffer[BUFSIZE];
				int lenght;
				if ((lenght = recv(clients[i].id, (void *)buffer, BUFSIZE - 1, 0)) <= 0)
				{
					close_client(i);
					break;
				}
				else
				{
					parse(buffer, lenght, i);
					trace_log(inet_ntoa((struct in_addr)client_info.sin_addr), buffer);
				}
			}
		}
		FD_ZERO(&socket_set);
		FD_SET(main_socket, &socket_set);
		max = 0;
		for(int i = 0; i < client_count; i++)
		{
			if (clients[i].id > max)max = clients[i].id;
			FD_SET(clients[i].id, &socket_set);
		}
		if (max < main_socket)max = main_socket;
		tv.tv_sec = 180;
		tv.tv_usec = 0;
	}
	close(main_socket);
	close_clients();
	return (EXIT_SUCCESS);
}

int get_socket()
{
	int sock;
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
	{
		printf("Nelze vytvořit socket.");
		exit(-1);
	}
	return sock;
}

int bind_socket(int socket, int port)
{
	struct sockaddr_in sock_name;
	sock_name.sin_family = AF_INET;
	sock_name.sin_port = htons(port);
	sock_name.sin_addr.s_addr = INADDR_ANY;

	if(bind(socket, (struct sockaddr *)&sock_name, sizeof(sock_name)) == -1)
	{
		printf("Problém s pojmenováním socketu.");
		exit(-1);
	}
	return socket;
}

int listen_on_socket(int socket)
{
	if(listen(socket, 10) == -1)
	{
		printf("Problém s vytvořením fronty.");
		exit(-1);
	}
	return socket;
}

int get_configured_socket(int port)
{
	return listen_on_socket(bind_socket(get_socket(), port));
}

void add_client(int client)
{
	clients[client_count].id = client;
	clients[client_count].logged = 0;
	client_count++;
}

void remove_client(int start)
{
	client_count--;
	for(int i = start; i < client_count; i++)
	{
		clients[i] = clients[i+1];
	}
}

void send_text_to_logged_clients(char *buffer, int length, int index_of_sender)
{
	int len = length + clients[index_of_sender].logged + 2;
	char *message = malloc(len);
	strcpy(message, clients[index_of_sender].login);
	strcat(message, ": ");
	strcat(message, buffer);
	for(int i = 0; i < client_count; i++)
	{
		if(clients[i].logged)send(clients[i].id, message, len, 0);
	}
	free(message);
}

void close_clients()
{
	for(int i = 0; i < client_count; i++)
	{
		close_client(i);
	}
}

void close_client(int client_index)
{
	send(clients[client_index].id, "OK\n", 3, 0);
	close(clients[client_index].id);
	remove_client(client_index);
}

void parse(char *buffer, int lenght, int client_index)
{
	check_login(buffer, lenght, client_index);
	check_logout(buffer, client_index);
	check_all_msg(buffer, lenght, client_index);
	check_ping(buffer, client_index);
	check_users(buffer, client_index);
	check_priv_msg(buffer, lenght, client_index);
}

void check_login(char *buffer, int lenght, int client_index)
{
	char *start = strstr(buffer, "LOGIN");
	if(start != NULL)login(buffer, lenght, client_index);
}

void check_logout(char *buffer, int client_index)
{
	char *start = strstr(buffer, "LOGOUT");
	if(start != NULL)close_client(client_index);
}

void check_all_msg(char *buffer, int lenght, int client_index)
{
	char *start = strstr(buffer, "ALL_MSG");
	if(start != NULL) send_text_to_logged_clients(start+7, lenght-7, client_index);
}

void check_ping(char *buffer, int client_index)
{
	if(strstr(buffer, "PING") != NULL)
	{
		send(clients[client_index].id, "OK\n", 3, 0);
	}
}

void check_users(char *buffer, int client_index)
{
	if(strstr(buffer, "USERS") != NULL)send_user_list(client_index);
}

void login(char *buffer, int length, int client_index)
{
	buffer[length] = '\0';
	strcpy(clients[client_index].login, &buffer[5]);
	clients[client_index].logged = length-5;
	send(clients[client_index].id, "OK\n", 3, 0);
}

void send_user_list(int client_index)
{
	int length = 1;
	for(int i = 0; i < client_count; i++)
	{
		if(clients[i].logged) length += strlen(clients[i].login)+1;
	}
	char *users = malloc(length);
	for(int i = 0; i < client_count; i++)
	{
		if(clients[i].logged)
		{
			strcat(users, clients[i].login);
			strcat(users, ",");
		}
	}
	strcat(users, "\n");
	send(clients[client_index].id, users, length, 0);
	free(users);
}

void check_priv_msg(char *buffer, int length, int client_index)
{
	if(strstr(buffer, "PRIV_MSG") != NULL)
	{
		send_private_message(buffer, length, client_index);
	}
}

void send_private_message(char *buffer, int length, int index_of_sender)
{
	int sended = 0;
	for(int i = 0; i < client_count; i++)
	{
		if(strstr(buffer, clients[i].login) != NULL)
		{
			int len = length + clients[index_of_sender].logged - (clients[i].logged + 6);
			char *message = malloc(len);
			strcpy(message, clients[index_of_sender].login);
			strcat(message, ": ");
			strcat(message, buffer + 8 + clients[i].logged);
			send(clients[i].id,  message, len, 0);
			send(clients[index_of_sender].id, "OK\n", 3, 0);
			sended++;
		}
	}
	if(!sended)send(clients[index_of_sender].id, "ERR\n", 4, 0);
}

void trace_log(char *ip_address, char *line)
{
	FILE *log = fopen("trace.log", "a+");
	fprintf(log, "%s %s\n̈́", ip_address, line);
	fclose(log);
}