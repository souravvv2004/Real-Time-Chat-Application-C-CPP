# Real-Time Chat Application (C/C++)

A multi-client TCP chat application in C/C++ using `select()` for I/O multiplexing — supports multiple simultaneous clients chatting in real time, with named messages broadcast to all connected users.



## Features

- Multi-client support via `select()` based I/O multiplexing
- Real-time message broadcasting to all connected clients
- Named clients — each message is prefixed with the sender's name
- Non-blocking interaction — client can type and receive messages without waiting on either
- Graceful client disconnect handling

---

## Project Structure

```
Real-Time-Chat-Application-C-CPP/
│
├── serverselect.cpp   # Multi-client server (select-based)
├── clientselect.cpp   # Chat client (select-based)
├── Makefile
└── README.md
```

---

## How It Works

## Architecture
```
+---------------------------+
        |     Listening Socket      |
        |        TCP :PORT          |
        +-------------+-------------+
                      |
                      |  new connection
                      v
        +---------------------------+
        |       select() Loop       |
        |     monitors all fds      |
        +---+-------------------+---+
            |                   |
            |                   |
            v                   v
    +-----------------+   +-----------+
    |  New connection |   | Client fd |
    |  accept()       |   | has data  |
    +-------+---------+   +-----+-----+
            |                   |
            v                   v
    +-----------------+   +-----------+
    | Add to          |   | recv()    |
    | client_fds[]    |   | message   |
    +-----------------+   +-----+-----+
                                |
                                |  "name: message"
                                v
                    +-----------------------+
                    |   Broadcast Engine    |
                    |  send() to all other  |
                    |  client_fds[]         |
                    +-----------+-----------+
                                |
               +----------------+----------------+
               |                |                |
               v                v                v
       +------------+   +------------+   +------------+
       |  Client A  |   |  Client B  |   |  Client C  |
       |   fd = 4   |   |   fd = 5   |   |   fd = 6   |
       +------------+   +------------+   +------------+

   Disconnect handling:
   recv() == 0  -->  close(fd)  -->  client_fds[i] = -1

```

### Server

- Maintains an array of connected client file descriptors (`client_fds[]`)
- Uses `select()` to watch the listening socket and all connected clients in a single loop
- On a new connection → `accept()`s it and stores the fd in the first free slot
- On incoming data from a client → reads the message and broadcasts it to all other connected clients
- On disconnect (`recv()` returns `0`) → closes the fd and frees its slot

### Client

- Prompts the user for a name before chatting
- Uses `select()` to watch both `stdin` (keyboard) and the socket at the same time
- Typing a message sends `"name: message"` to the server
- Incoming broadcasts from other clients are printed immediately, without blocking on user input

---

## Build

```bash
make              # builds both client and server
```

## Run

**Start the server** (in one terminal):
```bash
make run-server
```
or manually:
```bash
./server 8080
```

**Connect clients** (in separate terminals):
```bash
make run-client
```
or manually:
```bash
./client 127.0.0.1 8080
```

Open 2 or more client terminals to test real-time broadcasting between them.

## Clean

```bash
make clean
```

---

## Example Session

```
Terminal 1 (server):
Server listening on port 8080...
New client connected (fd=4), slot 0
New client connected (fd=5), slot 1
Client (fd=4): saurabh: hello everyone

Terminal 2 (client - saurabh):
Enter Name: saurabh 
Connected to server.
Type a message and press Enter to send.
hello everyone

Terminal 3 (client - claude):
Enter Name: claude
Connected to server.
Type a message and press Enter to send.
saurabh: hello everyone
```


---