#include <thread>
#include "TCPRequestChannel.h"
#include <unistd.h>
#include "json.hpp"
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <iostream>
using namespace std;
using json = nlohmann::json;


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

void process_file_request (TCPRequestChannel* rc, int fOffset, int fLength, string filename, char* request) {
	/* request buffer can be used for response buffer, because everything necessary have
	been copied over to filemsg f and filename*/
	char* response = request; 
    
	// make sure that client is not requesting too big a chunk
	if (fLength > buffercapacity) {
		cerr << "Client is requesting a chunk bigger than server's capacity" << endl;
		cerr << "Returning nothing (i.e., 0 bytes) in response" << endl;
		rc->cwrite(response, 0);
	}

	FILE* fp = fopen(filename.c_str(), "rb");
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

void process_request(TCPRequestChannel *rc, char * buffer){
    auto reqObj = json::parse(buffer);
    int fOffset = stoi(string(reqObj["offset"]));
    int fLength = stoi(string(reqObj["length"]));
    string filename = reqObj["filename"];
    filename = "./DATABASE/" + filename;
    string type = reqObj["type"];
    if(type=="QUIT_MSG"){
        printf("%s\n", "Client-side is done and exited");
        exit(0);
    }
    else if(type=="FILE_SREQ"){
        __int64_t fs = get_file_size (filename);
        rc->cwrite ((char*) &fs, sizeof(__int64_t));
    }
    else if(type=="FILE_DREQ"){
        process_file_request(rc, fOffset, fLength, filename, buffer);
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
		}
		else if (nbytes == 0) {
			perror("Server could not read anything... Terminating");
			break;
		}
		process_request(channel, buffer);
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
