#
# *****************************************************************************
# 
# Author:    Erik Ratcliffe
# Date:      August 9, 2015
# Project:   Program 2 - File transfer server/client
# Filename:  ftclient.py
# Class:     CS 372 (Summer 2015)
#
#
# Overview:
#    Basic file transfer server/client combo. Server in C, client in Python.
#
#    This file contains file transfer client code.
#
# *****************************************************************************
#

import os.path
import socket
import struct
import errno
import sys
import time


#
# *****************************************************************************
#
# checkArgs()
#
#    Entry:   None
#
#    Exit:    None
#
#    Purpose: Checks the validity of command line arguments
#
# *****************************************************************************
#
def checkArgs():
    # Grab the number of arguments on the command line so we don't have
    # to keep doing this.
    numArgs = len(sys.argv)
    
    # We need at least 5 arguments (sys.argv[0-4]) to keep going.
    if numArgs >= 5:
        # Arg 2 should be a port (integer)
        if str.isdigit(sys.argv[2]):
            arg2 = int(sys.argv[2])
            if arg2 < 1 or arg2 > 65536:
                print "Invalid server port number (range: 0-65536)"
                exit(1)
        else:
            print 'Server port number must be an integer (range: 0-65536)'
            
        # If an invalid command was given (must be -l or -g)...
        if '-l' not in sys.argv[3] and '-g' not in sys.argv[3]:
            print 'Invalid command (valid commands: -l (directory list) or -g (get a file)'
            exit(1)
    
        # Check for valid number of arguments...
        if ('-g' in sys.argv[3] and len(sys.argv) < 6) or ('-l' in sys.argv[3] and numArgs < 5):
            print 'Invalid number of arguments.'
            print 'Usage: %s [svr hostname] [svr port] -g [filename] [local port]' % sys.argv[0]
            exit(1)
    
        # At this point, we assume we have a valid hostname (verification
        # seems difficult without actually connecting), we have a valid server
        # port number, and we have a valid command.
    
        # If the user wants a directory listing...
        if '-l' in sys.argv[3]:
            # If an invalid data port was given...
            if str.isdigit(sys.argv[4]):
                arg4 = int(sys.argv[4])
                if arg4 < 1 or arg4 > 65536:
                    print 'Invalid local data port number (range: 0-65536)'
                    exit(1)
            else:
                print 'Data port number must be an integer (range: 0-65536)'
                exit(1)
        else: 
            # If the user wants to download a file...
            if '-g' in sys.argv[3] and numArgs == 6:
                if os.path.isfile(sys.argv[4]):
                    print 'File exists.'
                    exit(1)
                # If an invalid data port was given...
                if str.isdigit(sys.argv[5]):
                    arg5 = int(sys.argv[5])
                    if arg5 < 1 or arg5 > 65536:
                        print 'Invalid local data port number (range: 0-65536)'
                        exit(1)
                else:
                    print 'Data port number must be an integer (range: 0-65536)'
                    exit(1)
    else:
        print 'Usage: %s [svr hostname] [svr port] -l [local port]' % sys.argv[0]
        print '       %s [svr hostname] [svr port] -g [filename] [local port]' % sys.argv[0]
        print '       (hostname and filename are strings, ports are integers from 0-65536)'
        exit(1)

#
# *****************************************************************************
#
# controlConnect(serverName, controlPort)
#
#    Entry:   serverName
#                String containing server hostname
#             controlPort
#                Number of the port for the server.
#
#    Exit:    Returns a socket object for the client/server connection
#
#    Purpose: Create the "control" socket
#
# *****************************************************************************
#
def controlConnect(serverName, controlPort):
    # Create the socket
    controlSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Connect to the server, or die trying.
    try:
        controlSocket.connect((serverName, controlPort))
    except socket.error, errmsg:
        print "Connect failed: %s\nExiting..." % errmsg
        exit(1)

    return controlSocket


#
# *****************************************************************************
#
# dataConnect()
#
#    Entry:   None
#
#    Exit:    Returns a socket object for the client/server connection
#
#    Purpose: Create the "data" socket
#
# *****************************************************************************
#
def dataConnect():
    # Create a socket for the data connection
    try:
        dataSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    except socket.error as msg:
        print 'Socket creation failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        exit()
     
    #Bind socket to local host and port
    try:
        dataSocket.bind(('', dataPort))
    except socket.error as msg:
        print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        exit()
     
    #Start listening on data socket
    try:
        dataSocket.listen(1)
    except socket.error as msg:
        print 'Listen failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        exit()

    return dataSocket
 

#
# *****************************************************************************
#
# dataAccept(dataSocket)
#
#    Entry:   Socket for the "data" connection
#
#    Exit:    Returns a socket and address info for the "data" connection
#
#    Purpose: Complete the "data" connection
#
# *****************************************************************************
#
def dataAccept(dataSocket):
    # Wait for and accept the incoming "client" connection
    try:
        dataConn, dataAddr = dataSocket.accept()
    except socket.error as msg:
        print 'Accept failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
        exit()

    return dataConn, dataAddr


#
# *****************************************************************************
#
# sendMsg(outMsg, sock)
#
#    Entry:   outMsg
#                String containing full message to send.
#             sock
#                Socket object for the client/server connection.
#
#    Exit:    None.
#
#    Purpose: Send a message over a socket
#
# *****************************************************************************
#
def sendMsg(outMsg, sock):
    # Send an outgoing message, or die trying.
    try:
        sent = sock.send(outMsg)
        return sent
    except socket.error, errmsg:
        print "Send failed: %s\nExiting..." % errmsg
        exit(1)
        

#
# *****************************************************************************
#
# recvMsg(sock)
#
#    Entry:   sock
#                Socket object for the client/server connection.
#
#    Exit:    Returns a message received from the server
#
#    Purpose: Receive a message over a socket
#
# *****************************************************************************
#
def recvMsg(sock):
    # Receive an incoming message, or die trying.
    try:
        inMsg = sock.recv(1024) # Read the incoming message into inChatMsg
    except socket.error, errmsg:
        print "Read failed: %s\nExiting..." % errmsg
        exit(1)

    return inMsg  # The length of the message string may be used by
                  # the calling function to determine if the connection 
                  # was closed by the server. We must return it, not
                  # process or display it here.


#
# *****************************************************************************
#
# recvStream(sock)
#
#    Entry:   sock
#                Socket object for the client/server connection.
#
#    Exit:    Returns a message received from the server
#
#    Purpose: Receive a large message over a socket
#
# *****************************************************************************
#
def recvStream(sock):
    # Clear out the strings that will be used to collect incoming data
    inMsg  = ""
    inFrag = ""

    # Keep grabbing fragments from the socket until all have arrived.
    # Assemble them as you go.
    while 1:
        inFrag = recvMsg(sock)
        if not inFrag:
            break
        inMsg += inFrag

    return inMsg

#
# *****************************************************************************
#
# runCommand(commandMsg)
#
#    Entry:   commandMsg
#                String with the command from command line
#
#    Exit:    None
#
#    Purpose: Run the command specified by the user on the command line
#
# *****************************************************************************
#
def runCommand(commandMsg):
    # If the user specified that (s)he wants a file listing...
    if '-l' in commandMsg:
        print 'Receiving directory structure from server.'
        inMsg = recvStream(dataConn)
        print inMsg
    
    # If the user specified that (s)he wants to get a file...
    if '-g' in commandMsg:
        # Receive the file. Hints from 
        # http://stackoverflow.com/questions/289035/receiving-data-over-a-python-socket
        fileName = sys.argv[4]
        outMsg   = fileName
    
        # Send the name of the file
        sendMsg(outMsg, controlSocket)

        # See if it's okay to continue. If this message is "OK", 
        # we're good.
        inMsg = recvMsg(controlSocket)
        if 'OK' in inMsg:
            print 'Receiving "%s" from server.' % fileName
            inMsg = recvStream(dataConn)

            # Write the received file content to a file
            outFile = open(fileName, 'w')
            outFile.write(inMsg)
            outFile.close()
            print 'File transfer complete.'
        else:
            # We got "NO", so display an error
            print 'Server says file was not found or cannot be opened.'


# *****************************************************************************
#
# End of function definitions. On to the main program.
#
# *****************************************************************************

# Verify that we got the correct command line arguments before 
# moving forward
checkArgs()

# Set up variables with values from command line arguments
serverName  = sys.argv[1]
controlPort = int(sys.argv[2]) 
commandMsg  = sys.argv[3]
if '-g' in commandMsg:
    dataPort = int(sys.argv[5])
if '-l' in commandMsg:
    dataPort = int(sys.argv[4])

# Set up the control socket
controlSocket = controlConnect(serverName, controlPort)

# Set up the data socket
dataSocket = dataConnect()

# Send the data port number.
outMsg = socket.htons(dataPort)
sendMsg(str(outMsg), controlSocket)

# Accept an incoming connection to the data socket
dataConn, dataAddr = dataAccept(dataSocket)

# Send the command message on the control socket
outMsg = commandMsg
sendMsg(outMsg, controlSocket)

# Receive confirmation that the command string was received
inMsg = recvMsg(controlSocket)

# If the command string was received...
if 'OK' in inMsg:
    runCommand(commandMsg)

# We're done with the data connection. Close it.
dataConn.close()

# We're done with everything. Close the control socket.
controlSocket.close()

# Close the data socket.
dataSocket.close()
