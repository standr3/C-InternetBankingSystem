
# Fisierul cu datele clientilor 
FILE_USERS = "users_data_file"

# Portul pe care asculta serverul
PORT = 34445

# Adresa IP a serverului
IP_SERVER = 127.0.0.1


all: build

build: build_server build_client

build_server: 
	gcc server.c -o server
build_client: 
	gcc client.c -o client
run: run_server 

run_server: build_server
	./server ${PORT} ${FILE_USERS}
run_client: build_client
	./client ${IP_SERVER} ${PORT}
clean: 
	rm -f server client client-*