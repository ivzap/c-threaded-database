import socket
import json
import struct
import os
import sys

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
        payload = f"TYPE FILE_SREQ, OFFSET 0, LENGTH 0, FILENAME {filename}, DATA " + '\0'
        # - serialize payload
        self.sockfd.send(payload.encode('ascii'))
        resp = self.sockfd.recv(max_buffersize)
        # - convert bytes to int
        bytes = struct.unpack('<Q', resp)[0] 
        # 2.) Transfer file in byte chunks
        fd = os.open(filepath, os.O_CREAT | os.O_WRONLY)
        offset = 0
        for _ in range(bytes//max_buffersize):
            payload = f"TYPE FILE_DREQ, OFFSET {offset}, LENGTH {max_buffersize}, FILENAME {filename}, DATA " + '\0'
            # - Serialize payload
            self.sockfd.send(payload.encode('ascii'))
            resp = self.sockfd.recv(max_buffersize)
            # - Write chunk to received dir
            os.lseek(fd, offset, os.SEEK_SET) 
            os.write(fd, resp)
            offset += max_buffersize
        remainder = bytes%max_buffersize
        if remainder:
            payload = f"TYPE FILE_DREQ, OFFSET {offset}, LENGTH {remainder}, FILENAME {filename}, DATA " + '\0'
            # - Write the remainder bytes
            self.sockfd.send(payload.encode('ascii'))
            resp = self.sockfd.recv(remainder)
            # - Write chunk to received dir                 
            os.lseek(fd, offset, os.SEEK_SET)
            os.write(fd, resp)
        os.close(fd)
    
    def close(self):
        self.sockfd.close()

    def _calculate_chunk(self, filename, offset, bytes):
        payload = f"TYPE FILE_UREQ, OFFSET {offset}, LENGTH 0, FILENAME {filename}, DATA " + '\0'
        chunk = max(0, min(bytes, max_buffersize - len(payload)))
        return chunk
    
    def get_filenames(self):
        payload = f"TYPE FILE_LREQ, OFFSET , LENGTH , FILENAME , DATA " + '\0'
        self.sockfd.send(payload.encode('ascii'))
        resp = self.sockfd.recv(max_buffersize)
        files = [f for f in resp.decode('ascii').split('\n') if f]
        return files


    def upload(self, filename, path):
        bytes = os.path.getsize(path)
        fd = os.open(path, os.O_RDONLY)
        chunk = self._calculate_chunk(filename, 0, bytes)
        offset = 0
        while bytes:
            #payload = {'type': 'FILE_UREQ', 'offset': str(offset), 'length': '0', 'filename': filename, 'data': ''} 
            payload = f"TYPE FILE_UREQ, OFFSET {offset}, LENGTH 0, FILENAME {filename}, DATA "
            # read data from file
            os.lseek(fd, offset, os.SEEK_SET)
            payload = payload.encode('ascii') + os.read(fd, chunk) + b'\0'
            self.sockfd.send(payload)
            resp = self.sockfd.recv(max_buffersize)
            # update offset, chunk, and bytes left 
            offset += chunk
            bytes -= chunk
            chunk = self._calculate_chunk(filename, offset, bytes)
        os.close(fd) 


def main():
    # Connect to the server
    client = Client("127.0.0.1", 8000)
    client.connect()
    #client.download("Cat03.jpg")
    #client.upload("test.txt", "./test.txt")
    files = client.get_filenames()
    print(files)
    client.close()
    print("Socket closed.")

if __name__ == "__main__":
    main()

