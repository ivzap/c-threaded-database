# C-Threaded Database
Simple C/C++ database which can have files uploaded to it in addition to giving files to clients in byte-sized chunks. Server uses raw sockets with tcp protocol.

# Limitations
Due to packet splitting file upload chunks over ethernet should be less than 1500 bytes. To be safe, use around 1300 bytes. Not sure how to solve this as the server would need to know how long the incoming request is in bytes exactly. If not, it waits indefinitely.

# How it works
A tcp connection is made between a client and server. The client then makes a request to the server and the server responds accordingly.

```
   Server                                Client
  +-------+                             +-------+
  |       |                             |       |
  | LISTEN|                             |       |
  +-------+                             +-------+
      |                                      |
      |     SYN (seq=x)                      |
      | -----------------------------------> |
      |                                      |
      |                                      |
      |     SYN, ACK (seq=y, ack=x+1)       |
      | <----------------------------------- |
      |                                      |
      |                                      |
      |     ACK (ack=y+1, seq=x+1)           |
      | -----------------------------------> |
      |                                      |
      |                                      |
      |   Request for File                   |
      | <----------------------------------- |
      |                                      |
      |                                      |
      |   File Chunk [0, 1023]               |
      | -----------------------------------> |
      |                                      |


```
_Figure 1._ The client requests a file from the server and the server sends the contents in a byte-sized chunk back to the client.



Byte-sized chunks look like the following...
```
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |10 |11 |12 |13 |14 |15 |16 |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
```
_Figure 2._ A byte-size chunk of 16 bytes.

Multiple requests will likely be made per file request and look like the following...
```
REQUEST(offset 1, length 16, ...)
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |10 |11 |12 |13 |14 |15 |16 |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

REQUEST(offset 17, length 16, ...)
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
|17 |18 |19 |20 |21 |22 |23 |24 |25 |26 |27 |28 |29 |30 |31 |32 |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

REQUEST(offset 33, length 4, ...)
+---+---+---+---+
|33 |34 |35 |36 |
+---+---+---+---+

DONE...
```
_Figure 3._ File download request for a file with size of 36 bytes broken up into chunks of 16 * 2 + 4 * 1 bytes
