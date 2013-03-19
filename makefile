all:client.o server.o
	gcc  ./client/client.o -o ./client/client
	gcc  ./server/server.o -o ./server/server
client.o:
	gcc -c ./client/client.c -o ./client/client.o
server.o:
	gcc -c ./server/server.c -o ./server/server.o
clean:
	rm -r ./client/client.o
	rm -r ./client/client
	rm -r ./server/server.o
	rm -r ./server/server