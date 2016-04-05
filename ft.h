//
// *****************************************************************************
// 
// Author:    Erik Ratcliffe
// Date:      August 9, 2015
// Project:   Program 2 - File transfer server/client
// Filename:  ft.h
// Class:     CS 372 (Summer 2015)
//
//
// Overview:
//    Basic file transfer server/client combo. Server in C, client in Python.
//
//    This file contains variable definitions and function prototypes.
//
// *****************************************************************************
//

#ifndef FT_H
#define FT_H


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>


// *****************************************************************************
// 
// int setupServer(int port)
//
//    Entry:   int port
//                Number of the port for the server
//
//    Exit:    Integer containing the socket descriptor.
//
//    Purpose: Setup the server connection and listen for clients.
//
// *****************************************************************************
//
int setupServer(int port);


// *****************************************************************************
// 
// int setupControl(int sock)
//
//    Entry:   int sock
//                Descriptor for the server socket.
//
//    Exit:    Integer containing the control connection descriptor.
//
//    Purpose: Setup the control connection
//
// *****************************************************************************
//
int setupControl(int sock);


// *****************************************************************************
// 
// int setupDataConn(int sock)
//
//    Entry:   int sock
//                Descriptor for the data socket.
//
//    Exit:    Integer containing the data connection descriptor.
//
//    Purpose: Setup the data connection
//
// *****************************************************************************
//
int setupDataConn(int sock);


// *****************************************************************************
// 
// void ftAction(int control, int data)
//
//    Entry:   int control
//                Descriptor for the control connection
//             int data
//                Descriptor for the data connection
//
//    Exit:    None.
//
//    Purpose: Actual file transfer functionality between server and client.
//
// *****************************************************************************
//
void ftAction(int control, int data);


// *****************************************************************************
// 
// void sendDir(int sock)
//
//    Entry:   int sock
//                Socket descriptor for the connection
//
//    Exit:    None
//
//    Purpose: Send a listing of the current local directory
//
// *****************************************************************************
//
void sendDir(int sock);


// *****************************************************************************
// 
// void sendFile(int *control, int *data, char *fileName)
//
//    Entry:   int *control
//                Socket for the control connection
//             int *data
//                Socket for the data connection
//             char *fileName
//                Name of the file to send
//
//    Exit:    None
//
//    Purpose: Send a file across a network connection.
//
// *****************************************************************************
//
void sendFile(int *control, int *data, char *fileName);


// *****************************************************************************
// 
// void sendStr(int *sock, char *str)
//
//    Entry:   int *sock
//                Socket for the current network connection
//             char *str
//                String to send across the connection
//
//    Exit:    None.
//
//    Purpose: Send a string across a network connection.
//
// *****************************************************************************
//
void sendStr(int *sock, char *str);


// *****************************************************************************
// 
// void sendNum(int *sock, long *num)
//
//    Entry:   int *sock
//                Socket for the current network connection
//             long *num
//                Number to send across the connection
//
//    Exit:    None.
//
//    Purpose: Translates a number from host to network byte order and sends
//    it across a network connection.
//
// *****************************************************************************
//
void sendNum(int *sock, long *num);


// *****************************************************************************
// 
// int recvNum(int *sock)
//
//    Entry:   int *sock
//                Socket for the current network connection
//
//    Exit:    Integer translated from network to host byte order.
//
//    Purpose: Receive a number from across a network connection.
//
// *****************************************************************************
//
int recvNum(int *sock);


// *****************************************************************************
// 
// int recvStr(int *sock, char *str)
//
//    Entry:   int *sock
//                Socket for the current network connection
//             char *str
//                Short string to receive from across the connection 
//
//    Exit:    An integer containing the actual number of characters received,
//             or -1 if the client disconnected.
//
//    Purpose: Receive a short string from across a network connection.
//
// *****************************************************************************
//
int recvStr(int *sock, char *str);


// *****************************************************************************
// 
// void sigint_handler(int signum)
//
//    Entry:   int signum
//                Number representing the signal that was received (not used)
//
//    Exit:    None
//
//    Purpose: Perform actions when SIGINT is received.
//
// *****************************************************************************
//
void sigint_handler(int signum);


#endif
