#include <thread>
#include "TCPRequestChannel.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include <dirent.h>
using namespace std;


int buffercapacity = 1024;
char* buffer = NULL; // buffer used by the server, allocated in the main

__int64_t get_file_size (string filename) {
    struct stat buf;
    int fd = open(filename.c_str(), O_RDONLY);
    fstat(fd, &buf);
    __int64_t size = (__int64_t) buf.st_size;
    close(fd);
    return size;
}

void process_file_request (TCPRequestChannel* rc, int fOffset, int fLength, char *filename, char* request) {
	/* request buffer can be used for response buffer, because everything necessary have
	been copied over to filemsg f and filename*/
	char* response = request; 
    
	// make sure that client is not requesting too big a chunk
	if (fLength > buffercapacity) {
		cerr << "Client is requesting a chunk bigger than server's capacity" << endl;
		cerr << "Returning nothing (i.e., 0 bytes) in response" << endl;
		rc->cwrite(response, 0);
	}

	FILE* fp = fopen(filename, "rb");
	if (!fp) {
		cerr << "Server received request for file: " << filename << " which cannot be opened" << endl;
		rc->cwrite(buffer, 0);
		return;
	}
	fseek(fp, fOffset, SEEK_SET);
	int nbytes = fread(response, 1, fLength, fp);
    
	/* making sure that the client is asking for the right # of bytes,
	this is especially imp for the last chunk of a file when the 
	remaining lenght is < buffercap of the client*/
	assert(nbytes == fLength); 

	rc->cwrite(response, nbytes); // +1 for the byte offset?
	
    fclose(fp);
}

void process_file_upload_request(TCPRequestChannel* rc, int fOffset, int fLength, char * filename, char* request){
    int fd = -1;
    if((fd = open(filename, O_CREAT | O_WRONLY)) < 0){
        perror("Failed to open/create file during upload request.");
        exit(1);
    }
    lseek(fd, fOffset, SEEK_SET);

    if(write(fd, request, fLength) < 0){
        perror("Failed to write file chunk to database during upload request.");
        exit(1);
    }
    rc->cwrite(request, fLength); // +1 for the byte offset?

    close(fd);
}

// return start and length of specific protocol operation to avoid copying request

pair<int, int> get_request_protocol(char * request, long unsigned int nbytes, char * protocol){
    // find the last index + 1 of type in request
    pair<int,int> type = {-1,-1};
    int start = -1;
    for(long unsigned int i = 0; i < nbytes; i++){
        long unsigned int j = i;
        while(j < nbytes && j-i < nbytes && protocol[j-i] == request[j]){
            j++;
        }
        if(j-i == strlen(protocol)){
            start = j+1; // +1 bc of space
            break;
        }
    }
    type.first = start;
    if(start == -1){
        return type;
    }
    if(strcmp(protocol, "DATA") == 0){
        type.second = nbytes - 1 - 1; // -1 since index and -1 for null terminated character sent from client 
        return type;
    }

    long unsigned int end = start;
    while(end+1 < nbytes and request[end+1] != ','){
       end++; 
    }
    type.second = end;
    return type;
}

void process_ls_request(TCPRequestChannel *rc, char * buffer){
    buffer[0] = '\0';
    DIR *d;
    d = opendir("./DATABASE");
    struct dirent *dir;
    int offset = 0;
    while((dir = readdir(d))){
        if(strcmp(dir->d_name,".") == 0 || strcmp(dir->d_name, "..") == 0){
            continue;
        }
        char * filename = dir->d_name;
        strcat(filename, "\n");
        int len = strlen(filename);
        strcat(buffer, filename);
        offset += len;
    }
    rc->cwrite(buffer, offset+1); 
}

void process_request(TCPRequestChannel *rc, char * buffer, int nbytes){
    
    pair<int, int> type = get_request_protocol(buffer, nbytes, (char*)"TYPE");
    pair<int, int> offset = get_request_protocol(buffer, nbytes, (char*)"OFFSET");
    pair<int, int> length = get_request_protocol(buffer, nbytes, (char*)"LENGTH");
    pair<int, int> filename = get_request_protocol(buffer, nbytes, (char*)"FILENAME");
    pair<int, int> data = get_request_protocol(buffer, nbytes, (char*)"DATA");
    if(type.first == -1 || offset.first == -1 || length.first == -1 || filename.first == -1 || data.first == -1){
        perror("Unknown request, cannot parse.\n");
        exit(1);
    }
    char filenameCstr[filename.second - filename.first + 2];
    strncpy(filenameCstr, buffer + filename.first, filename.second - filename.first+1);
    filenameCstr[filename.second - filename.first + 1] = '\0'; // safety
    
    string path = "./DATABASE/" + string(filenameCstr);
    
    char offsetCstr[offset.second - offset.first + 2];
    strncpy(offsetCstr, buffer + offset.first, offset.second - offset.first+1);
    offsetCstr[offset.second - offset.first + 1] = '\0'; // safety

    char lengthCstr[length.second - length.first + 2];
    strncpy(lengthCstr, buffer + length.first, length.second - length.first+1);
    lengthCstr[length.second - length.first + 1] = '\0'; // safety
    
    if(strncmp(buffer + type.first, "QUIT_MSG", 8) == 0){
        printf("%s\n", "Client-side is done and exited");
        exit(0);
    }
    else if(strncmp(buffer + type.first, "FILE_SREQ", 9) == 0){
        __int64_t fs = get_file_size (path);
        rc->cwrite ((char*) &fs, sizeof(__int64_t));
    }
    else if(strncmp(buffer + type.first, "FILE_DREQ", 9) == 0){
        process_file_request(rc, stoi(offsetCstr), stoi(lengthCstr), (char*)path.c_str(), buffer);
    }
    else if(strncmp(buffer + type.first, "FILE_UREQ", 9) == 0){
        process_file_upload_request(rc, stoi(offsetCstr), data.second - data.first + 1, (char*)path.c_str(), buffer + data.first);
    }
    else if(strncmp(buffer + type.first, "FILE_LREQ", 9) == 0){
        process_ls_request(rc, buffer); 
    }

}

void handle_process_loop (TCPRequestChannel* channel) {
	/* creating a buffer per client to process incoming requests
	and prepare a response */
	char* buffer = new char[buffercapacity];
	if (!buffer) {
		perror("Cannot allocate memory for server buffer");
        exit(1);
	}
    
	while (true) {
        // read whatever the client has sent: blocks until we read something
		int nbytes = channel->cread(buffer, buffercapacity, false);
        if (nbytes < 0) {
			perror("Client-side terminated abnormally");
			break;
		}else if (nbytes == 0) {
			perror("Server could not read anything... Terminating");
			break;
		}
		process_request(channel, buffer, nbytes);
	}
	delete[] buffer;
	delete channel;
}

int main (int argc, char* argv[]) {
	buffercapacity = 1024;
    string port = "";
	int opt;
	while ((opt = getopt(argc, argv, "m:r:")) != -1) {
		switch (opt) {
			case 'm':
				buffercapacity = atoi(optarg);
				break;
            case 'r':
                port = optarg;
                break;
		}
	}
	srand(time_t(NULL));
	
	// FIXME: open socket with address="" and r from CLI
	//		  then enter infinite loop calling TCPReqChan::accept_conn() and 2nd TCP constructor
	//		  dispatching handle_process_loop thread for new channel
    TCPRequestChannel *control_channel = new TCPRequestChannel(std::string(""), port);
    while (true) {
        // listen for incoming channel requests-- accept them.
        int cSockFd = control_channel->accept_conn();
        TCPRequestChannel *newClient = new TCPRequestChannel(cSockFd);
        std::thread t(handle_process_loop, newClient);
        t.detach(); 
    }
	delete control_channel;
}
