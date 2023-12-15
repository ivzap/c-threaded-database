import socket
import json
import struct
import os

max_buffersize = 1024 # warning never set buffer size to be larger than servers.

class Client:
    def __init__(self, ip, port):
        self.server_address = (ip, port)
        self.sockfd = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    def connect(self):
        self.sockfd.connect(self.server_address)
    
    def download(self, filename):
        filepath = "./received/" + filename
        # 1.) Get the file size
        payload = {'type': 'FILE_SREQ', 'offset': '0', 'length': '0', 'filename': filename}
        # - serialize payload
        json_data = json.dumps(payload) + '\0'
        self.sockfd.send(json_data.encode('utf-8'))
        resp = self.sockfd.recv(max_buffersize)
        # - convert bytes to int
        bytes = struct.unpack('<Q', resp)[0] 
        # 2.) Transfer file in byte chunks
        fd = os.open(filepath, os.O_CREAT | os.O_WRONLY)
        offset = 0
        for _ in range(bytes//max_buffersize):
            payload = {'type': 'FILE_DREQ', 'offset': f'{offset}', 'length': f'{max_buffersize}', 'filename': filename}
            # - Serialize payload
            json_data = json.dumps(payload) + '\0'
            self.sockfd.send(json_data.encode('utf-8'))
            resp = self.sockfd.recv(max_buffersize)
            # - Write chunk to received dir
            os.lseek(fd, offset, os.SEEK_SET) 
            os.write(fd, resp)
            offset += max_buffersize
        remainder = bytes%max_buffersize
        if remainder:
            payload = {'type': 'FILE_DREQ', 'offset': f'{offset}', 'length': f'{remainder}', 'filename': filename}
            # - Write the remainder bytes
            json_data = json.dumps(payload) + '\0'
            self.sockfd.send(json_data.encode('utf-8'))
            resp = self.sockfd.recv(remainder)
            # - Write chunk to received dir                 
            os.lseek(fd, offset, os.SEEK_SET)
            os.write(fd, resp)
        os.close(fd)
    
    def close(self):
        self.sockfd.close()

def main():
    # Connect to the server
    client = Client("127.0.0.1", 8000)
    client.connect()
    client.download("largetest.txt")
    client.close()
    print("Socket closed.")

if __name__ == "__main__":
    main()

