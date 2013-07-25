/*
 * select.c
 *
 *  Created on: Jun 30, 2013
 *      Author: lucas
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "SocketsBasic.h"
#include <commons/string.h>
#include <string.h>
#include <unistd.h>

#define MAXSIZE 1024
#define MAXQUEUE 100
#define BROKEN "BROKEN"

void setnonblocking(int sock) {
	int opts;

	opts = fcntl(sock, F_GETFL);
	if (opts < 0) {
		perror("fcntl(F_GETFL)");
		exit(1);
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sock, F_SETFL, opts) < 0) {
		perror("fcntl(F_SETFL)");
		exit(1);
	}
	return;
}

void build_select_list(int sock, int connectlist[], int highsock, fd_set *socks) {
	int listnum; /* Current item in connectlist for for loops */

	/* First put together fd_set for select(), which will
	 consist of the sock veriable in case a new connection
	 is coming in, plus all the sockets we have already
	 accepted. */

	/* FD_ZERO() clears out the fd_set called socks, so that
	 it doesn't contain any file descriptors. */

	FD_ZERO(socks);

	/* FD_SET() adds the file descriptor "sock" to the fd_set,
	 so that select() will return if a connection comes in
	 on that socket (which means you have to do accept(), etc. */

	FD_SET(sock, socks);

	/* Loops through all the possible connections and adds
	 those sockets to the fd_set */

	for (listnum = 0; listnum < MAXQUEUE; listnum++) {
		if (connectlist[listnum] != 0) {
			FD_SET(connectlist[listnum], socks);
			if (connectlist[listnum] > highsock)
				highsock = connectlist[listnum];
		}
	}
}

int *handle_new_connection(int sock, int *connectlist, int *highsock,
		fd_set *socks) {
	int listnum; /* Current item in connectlist for for loops */
	int *connection = (int*) malloc(sizeof(int)); /* Socket file descriptor for incoming connections */
	int finish = 0;

	/* We have a new connection coming in!  We'll
	 try to find a spot for it in connectlist. */
	connection = accept(sock, NULL, NULL );
	if (connection < 0) {
		perror("accept");
		exit(1);
	}
	sendMessage(connection, "CONNECTED");
	recieveMessage(connection);
	sleep(1);
//	setnonblocking(connection);
	for (listnum = 0; (listnum < MAXQUEUE) && (finish == 0); listnum++)
		if (connectlist[listnum] == 0) {
			printf("\nConnection accepted:   FD=%d; Slot=%d\n", connection,
					listnum);
			connectlist[listnum] = connection;
			finish = 1;
		}
	return connection;
}

int *handle_new_connection_scheduler(int sock, int connectlist[], int highsock,
		fd_set socks) {
	int listnum; /* Current item in connectlist for for loops */
	int *connection = (int*) malloc(sizeof(int)); /* Socket file descriptor for incoming connections */
	int finish = 0;

	/* We have a new connection coming in!  We'll
	 try to find a spot for it in connectlist. */
	connection = accept(sock, NULL, NULL );
	if (connection < 0) {
		perror("accept");
		exit(1);
	}
	setnonblocking(connection);
	return connection;
}

void deal_with_data(int listnum, int connectlist[MAXQUEUE]) {
	char *buffer = (char*) malloc(MAXSIZE); /* Buffer for socket reads */

	int fd = connectlist[listnum];

	buffer = recieveMessage(fd);

	if (string_starts_with(buffer, BROKEN)) {
		/* Connection closed, close this end
		 and free up entry in connectlist */
		printf("\nConnection lost: FD=%d;  Slot=%d\n", connectlist[listnum],
				listnum);
		close(connectlist[listnum]);
		connectlist[listnum] = 0;
	} else {

		/**
		 * HAGO ALGO
		 */

	}
}

void read_socks(int sock, int connectlist[], int highsock, fd_set socks) {
	int listnum;

// Devuelvo el valor correspondiente al fd listener para primero gestionar conexiones nuevas.
	if (FD_ISSET(sock,&socks))
		handle_new_connection(sock, &connectlist, &highsock, &socks);

	for (listnum = 0; listnum < MAXQUEUE; listnum++) {
		if (FD_ISSET(connectlist[listnum],&socks))
			deal_with_data(listnum, connectlist);
	}
}

//int selecting() {
//
//	int sock; /* fd del listener*/
//	int connectlist[MAXQUEUE]; /* array de sockets conectados */
//	fd_set socks; /* lista de fds */
//	int highsock; /* Highest #'d file descriptor, needed for select() */
//
//	int port; /* The port number after conversion from ascport */
//	struct sockaddr_in server_address; /* bind info structure */
//	int reuse_addr = 1; /* Used so we can re-bind to our port
//	 while a previous connection is still
//	 in TIME_WAIT state. */
//	int readsocks; /* Number of sockets ready for reading */
//
//	/* Obtain a file descriptor for our "listening" socket */
//	sock = socket(AF_INET, SOCK_STREAM, 0);
//	if (sock < 0) {
//		perror("socket");
//		exit(1);
//	}
//	/* So that we can re-bind to it without TIME_WAIT problems */
//	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
//
//	/* Set socket to non-blocking with our setnonblocking routine */
//	setnonblocking(sock);
//
//	/* Get the address information, and bind it to the socket */
//	port = atoi(port); /* le paso el puerto */
//	memset((char *) &server_address, 0, sizeof(server_address));
//	server_address.sin_family = AF_INET;
//	server_address.sin_addr.s_addr = htonl(INADDR_ANY );
//	server_address.sin_port = port;
//	if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address))
//			< 0) {
//		perror("bind");
//		close(sock);
//		exit(1);
//	}
//
//	/* Set up queue for incoming connections. */
//	listen(sock, MAXQUEUE);
//
//	highsock = sock;
//	memset((char *) &connectlist, 0, sizeof(connectlist));
//
//	while (1) { /* Main server loop - forever */
//		build_select_list(sock, connectlist, highsock, &socks);
//
//		readsocks = select(FD_SETSIZE, &socks, (fd_set *) 0, (fd_set *) 0,
//				NULL );
//
//		if (readsocks < 0) {
//			perror("select");
//			exit(1);
//		}
//		if (readsocks == 0) {
//			/* Nothing ready to read, just show that
//			 we're alive */
//			printf(".");
//			fflush(stdout);
//		} else
//			read_socks(sock, connectlist, highsock, socks);
//	}
//}
