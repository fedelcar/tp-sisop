#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include "inotify.h"
#include <pthread.h>
#include <uncommons/fileStructures.h>
#include <uncommons/SocketsBasic.h>
#include <commons/config.h>
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
#define MAXSIZE 1024
#define TIEMPOCHEQUEODEADLOCK "TiempoChequeoDeadlock"

void getValues(inotify_struct * datos) {

	t_config *configFile;


	configFile = config_create(datos->path);

	int y;
	for (y = 0; y<list_size(datos->lista);y++){

		inotify_list_struct * dato = list_get(datos->lista,y);
		dato->valor = atoi(config_get_string_value(
										configFile,dato->nombre));
	}
}


int inotify(inotify_struct *datos) {

	int fd;

	fd = inotify_init();
	if (fd < 0)
		perror("inotify_init");

	int wd;

	wd = inotify_add_watch (fd,
	                datos->path,
	                IN_MODIFY );

	if (wd < 0)
	        perror ("inotify_add_watch");

	char buf[BUF_LEN];
	int len;

	while (1){

	len = read (fd, buf, BUF_LEN);
	if (len < 0) {
	        if (errno == EINTR){}
	                /* need to reissue system call */
	        else
	                perror ("read");
	} else if (!len){}
	        /* BUF_LEN too small? */
	getValues(datos);

	}

	return 0;
}
