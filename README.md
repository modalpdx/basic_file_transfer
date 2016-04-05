#Basic File Transfer Client/Server
A school project for a basic FTP-like data- and command-connection file
transfer suite.

##About the server:

The file transfer server is written in C. The source code is in the following
files:

- ftserver.c
- ft_func.c
- ft.h

To build the server, just run 'make'. The Makefile should take care of the
rest.

```
  make
```

This should result in a binary called ftserver. To start the server,
run the ftserver program and pass an available port number as its first
(and only) argument. Example:

```
  ./ftserver 54321
```

Note: the ftserver program must be running on the server before running
the client.

The ftserver program only allows one client at a time, but it will cycle
itself to accept a new connection when the client terminates. To completely
quit the ftserver program, send a SIGINT signal either with the UNIX "kill"
command or by hitting CTRL-C.


##About the client:

The client is written in Python. There are no build steps involved. 

To start the client, pass the ftclient.py script to Python and specify
the server's hostname (string, not IP address) and port number, then a
command, then optionally a filename and data port (if downloading a file) or
just a data port (if getting a directory listing). Examples:

  Downloading a file:

```
    python ftclient.py ftserverhost 54321 -g filename 12345
```

  Getting a directory listing:

```
    python ftclient.py ftserverhost 54321 -l 12345
```

Note: the ftserver program must be running on the server before running
the client.

After the server has finished sending either the file or the directory listing
(or an error message if any errors occurred), the client will close and the
server will cycle to accept a new client connection.


##Disclaimer:

This is code from a school project. It satisfies assignment requirements but is nowhere near as "scrubbed" as released software should be. Security is not addressed, only functionality and no input validation. If you use this code for anything other than satisfying your curiosity, please keep the following in mind:

- there is no warranty of any kind (you use the code as-is)
- there is no support offered, please do not ask
- there is no guarantee it'll work, although it's not complex so it should work
- please do not take credit for code you did not write, especially if you are a student. NO CHEATING.

Thanks!
