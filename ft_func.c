//
// *****************************************************************************
// 
// Author:    Erik Ratcliffe
// Date:      August 9, 2015
// Project:   Program 2 - File transfer server/client
// Filename:  ft_func.c
// Class:     CS 372 (Summer 2015)
//
//
// Overview:
//    Basic file transfer server/client combo. Server in C, client in Python.
//
//    This file contains functions that are called by main(). They are
//    here to keep main() tidy.
//
// *****************************************************************************
//

#include "ft.h"  // Specifications for functions are in this header


// *****************************************************************************
// 
// int setupServer(int port)
//
// Purpose: Setup the server connection and listen for clients.
//
// *****************************************************************************
//
int setupServer(int port)
{
    int    sock;                // Socket descriptor for server
    int    optval;              // Holds option values from setsockopt()
    struct sockaddr_in myServ;  // Info describing server sockets

    // Set up the socket: AF_INET for Internet domain, SOCK_STREAM for
    // TCP, 0 for default Internet protocol. If this was a UDP socket,
    // SOCK_STREAM would be SOCK_DGRAM instead.
    //
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { 
        perror("Socket failed");
        exit(1);
    }

    // This is rumored to help with the "address in use" errors that
    // sometimes pop up when trying to start a server on a busy socket. I
    // have found it works sometimes but not always. It is possible my
    // syntax is incorrect but based on everything I have read it is not.
    // I am leaving it here for future experimentation.
    //
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        perror("Setsockopt failed");
        exit(1);
    }

    // Initialize socket structure with zeroes (once handled by bzero()
    // which has been deprecated).
    // 
    memset((char *)&myServ, 0, sizeof(myServ));

    // Use the server hostent information to help populate the sockaddr_in
    // struct that holds server connection data. Set the domain to
    // Internet (AF_INET), convert the port passed on the command line
    // from host to network byte ordering and assign to the sockaddr_in
    // port, and indicate that connections from any address on our network
    // are okay (INADDR_ANY).
    //
    myServ.sin_family      = AF_INET;
    myServ.sin_port        = htons(port); // host to network endian conversion
    myServ.sin_addr.s_addr = htonl(INADDR_ANY);

    // Associate the address assocated with myServ with the socket for
    // this server.
    //
    if(bind(sock, (struct sockaddr *)&myServ, sizeof(myServ)) == -1)
    {
        perror("Bind failed");
        exit(1);
    }

    // Listen for connections.
    //
    if(listen(sock, 1) == -1)
    {
        perror("Listen failed");
        exit(1);
    }

    return sock;
}


// *****************************************************************************
// 
// int setupControl(int sock)
//
// Purpose: Setup the control connection
//
// *****************************************************************************
//
int setupControl(int sock)
{
    int       control;               // Socket descriptor for the control connection
    socklen_t myControlLen;          // Holds size of control socket info
    struct    sockaddr_in myControl; // Info describing control socket

    // Grab the size of the myControl sockaddr_in struct 
    //
    myControlLen = sizeof(myControl);

    // Wait for a client connection. If one is requested, accept it and 
    // return a socket descriptor for the new connection. If it fails,
    // exit with an error. 
    //
    control = accept(sock, (struct sockaddr *)&myControl, &myControlLen);
    if(control == -1)
    {
        perror("Accept failed");
        exit(1);
    }
    else
    {
        fprintf(stderr, "Client connection established.\n");
    }

    return control;
}


//******************************************************************************
// 
// int setupDataConn(int sock)
//
// Purpose: Setup the data connection
//
// *****************************************************************************
//
int setupDataConn(int sock)
{
    char      dataPortStr[6];      
    int       dataPort;             
    int       dataSock;
    socklen_t dataAddrLen;      
    struct    sockaddr_in dataAddr, dataServ;

    dataPortStr[0] = '\0';

    // Get port from client. It's a string when it arrives, so 
    // convert it to an integer. Set the last index of the string to a
    // null terminator before conversion, just to be safe.
    recvStr(&sock, dataPortStr);
    dataPortStr[5] = '\0';
    dataPort = ntohs(atoi(dataPortStr));

    // Get the length of the dataAddr struct (needed to get client IP and
    // port values).
    //
    dataAddrLen = sizeof(dataAddr); 

    // Prepare to extract an IP address from the control socket. I could
    // have extracted the address when we accepted the incoming connection
    // from the client, but that's in another function and it's easy
    // enough to reload the data using getpeername().
    //
    getpeername(sock, (struct sockaddr *)&dataAddr, &dataAddrLen);

    // Create a socket for the data connection.
    //
    if((dataSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    { 
        perror("Data socket failed");
        exit(1);
    }

    // Initialize socket structure with zeroes (once handled by bzero()
    // which has been deprecated).
    // 
    memset((char *)&dataServ, 0, sizeof(dataServ));

    // Use the data that was reloaded by getpeername() above to help
    // populate the sockaddr_in struct that holds data connection info.
    // Set the domain to Internet (AF_INET), convert the port passed on
    // the command line from host to network byte ordering and assign to
    // the sockaddr_in port. Set the server address to the client IP.
    //
    dataServ.sin_family = AF_INET;
    dataServ.sin_port   = htons(dataPort); 
    memcpy(&dataServ.sin_addr, &dataAddr.sin_addr, sizeof(dataAddr.sin_addr));

    // Connect to the client's data port. Cast the sockaddr_in struct to 
    // (sockaddr *) (required).
    //
    if(connect(dataSock, (struct sockaddr *)&dataServ, sizeof(dataServ)) == -1)
    {
        perror("Connect to data port failed");
        exit(1);
    }

    return dataSock;
}


// *****************************************************************************
// 
// void sendDir(int sock)
//
// Purpose: Send a listing of the current local directory
//
// *****************************************************************************
//
void sendDir(int sock)
{
    DIR    *dir;            // Descriptor for the current directory
    struct dirent *entry;   // Holds data about a file in the directory
    int    fnameLen;        // Length of filename (for character checking)
    char   outStr[1024];    // Holds outgoing strings of info/data

    // Try to open the directory. If it works, start reading and sending
    // filenames to the client.
    //
    if((dir = opendir(".")) != NULL)
    {
        while((entry = readdir(dir)) != NULL)
        {
            // Don't show any files beginning with '.' (hidden,
            // directories) or backup files ending with '~'.
            //
            // This just sends each filename as it's found. No sorting.
            //
            fnameLen = strlen(entry->d_name);
            if((entry->d_name[0] != '.') && (entry->d_name[fnameLen - 1] != '~'))
            {
               outStr[0] = '\0';
               sprintf(outStr, "%s%s", entry->d_name, "\n");
               sendStr(&sock, outStr);
            }
        }

        // Close the directory.
        //
        closedir(dir);
    }
    else
    {
        sendStr(&sock, "ERROR: Can not open directory.");
    }
}


// *****************************************************************************
// 
// void sendFile(int *control, int *data, char fileName)
//
// Purpose: Send a file across a network connection.
//
// *****************************************************************************
//
void sendFile(int *control, int *data, char *fileName)
{
    long   inFileSize;   // Input file size
    int    inFp;         // Input file descriptor
    int    inChars;      // Number of input file chars read
    char   *inContent;   // Read content of input file
    struct stat inFile;  // File information for input file

    // Try to open the file for reading. If it works, read the file's
    // content into inContent and send it to the client via the data port.
    //
    if((inFp = open(fileName, O_RDONLY)) >= 0)
    {
        // Let the client know the file successfully opened (read: was
        // found and is readable) so we're good to go.
        //
        sendStr(control, "OK");

        // Get file size info for input file. 
        //
        stat(fileName, &inFile);
        inFileSize = inFile.st_size;

        // Create a properly sized buffer to hold the content.
        //
        inContent = malloc(sizeof(char) * inFileSize);
   
        // Read in the contents of the input file
        //
        inChars = read(inFp, inContent, inFileSize);
        if(inChars == -1)
        {
            perror("Error reading input file");
        }

        // Add a null terminator to the end of the file content.
        //
        inContent[inFileSize] = '\0';

        // Close the input file.
        //
        close(inFp);

        // Send the input file
        //
        fprintf(stderr, "Sending \"%s\".\n", fileName);
        sendStr(data, inContent);

        // Free the input file string pointer
        //
        free(inContent);
        inContent = 0;
    }
    else
    {
        // The file could not be opened for reading (read: doesn't exist,
        // not readable). Let the client know we can't move forward with
        // the transfer.
        //
        fprintf(stderr, "File not found or error opening file. Sending error to client.\n");
        sendStr(control, "NO");
    }
}


// *****************************************************************************
// 
// void ftAction(int control, int data)
//
// Purpose: Actual file transfer functionality between server and client.
//
// *****************************************************************************
//
void ftAction(int control, int data)
{
    char cmd[3];           // -l = list, -g = get
    char outFile[1024];    // Filename requested by client
    int  outFileLen;       // Length of filename requested by client

    // Clear out string that will hold requested filename.
    //
    cmd[0] = '\0';

    // Get command from client. Make the last index of the string a null
    // terminator.
    //
    recvStr(&control, cmd);
    cmd[2] = '\0';

    // Send the client confirmation that the command string was received.
    //
    printf("Client command string: %s\n", cmd);
    sendStr(&control, "OK");

    // If "-l" is received from the client, send a directory listing.
    //
    if(strncmp(cmd, "-l", 2) == 0)
    {
        fprintf(stderr, "List directory requested.\nSending directory contents.\n");
        sendDir(data);
    }
    else if(strncmp(cmd, "-g", 2) == 0)
    {
        // Read requested filename. Add a trailing null terminator (recv()
        // doesn't add that).
        //
        outFile[0] = '\0';
        outFileLen = recvStr(&control, outFile);
        outFile[outFileLen] = '\0';

        fprintf(stderr, "File \"%s\" requested.\n", outFile);

        // Send the file.
        //
        sendFile(&control, &data, outFile);
    }
    else
    {
        // We did not get a valid command. Let the client know there was 
        // a problem.
        //
        sendStr(&control, "ERROR: Only valid commands are -l (list) or -g (get).");
    }
}


// *****************************************************************************
// 
// void sendStr(int *sock, char *str)
//
// Purpose: Send a string across a network connection.
//
// *****************************************************************************
//
void sendStr(int *sock, char *str)
{
    long len;       // Holds the length of the input string
    int  numSent;   // Characters transferred

    // Get the input string length.
    //
    len = strlen(str);

    // Send the string, report the number of characters transferred. If
    // -1, exit with an error.
    //
    if((numSent = send(*sock, str, len, 0)) == -1)
    {
        perror("client send failed");
        exit(1);
    }
}


// *****************************************************************************
// 
// void sendNum(int *sock, long *num)
//
// Purpose: Translates a number from host to network byte order and sends
// it across a network connection.
//
// *****************************************************************************
//
void sendNum(int *sock, long *num)
{
    long numToSend;  // Number to send
    int  numSent;    // Characters transferred

    // Convert the number we're sending from host to network byte order.
    //
    numToSend = htonl(*num);

    // Send the number, report the number of characters transferred. If
    // -1, exit with an error.
    //
    if((numSent = send(*sock, &numToSend, sizeof(numToSend), 0)) == -1)
    {
        perror("client send failed");
        exit(1);
    }
}


// *****************************************************************************
// 
// int recvNum(int *sock)
//
// Purpose: Receive a number from across a network connection.
//
// *****************************************************************************
//
int recvNum(int *sock)
{
    long outNum;    // Number to return
    long inNum;     // Number received 
    int  numRecv;   // Characters transferred

    // Read the number, report the number of characters transferred. If
    // -1, exit with an error.
    //
    if((numRecv = recv(*sock, &inNum, sizeof(inNum), 0)) == -1)
    {
        perror("server recv failed (client type)");
        exit(1);
    }
    else if(numRecv == 0)
    {
        perror("socket closed during recv");
        exit(1);
    }

    outNum = ntohl(inNum); // Convert the number from network to host byte order

    return outNum;         // Return the received, converted number
}


// *****************************************************************************
// 
// int recvStr(int *sock, char *str)
//
// Purpose: Receive a short string from across a network connection.
//
// *****************************************************************************
//
int recvStr(int *sock, char *str)
{
    int numRecv;  // Number of characters received

    // Read the string, report the number of characters transferred. If
    // -1, exit with an error.
    //
    // Set buffer size 1024 to keep things simple. This is for small
    // strings so this shouldn't be a problem.
    //
    if((numRecv = recv(*sock, str, 1024, 0)) == -1)
    {
        perror("client recv failed");
        exit(1);
    }

    // Return the number of characters received.
    //
    return numRecv;
}


// *****************************************************************************
// 
// void sigint_handler(int signum)
//
// Purpose: Perform actions when SIGINT is received.
//
// *****************************************************************************
//
void sigint_handler(int signum)
{
    // If SIGINT is caught, just exit with a message. We're in a signal
    // handler, so printf() won't work. Use write() instead.
    //
    write(STDERR_FILENO, "SIGINT caught, exiting server...\n", 34);
    exit(1);
}

