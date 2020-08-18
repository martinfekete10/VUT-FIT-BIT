import re, sys, socket, threading
from signal import signal, SIGINT

# Argument check
if (len(sys.argv) != 2):
    print("To run the program type: make run PORT=no")
    sys.exit(1)
elif not (0 <= int(sys.argv[1]) <= 65535):
    print("Argument should be int less or eq to 65535")
    sys.exit(1)

HOST = '127.0.0.1'
PORT = int(sys.argv[1])

# SIGINT signal handler
def handler(signal_received, frame):
    s.close()
    sys.exit(0)

# Checks is passed string matches IPv4 format
def ip_check(string):
    if (re.fullmatch("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$", string)):
        return True
    else:
        return False

################################## GET ##################################

def get(request_arr):
    # Check validity of GET parameter
    if (re.fullmatch('(/resolve\?name=(.+)\\&type=(PTR|A))', request_arr[1]) == None):
        return str.encode(request_arr[2]) + b" 400 Bad Request\r\n\r\n"
    
    # GET type, either A or PTR
    op_type = re.search('&type=(.*)', request_arr[1]).group(1)
    
    if (op_type == "A"):
        name = re.search('/resolve\?name=(.*)&', request_arr[1]).group(1)
        if ip_check(name) == True:
            return str.encode(request_arr[2]) + b" 400 Bad Request\r\n\r\n"
        try:
            ip = socket.gethostbyname(name)
            return str.encode(request_arr[2]) + b" 200 OK\r\n\r\n" + str.encode(name) + b":A=" + str.encode(ip) + b"\r\n"
        except:
            return str.encode(request_arr[2]) + b" 404 Not Found\r\n\r\n"
    else:
        ip = re.search('/resolve\?name=(.*)&', request_arr[1]).group(1)
        if ip_check(ip) == False:
            return str.encode(request_arr[2]) + b" 400 Bad Request\r\n\r\n"
        try:
            name = socket.gethostbyaddr(ip)[0]
            return str.encode(request_arr[2]) + b" 200 OK\r\n\r\n" + str.encode(ip) + b":PTR="+ str.encode(name) + b"\r\n"
        except:
            return str.encode(request_arr[2]) + b" 404 Not Found\r\n\r\n"

################################## POST ##################################

def post(request, request_arr):
     # Check validity of POST parameter
    if (re.fullmatch('/dns-query', request_arr[1]) == None):
        return str.encode(request_arr[2]) + b" 400 Bad Request\r\n\r\n"

    result = ""
    bad_request = False
    header = request_arr[2] + " 200 OK\r\n\r\n"

    # Get the body of request and read it line by line
    body = re.split("(\r\n\r\n)|(\n\n)", request, maxsplit=1)[3]
    for line in body.splitlines():
        line = "".join(line.split())
        split = line.split(":")
        if (len(split) != 2 or split[0] == ""):
            bad_request = True
            continue
        if (split[1] == "A"):
            if ip_check(split[0]) == True:
                continue
            try:
                result += (line + "=" + socket.gethostbyname(split[0]) + "\r\n")
            except:
                continue
        elif (split[1] == "PTR"):
            if ip_check(split[0]) == False:
                continue
            try:
                result += (line + "=" + socket.gethostbyaddr(split[0])[0] + "\r\n")
            except:
                continue
        else:
            bad_request = True
            continue
    
    if (bad_request == True and result == ""):
        return str.encode(request_arr[2]) + b" 400 Bad Request\r\n\r\n"
    elif (result == "" and body != ""):
        return str.encode(request_arr[2]) + b" 404 Not Found\r\n\r\n"
    else:
        result = header + result
        return str.encode(result)


############################# NEW CLIENT ##############################

def new_client(clientsocket, addr):
    while True:
        data = clientsocket.recv(1024)
        if not data: break
        data = data.decode()
        request_arr = data.split()
        if (request_arr[0] == "GET"):
            result = get(request_arr)
        elif (request_arr[0] == "POST"):
            result = post(data, request_arr)
        else:
            result = str.encode(request_arr[2]) + b" 405 Method Not Allowed\r\n\r\n"
        clientsocket.sendall(result)
        break
    clientsocket.close()

################################## MAIN ##################################

signal(SIGINT, handler)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((HOST, PORT))
s.listen()

while True:
    conn, addr = s.accept()
    threading.Thread(target=new_client, args=(conn, addr)).start()
