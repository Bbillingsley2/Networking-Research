import socket

# dictionary containing all of the servers that will have conenctions to the client
urls = [
        'https://mirror.umd.edu/fedora/linux/releases/34/Workstation/x86_64/iso/Fedora-Workstation-Live-x86_64-34-1.2.iso',
        'https://mirror.chpc.utah.edu/pub/fedora/linux/releases/34/Workstation/x86_64/iso/Fedora-Workstation-Live-x86_64-34-1.2.iso',
        'https://mirror.linux-ia64.org/fedora/releases/34/Workstation/x86_64/iso/Fedora-Workstation-Live-x86_64-34-1.2.iso'
    ]

# SERVER 1
# making all of the sockets and ports that will we be using 
host1_sock1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host1_sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host1_sock3 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host1_port1 = 80
host1_port2 = 443
host1_port3 = 8080

# dictionary containing all of server 1's sockets
server1_sockets = [host1_sock1, host1_sock2, host1_sock3]

# binding each socket for server 1 to its respective port 
host1_sock1.bind((urls[0], host1_port1))
host1_sock2.bind((urls[0], host1_port2))
host1_sock3.bind((urls[0], host1_port3))

# looping through the list of sockets in the list and setting the maximum number of conenctions
# for each socket to 1
for sock in server1_sockets:
    sock.listen(1)

# waiting for a conenction on each of the sockets, and when there is one a conenction,
# printing what address it conencted to
conn1, addr1 = host1_sock1.accept()
print("Server 1 connected to: ", addr1)
conn2, addr2 = host1_sock2.accept()
print("Server 1 connected to: ", addr2)
conn3, addr3 = host1_sock3.accept()
print("Server 1 connected to: ", addr3)


# SERVER 2
# making all of the sockets and ports that will we be using 
host2_sock1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host2_sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host2_sock3 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host2_port1 = 80
host2_port2 = 443
host2_port3 = 8080

# dictionary containing all of server 1's sockets
server2_sockets = [host2_sock1, host2_sock2, host2_sock3]

# binding each socket for server 1 to its respective port 
host2_sock1.bind((urls[1], host2_port1))
host2_sock2.bind((urls[1], host2_port2))
host2_sock3.bind((urls[1], host2_port3))

# looping through the list of sockets in the list and setting the maximum number of conenctions
# for each socket to 1
for sock in server2_sockets:
    sock.listen(1)

# waiting for a conenction on each of the sockets, and when there is one a conenction,
# printing what address it conencted to
conn1, addr1 = host2_sock1.accept()
print("Server 2 connected to: ", addr1)
conn2, addr2 = host2_sock2.accept()
print("Server 2 connected to: ", addr2)
conn3, addr3 = host2_sock3.accept()
print("Server 2 connected to: ", addr3)



# SERVER 3
# making all of the sockets and ports that will we be using 
host3_sock1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host3_sock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host3_sock3 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
host3_port1 = 80
host3_port2 = 443
host3_port3 = 8080

# dictionary containing all of server 1's sockets
server3_sockets = [host3_sock1, host3_sock2, host3_sock3]

# binding each socket for server 1 to its respective port 
host3_sock1.bind((urls[2], host3_port1))
host3_sock2.bind((urls[2], host3_port2))
host3_sock3.bind((urls[2], host3_port3))

# looping through the list of sockets in the list and setting the maximum number of conenctions
# for each socket to 1
for sock in server3_sockets:
    sock.listen(1)

# waiting for a conenction on each of the sockets, and when there is one a conenction,
# printing what address it conencted to
conn1, addr1 = host3_sock1.accept()
print("Server 3 connected to: ", addr1)
conn2, addr2 = host3_sock2.accept()
print("Server 3 connected to: ", addr2)
conn3, addr3 = host3_sock3.accept()
print("Server 3 connected to: ", addr3)