/*
 * select.h
 *
 *  Created on: Jun 30, 2013
 *      Author: lucas
 */
#define MAXQUEUE 100

void setnonblocking(int sock);

void build_select_list(int sock, int connectlist[], int highsock, fd_set *socks);

int *handle_new_connection(int sock, int connectlist[], int highsock, fd_set socks);

void deal_with_data(int listnum, int connectlist[MAXQUEUE]);

void read_socks(int sock, int connectlist[], int highsock, fd_set socks);

int *handle_new_connection_scheduler(int sock, int connectlist[], int highsock, fd_set socks);
