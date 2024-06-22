LIBS = -lssl -lcrypto
all:client
client:client.cpp myssl.cpp
	g++ -g -o client client.cpp myssl.cpp  -lssl -lcrypto 
clean:
	rm -f epoll

