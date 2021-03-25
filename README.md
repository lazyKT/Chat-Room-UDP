# Chat-Room-UDP

Chat room for the clients within the same network using UDP Sockets.

### Compilation Instruction
Server
```
gcc -o uchatserv.o src/uchatserv.c src/netutils.c
```
Client
```
gcc -o uchatcln.o src/uchatcln.c
```

### Run Instruction
Server
```
./uchatserv.o
```
Client
```
./uchatcln.o -u username <server-ip-address>
```
> Please run the server program first. Client program will throw an socket_timeout error if there is no server is available.
>

#### Server and Client program Command List
- ./exit     // _Exit or terminate the current process(program)_
- ./sh cli   // _Show currently connected client list_
