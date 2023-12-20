# C-Threaded Database
Simple C/C++ database which can have files uploaded to it in addition to giving files to clients in byte sized chunks. Server uses raw sockets with tcp protocol.

# Limitations
Due to packet splitting file upload chunks over ethernet should be less than 1500 bytes. To be safe, use around 1300 bytes. Not sure how to solve this as the server would need to know how long the incoming request is in bytes exactly. If not, it waits indefinitely.
