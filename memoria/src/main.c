#include <stdio.h>
#include <funcionesCompartidas/log.h>
#include "console.h"
#include "console.c"
#include "server.h"
#include "server.c"
#include <commons/collections/list.h>
#include <pthread.h>

#define IPFILESYSTEM "192.168.0.21"
#define PORTFILESYSTEM "8080"

#define MYPORT "8081"

t_log *file_log;
t_log *log_server;

char *IP_FS = IPFILESYSTEM;
char *PORT_FS = PORTFILESYSTEM;


int main(int argc, char const *argv[]) {

  pthread_t server;
  pthread_t client;

  int control = 0;
  t_log *file_log = crear_archivo_log("Memoria", true,"./logC");
  int socketClient = establecerConexion(IP_FS,PORT_FS,file_log,&control);

  console(socketClient);
  pthread_create(&server,NULL, &start_server, MYPORT);
  //pthread_create(&client,NULL, &connectToSeeds,(void *) totalSeeds);

  pthread_join(server,NULL);
  //pthread_join(client,NULL);

  return 0;
}
