compiler:
	rm -rf *.o
	gcc -o "server.o" ./server.c -lcommons -lcompartidas
	gcc -o "client.o" ./client.c -lreadline -lcommons -lcompartidas
	gcc -o "pthreadServer.o" ./pthreadServer.c -lpthread -lcommons -lcompartidas
	gcc -o "hilosicro.o" ./pthread.c -lreadline -lcommons -lcompartidas -lpthread
	gcc -o "listaMemoria.o" ./listaMemoria.c -lcommons -lcompartidas
	gcc -o "serverMemoria.o" ./serverMemoria.c -lcommons -lcompartidas
	gcc -o "listaMetaData.o" ./listaMetadata.c -lcommons -lcompartidas
	gcc -o "registro.o" ./registro.c -lcommons -lcompartidas