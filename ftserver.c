//
// *****************************************************************************
// 
// Author:    Erik Ratcliffe
// Date:      August 9, 2015
// Project:   Program 2 - File server/client
// Filename:  ftserver.c
// Class:     CS 372 (Summer 2015)
//
//
// Overview:
//    Basic file server/client combo. Server in C, client in Python.
//
//    This file contains the main() code for the file server.
//
// *****************************************************************************
//

#include "ft.h"


int main(int argc, char **argv)
{
    int sock, control, data;  // Socket descriptors for server and client:
                              //   control = control message connection
                              //   data = data exchange connection

    // If we did not get two items on our command line, vital information
    // is missing. Display a usage message and exit.
    //
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }

    // The server does not exit when clients disconnect; instead, SIGINT is
    // required. Use a signal handler to clean up whatever needs to be 
    // cleaned up (if anything) when the server recieves a SIGINT.
    //
    if(signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        perror("Could not catch SIGINT");
        exit(1);
    }

    // Start the server and listen for control connections.
    //
    fprintf(stderr, "Server open on %s.\n", argv[1]);
    sock = setupServer(atoi(argv[1]));

    // Only SIGINT will cleanly kill this server. Without it, keep running.
    //
    while(1)
    {
        // Setup a control connection
        //
        control = setupControl(sock);

        // Setup a data connection
        //
        data = setupDataConn(control);

        // Do some file/info transferring
        //
        ftAction(control, data);

        // Close the client and data connections
        //
        fprintf(stderr, "Closing the connection...\n");
        close(control);
        control = -1;
        close(data);
        data = -1;
    }

    // Close the server socket
    //
    close(sock);
    sock = -1;

    return 0;
}
