# STOMP Based Server & Client
### Overview
The world cup is upon us, and you want to stay updated. Thus, we implemented a ”community-led” world cup update subscription
service. Users can subscribe to a game channel and report and receive reports
about the game to and from the other subscribed users.

The communication between the client and the server is based on STOMP frames. The client sends a frame to the server, which parses the
frame, interpets the command and processes it. Finally a reply frame is sent back to the client.

### Quick Start ###
**Compile & Run Server**  
You can run the server by compiling the Java project inside the [server](server) folder, and running it. Since it's written in Java
the server can be compiled and run on both Linux and Windows. When running the server, it recieves a single
command line argument that specifies whether to run in TPC or Reactor mode.

**Run Client**  
After the server is running, you can run the server:
```
client/bin/Client
```
Notice since the client was written in C++ and compiled for Linux, it can only run on Linux.
The source files for the client are found in [src](client/src) folder if you wish to recompile them.

**Usage**  
After both the server and client are running, you can connect and use the server using the built-in commands.

### STOMP Frame Format
This section describes the format of STOMP messages/data packets, as well as the semantics of the data packet exchanges.
A STOMP frame has the following general format:
```
< StompCommand >  
< HeaderName1 >: < HeaderValue1 >  
< HeaderName2 >: < HeaderValue2 >

< FrameBody >  
^ @
```

A STOMP frame always starts with a STOMP command (for example, `SEND`)
on a line by itself. The STOMP command may then be followed by zero or
more header lines. Each header is in a <key>:<value> format and terminated by
a newline. The order of the headers shouldn’t matter.
A blank line indicates the end of the headers and the beginning of the body (which can be empty for some commands).
h can be empty for some commands, as in the example of
SUBSCRIBE above). The frame is terminated by the null character, whichis
represented as `^ @` in this example (Ctrl + @ in ASCII, `\u0000` in Java, and `\0` in C++):

For example this could be a valid `SUBSCRIBE` STOMP frame:
```
SUBSCRIBE  
destination : / dest  
id : 1

^ @
```  

## Client Side
The client uses two threads - one reading the keyboard and writing to the socket, and the other reading from the socket and
outputting. The client first needs to connect to the server, then he can use the built-in commands to send and recieve messages
from the server.

### Client Commands
For any command below requiring a `game_name` input: `game_name` for a game
between some Team A and some Team B should always be of the form
`<team_a_name>_<Team_b_name>`.

**Login**
```
login {host:port} {username} {password}
```
The client tries to connect to the server using the given input.
A `CONNECT` frame is sent to the server.

**Join Game Channel**
```
join {game_name}
```
The client tries to join the given game channel.
A `SUBSCRIBE` frame is sent to the server.

**Exit Game Channel**
```
exit {game_name}
```
The client tries to exit a given game channel.
An `UNSUBSCRIBE` frame is sent to the server.

**Report**
```
report {file}
```
The client reports a file


### Client Frames
<details>

**`CONNECT`**  
A STOMP client initiates the stream or TCP connection to the server by sending
the `CONNECT` frame. The server may reply with a `CONNECTED` frame or an `ERROR` frame.
```
CONNECT
accept - version :1.2
host : stomp . cs . bgu . ac . il
login : meni
passcode : films

^ @
```
  * `accept-version` - The versions of the STOMP protocol the client supports.
    In this case it will be version 1.2.
  * `host` - The name of a virtual host that the client wishes to connect to.
    Since the server will hold only a single host, we can use BGU's university host, so this is set to be
    stomp.cs.bgu.ac.il by default.
  * `login` - The user identifier used to authenticate against a secured STOMP
    server. This is unique for every user.
  * `passcode` - The password used to authenticate against a secured STOMP
    server.
  * The body of this frame is empty.


**`SUBSCRIBE`**  
The `SUBSCRIBE` frame registers a client to a specific topic.
```
SUBSCRIBE
destination :/ topic / a
id :78

^ @
```
* `destination` - Similar to the destination header of SEND. This header
will indicate to the server to which topic the client wants to subscribe.
* `id` - An ID to identify this subscription. When an id header is supplied in the
SUBSCRIBE frame, the server will append the subscription header to
any `MESSAGE` frame sent to the client. This must be generated uniquely in the client before subscribing to a topic.
* The body of this frame is empty.


**`UNSUBSCRIBE`**
The `UNSUBSCRIBE` frame removes an existing subscription, so that the client no longer receives messages from that destination.
```
UNSUBSCRIBE
id :78

^ @
```
* `id` - The subscription ID supplied to the server with the `SUBSCRIBE` frame.
* The body of this frame is empty.

**`DISCONNECT`**
The `DISCONNECT` frame declares to the server that the client wants to disconnect from it.
```
DISCONNECT
receipt :77

^ @
```
* `receipt` - The recipt-id the client expects on the receipt returned by the server.
This number is generated uniquely by the client.
* The body of this frame should be empty

</details>

## Server Side
### Overview
A STOMP server is modeled as a set of topics (queues) to which messages can
be sent. Each client can subscribe to one topic or more and it can send messages
to any of the topics. Every message sent to a topic is being forwarded by the
server to all clients registered to that topic.
