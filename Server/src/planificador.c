/*
 * planificador.c
 *
 *  Created on: Apr 19, 2013
 *      Author: lucas
 */

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "uncommons/SocketsBasic.h"
#include "uncommons/SocketsServer.h"
#include "planificador.h"
#include "commons/string.h"
#include "uncommons/select.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#define TRUE             1
#define FALSE            0

#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <netinet/in.h>


//TODO CHANGE SIGNALS VALUES AND RESPONSE LENGTH
#define SIGNAL_OK "OK"
#define SIGNAL_BLOCKED "BLOCKED"
#define MAXDATASIZE 1024
#define MAXSIZE 1024
#define TERMINE_NIVEL "Termine nivel"
#define PEDIR "PEDIRRECURSO"
#define TRUE 1
#define FALSE 0
#define BROKEN "BROKEN"

void analize_response(char *response, t_scheduler_queue *scheduler_queue, int fd, int *breakIt);
void nuevo_fd(int sock, int connectlist[], int highsock, fd_set socks,
		t_scheduler_queue *scheduler_queue);

void planificar(t_scheduler_queue *scheduler_queue) {

	if(queue_size(scheduler_queue->character_queue )> 0){

	char *response = (char *) malloc(MAXDATASIZE); //CHECK LENGTH

	int *fd;

	t_level_attributes *level;

	int turno = 0;

	int *breakIt;

	breakIt = FALSE;

	level = getLevelAttributes();

	int turnoActual = atoi(level->turnos);

	int sleepTime = atoi(level->sleep);

	turno = 0;

	breakIt = FALSE;

	printf("Entro al while\n");

	fd = (int *) queue_pop(scheduler_queue->character_queue);

	while (turno < turnoActual && breakIt == FALSE) {



		printf("Realizo el pop\n");

		response = sendMessage(fd, "Paso por el planificador\n");

		if(string_starts_with(response, BROKEN)	){
			break;
		}

		printf("Mando mensaje\n");

		response = recieveMessage(fd);

		if (string_starts_with(response, BROKEN)) {
			break;
		}

		printf("Respondio mensaje\n");

		analize_response(response, scheduler_queue, fd, &breakIt);

		sleep(sleepTime);
		free(response);
		turno++;
	}
	if (string_equals_ignore_case(response, SIGNAL_OK) && breakIt == 0) {
		queue_push(scheduler_queue->character_queue, fd);
	}
	}
}

void planificador(t_scheduler_queue *scheduler_queue) {

	 int    j, len, rc, on = 1;
		   int    listen_sd, max_sd, new_sd;
		   int    desc_ready, end_server = FALSE;
		   int    close_conn;
		   char   buffer[MAXSIZE];
		   struct sockaddr_in   addr;
		   struct timeval       timeout;
		   fd_set master_set;
		   fd_set working_set;

		      /*************************************************************/
		      /* Create an AF_INET stream socket to receive incoming       */
		      /* connections on                                            */
		      /*************************************************************/
		      listen_sd = socket(AF_INET, SOCK_STREAM, 0);
		      if (listen_sd < 0)
		      {
		         perror("socket() failed");
		         exit(-1);
		      }

		      /*************************************************************/
		      /* Allow socket descriptor to be reuseable                   */
		      /*************************************************************/
		      rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
		                      (char *)&on, sizeof(on));
		      if (rc < 0)
		      {
		         perror("setsockopt() failed");
		         close(listen_sd);
		         exit(-1);
		      }

		      /*************************************************************/
		      /* Set socket to be non-blocking.  All of the sockets for    */
		      /* the incoming connections will also be non-blocking since  */
		      /* they will inherit that state from the listening socket.   */
		      /*************************************************************/
		      rc = ioctl(listen_sd, FIONBIO, (char *)&on);
		      if (rc < 0)
		      {
		         perror("ioctl() failed");
		         close(listen_sd);
		         exit(-1);
		      }

		      /*************************************************************/
		      /* Bind the socket                                           */
		      /*************************************************************/
		      memset(&addr, 0, sizeof(addr));
		      addr.sin_family      = AF_INET;
		      addr.sin_addr.s_addr = htonl(INADDR_ANY);
		      addr.sin_port        = htons(scheduler_queue->portInt);
		      rc = bind(listen_sd,
		                (struct sockaddr *)&addr, sizeof(addr));
		      if (rc < 0)
		      {
		         perror("bind() failed");
		         close(listen_sd);
		         exit(-1);
		      }

		      /*************************************************************/
		      /* Set the listen back log                                   */
		      /*************************************************************/
		      rc = listen(listen_sd, 32);
		      if (rc < 0)
		      {
		         perror("listen() failed");
		         close(listen_sd);
		         exit(-1);
		      }

		      /*************************************************************/
		      /* Initialize the master fd_set                              */
		      /*************************************************************/
		      FD_ZERO(&master_set);
		      max_sd = listen_sd;
		      FD_SET(listen_sd, &master_set);


		   do
		   {
		      /**********************************************************/
		      /* Copy the master fd_set over to the working fd_set.     */
		      /**********************************************************/
		      memcpy(&working_set, &master_set, sizeof(master_set));

		      /**********************************************************/
		      /* Call select() and wait 5 minutes for it to complete.   */
		      /**********************************************************/
		      timeout.tv_sec  = 0;
		      timeout.tv_usec = 0;

		      rc = select(FD_SETSIZE, &working_set, NULL, NULL, &timeout);

		      /**********************************************************/
		      /* Check to see if the select call failed.                */
		      /**********************************************************/
		      if (rc < 0)
		      {
		         perror("  select() failed");
		         break;
		      }

		      /**********************************************************/
		      /* Check to see if the 5 minute time out expired.         */
		      /**********************************************************/
		      if (rc == 0 && queue_size(scheduler_queue->character_queue) > 0)
		      {
		    	  planificar(scheduler_queue);
		      }else if(rc > 0){
		      desc_ready = rc;
		      for (j=0; j <= max_sd  &&  desc_ready > 0; ++j)
		      {
		         /*******************************************************/
		         /* Check to see if this descriptor is ready            */
		         /*******************************************************/
		         if (FD_ISSET(j, &working_set))
		         {
		            /****************************************************/
		            /* A descriptor was found that was readable - one   */
		            /* less has to be looked for.  This is being done   */
		            /* so that we can stop looking at the working set   */
		            /* once we have found all of the descriptors that   */
		            /* were ready.                                      */
		            /****************************************************/
		            desc_ready -= 1;

		            /****************************************************/
		            /* Check to see if this is the listening socket     */
		            /****************************************************/
		            if (j == listen_sd)
		            {
		               printf("  Listening socket is readable\n");
		               /*************************************************/
		               /* Accept all incoming connections that are      */
		               /* queued up on the listening socket before we   */
		               /* loop back and call select again.              */
		               /*************************************************/
		               do
		               {
		                  /**********************************************/
		                  /* Accept each incoming connection.  If       */
		                  /* accept fails with EWOULDBLOCK, then we     */
		                  /* have accepted all of them.  Any other      */
		                  /* failure on accept will cause us to end the */
		                  /* server.                                    */
		                  /**********************************************/
		                  new_sd = accept(listen_sd, NULL, NULL);
		                  if (new_sd < 0)
		                  {
		                     if (errno != EWOULDBLOCK)
		                     {
		                        perror("  accept() failed");
		                        end_server = TRUE;
		                     }
		                     break;
		                  }

		                  /**********************************************/
		                  /* Add the new incoming connection to the     */
		                  /* master read set                            */
		                  /**********************************************/
		                  printf("  New incoming connection - %d\n", new_sd);
		                  queue_push(scheduler_queue->character_queue, new_sd);
		                  if (new_sd > max_sd)
		                     max_sd = new_sd;

		                  /**********************************************/
		                  /* Loop back up and accept another incoming   */
		                  /* connection                                 */
		                  /**********************************************/
		               } while (new_sd != -1);
		            }

		            /****************************************************/
		            /* This is not the listening socket, therefore an   */
		            /* existing connection must be readable             */
		            /****************************************************/
		         }
		         } /* End of if (FD_ISSET(i, &working_set)) */
		      } /* End of loop through selectable descriptors */

		   } while (end_server == FALSE);


}


void nuevo_fd(int sock, int connectlist[], int highsock, fd_set socks,
		t_scheduler_queue *scheduler_queue) {
	int listnum;

// Devuelvo el valor correspondiente al fd listener para primero gestionar conexiones nuevas.
	if (FD_ISSET(sock,&socks)) {
		queue_push(scheduler_queue->character_queue, handle_new_connection_scheduler(sock, connectlist, highsock, socks));
	}

}

void analize_response(char *response, t_scheduler_queue *scheduler_queue, int fd, int *breakIt) {

	if (string_starts_with(response, SIGNAL_BLOCKED)) {
		response = string_substring_from(response, sizeof(SIGNAL_BLOCKED));
		blocked_character *blockedCharacter = (blocked_character*) malloc(
				sizeof(blocked_character));
		blockedCharacter->fd = fd;
		blockedCharacter->recurso = response[0];
		*breakIt = TRUE;
		queue_push(scheduler_queue->blocked_queue, blockedCharacter);
	} else if (string_equals_ignore_case(response, TERMINE_NIVEL)) {
		close(fd);
	} else if (string_equals_ignore_case(response, PEDIR)) {
		*breakIt = TRUE;
		queue_push(scheduler_queue->character_queue, fd);
	}
}
