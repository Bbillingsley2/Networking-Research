# import requests is needed for HTTP request
import requests
import multiprocessing
import socket

# Define constants
FILE_PARTS = [
    ('https://speed.hetzner.de/1GB.bin', 0, 999, 'file.part1'),
    ('https://speed.hetzner.de/1GB.bin', 1000, 1999, 'file.part2'),
    ('https://speed.hetzner.de/1GB.bin', 2000, 2999, 'file.part3'),
    ('https://speed.hetzner.de/1GB.bin', 3000, 3999, 'file.part4')
]

NUM_PROCESSES = 4
BUFFER_SIZE = 1024 * 1024  # 1 MB

def _download_part(address, start_byte, end_byte, part_index):

    url = address
    parsed_url = requests.utils.urlparse(url) #scheme (http/https)
    host = parsed_url.netloc # network location 
    path = parsed_url.path

    # Stream Sockets: Delivery in a networked environment is guaranteed
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, 80))

    request = f'GET {path} HTTP/1.1\r\nHost: {host}\r\nRange: bytes={start_byte}-{end_byte}\r\n\r\n'.encode()
    sock.sendall(request)

    buffer_size=BUFFER_SIZE

    with open("myfile", 'r+b') as file: # open the file and read in a binary mode?
        
        #The tell() method returns the current file position in a file stream.
        file.seek(start_byte)

        while True:
            data = sock.recv(buffer_size)
            if not data:
                break
            file.write(data)

            # Loop until all data has been received
            bytes_received = len(data)
            while bytes_received < buffer_size and bytes_received < end_byte - start_byte + 1:
                remaining_bytes = end_byte - start_byte + 1 - bytes_received
                data = sock.recv(min(buffer_size, remaining_bytes))
                if not data:
                    break
                file.write(data)
                bytes_received += len(data)

    sock.close()


if __name__=='__main__':

    
    manager = multiprocessing.Manager()
    # Start the download processes for each part
    processes = []
    for i in range(NUM_PROCESSES):
        process_parts = FILE_PARTS[i::NUM_PROCESSES]
        print(process_parts)
        process = multiprocessing.Process(target=_download_part, args=process_parts[0])
        process.start()
        processes.append(process)

    # Wait for all processes to finish
    for process in processes:
        process.join()

    print('File download complete')