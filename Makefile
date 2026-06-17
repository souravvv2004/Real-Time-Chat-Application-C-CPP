C = gcc
CFLAGS = -Wall -Wextra

CLIENT_SRC = clientselect.cpp
SERVER_SRC = serverselect.cpp

CLIENT_OUT = client
SERVER_OUT = server

all: $(CLIENT_OUT) $(SERVER_OUT)

$(CLIENT_OUT): $(CLIENT_SRC)
	$(C) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_OUT)

$(SERVER_OUT): $(SERVER_SRC)
	$(C) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_OUT)

run-server: $(SERVER_OUT)
	./$(SERVER_OUT) 8080

run-client: $(CLIENT_OUT)
	./$(CLIENT_OUT) 127.0.0.1 8080

clean:
	rm -f $(CLIENT_OUT) $(SERVER_OUT)

.PHONY: all run-server run-client clean