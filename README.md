# STOMP Based Server & Client
Our project involved creating a messaging system using the STOMP (Simple Text Oriented Messaging Protocol) protocol. We implemented a server-side application in Java and a client-side application in C++ to facilitate real-time communication between multiple clients.

## Project Goals
* Experience with socket programming and network communication in Java
* Familiarity with socket programming in C++ for creating a networked client application
* Practice multithreading and concurrent programming in both Java and C++
* Knowledge of integrating applications written in different languages (Java and C++)
* Practice handling data serialization and deserialization across different platforms
* Experience working on a large-scale project where not all code is written by us
* Identifying and resolving issues related to network connectivity, protocol compliance, and performance optimization

## Table of Contents
* [Overview](#overview)
* [Quick Start](#quick-start)
* [Game Events](#game-events)
* [STOMP Frame Format](#stomp-frame-format)
* [Client Side](#client-side)
  - [Client Overview](#client-overview)
  - [Client Commands](#client-commands)
  - [Client Frames](#client-frames)
* [Server Side](#server-side)
  - [Server Overview](#server-overview)
  - [Server Frames](#server-frames)

### Overview
The world cup is upon us, and you want to stay updated. Thus, we implemented a ”community-led” world cup update subscription
service. Users can subscribe to a game channel and report and receive reports
about the game to and from the other subscribed users.

The communication between the client and the server is based on STOMP (Simple Text Oriented Messaging Protocol) frames. The client sends a frame to the server, which parses the
frame, interpets the command and processes it. Finally a reply frame is sent back to the relevant client(/s).

### Quick Start
**Run Server**  
You can run the pre-compiled server by running [Server_Client.jar](server/Server_Client.jar).  
The server takes two command line arguments, port and run mode (in that order):
> java -jar -Server_Client.jar <port> <reactor|tpc>

So for example this would run the server on port 7777 in reactor mode:
```
java -jar -Server_Client.jar 7777 reactor
```
You can also compile it yourself, by compiling the Java project inside the [server](server) folder.  
Since it's written in Java, the server can be compiled and run on both Linux and Windows.


**Run Client**  
After the server is running, you can run the client:
```
client/bin/Client
```
The client was written in C++ and compiled for Linux, so it can only run on Linux.  
The source files for the client are found in [src](client/src) folder if you wish to recompile it by yourself.

**Usage**  
After both the server and client are running, you can connect and use the server using the [built-in commands](#commands).

### Game Events
A game event is the format clients use to report about the game. Each game event has the following properties:
* `event name` - The name of the game event. Does not have to be unique to the event.
* `description` - A description of the game event. Can be anything.
* `time` - The time in the game, in seconds, when the event occurred. This will be used to keep the order of the events reported on the game.
* Game updates properties:
  - `general game updates` - Any stat updates on the game that is not related to a particular team will be listed under this property.
  - `team a updates` - Any stat updates related to team a, such as ball possession, goals, etc.
  - `team b updates` - The same as for team a, but for team b.
Game events will be saved in JSON format and will be used to report by the client to the server.

An  example of how this file would look can be found in [client/data/events1.json](client/data/events1.json).

### STOMP Frame Format
This section describes the format of STOMP (Simple Text Oriented Messaging Protocol) messages/data packets, as well as the semantics of the data packet exchanges.
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
### Client Overview
The client uses two threads - one reading the keyboard and writing to the socket, and the other reading from the socket and
outputting. The client first needs to connect to the server, then he can use the built-in commands to send and recieve messages
from the server.

### <a name="commands">Client Commands</a>
For any command below requiring a `game_name` input: `game_name` for a game
between some Team A and some Team B should always be of the form
`<team_a_name>_<Team_b_name>`.

**Login**
```
login {host:port} {username} {password}
```
The client tries to connect to the server using the given input.
A [`CONNECT`](#connect_frame) frame is sent to the server.
username and password are assumed to be Alphanumeric and in English only.
<details>

<summary>Login Outputs</summary>

* Socket error: Connection error. In this case, the client will print ”Could not connect to server”.
* Client already logged in: If the client has already logged into a server you should not attempt to log in again. The client will print ”The client is already logged in, log out before trying again”.
* New user: If the server connection was successful and the server doesn’t find the username, then a new user is created, and the password is saved for that user. The server then sends back a [`CONNECTED`](#connected_frame) frame.
* User is already logged in: If the user is already logged in, then the server will respond with an [`ERROR`](#error_frame) frame and the client will print "User already logged in".
* Wrong password: If the user exists and the password doesn’t match the saved password, the server will send back an [`ERROR`](#error_frame) frame and the client will print "Wrong password".
* User exists: In case everything is OK, the server sends back a [`CONNECTED`](#connected_frame) frame and the client will print to the screen ”Login successful”.
 
</details>

**Join Game Channel**
```
join {game_name}
```
The client tries to join the given game channel.
A [`SUBSCRIBE`](#subscribe_frame) frame is sent to the server.

**Exit Game Channel**
```
exit {game_name}
```
The client tries to exit a given game channel.
An [`UNSUBSCRIBE`](#unsubscribe_frame) frame is sent to the server.

**Report**
```
report {file}
```
The client reads the Game Event JSON file, and sends each update to the server using a [`SEND`](#send_frame) frame.

**Summarize**
```
summary {game_name} {user} {file}
```
The client will print all received reports from a given user for a given game into the provided file.
If this file does not exist, creates it, otherwise overwrites it.
The game event reports are printed in the order that they happened in the game.

<details>
<summary>Summary output format</summary>
 
```
< team_a_name > vs < team_b_name >
Game stats :
General stats :
< stat_name1 >: < stat_val1 >
< stat_name2 >: < stat_val2 >
...
< team_a_name > stats :
< stat_name1 >: < stat_val1 >
< stat_name2 >: < stat_val2 >
...
< team_b_name > stats :
< stat_name1 >: < stat_val1 >
< stat_name2 >: < stat_val2 >
...
Game event reports :
< game_event_time1 > - < game_event_name1 >:
< game_event_description1 >
< game_event_time2 > - < game_event_name2 >:
< game_event_description2 >
...
```
 
</details>

**Logout**
```
logout
```
This command tells the client that the user wants to log out from
the server. The client will send a [`DISCONNECT`](#disconnect_frame) frame to the server.
The logout command removes the current user from all the topics.
Once the client logs out (receives the [`RECEIPT`](#receipt_frame) frame from the server), it closes the socket.

### Client Frames

<a name="connect_frame"></a>
**CONNECT**  
A STOMP client initiates the stream or TCP connection to the server by sending
the `CONNECT` frame. The server may reply with a [`CONNECTED`](#connected_frame) frame or an [`ERROR`](#error_frame) frame.
```
CONNECT
accept - version :1.2
host : stomp . cs . bgu . ac . il
login : user
passcode : pass

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


<a name="subscribe_frame"></a>
**SUBSCRIBE**  
The `SUBSCRIBE` frame registers a client to a specific topic.
```
SUBSCRIBE
destination :/ topic / a
id :78

^ @
```
* `destination` - Similar to the destination header of SEND. This header
will indicate to the server to which topic the client wants to subscribe.
* `id` - An ID to identify this subscription. When an `id` header is supplied in the `SUBSCRIBE` frame, the server will append the `subscription` header to any [`MESSAGE`](#message_frame) frame sent to the client. This must be generated uniquely in the client before subscribing to a topic.
* The body of this frame is empty.


<a name="unsubscribe_frame"></a>
**UNSUBSCRIBE**  
The `UNSUBSCRIBE` frame removes an existing subscription, so that the client no longer receives messages from that destination.
```
UNSUBSCRIBE
id :78

^ @
```
* `id` - The subscription ID supplied to the server with the `SUBSCRIBE` frame.
* The body of this frame is empty.


<a name="disconnect_frame"></a>
**DISCONNECT**
The `DISCONNECT` frame declares to the server that the client wants to disconnect from it.
```
DISCONNECT
receipt :77

^ @
```
* `receipt` - The recipt-id the client expects on the receipt returned by the server. This number is generated uniquely by the client.
* The body of this frame should be empty


## Server Side
### Server Overview
A STOMP server is modeled as a set of topics (queues) to which messages can
be sent. Each client can subscribe to one topic or more and it can send messages
to any of the topics. Every message sent to a topic is being forwarded by the
server to all clients registered to that topic.

### Server Frames

<a name="connected_frame"></a>
**CONNECTED**  
A `CONNECTED` frame is sent back as a reply to a clients [`CONNECT`](#connect_frame) frame to indicate that the client was successfuly connected to the server.
```
CONNECTED
version :1.2

^ @
```
* `version` - The STOMP version used for this connection. In this case we use version 1.2.
* The body of this frame is empty.

<a name="error_frame"></a> 
**ERROR** 
An `ERROR` frame can be sent back as a reply to any frame, to indicate an error. In this case, the connection is then closed.
```
ERROR
receipt - id : message -12345
message : malformed frame received

The message :
-----
SEND
destined :/ queue / a
receipt : message -12345
Hello queue a !
-----
Did not contain a destination header ,
which is REQUIRED for message propagation .
^ @
```
* `message` - a short description of the error.
* `receipt-id` - if the frame which the error is related to included a `receipt` header, this will match it's value.
* body may contain more detailed information (as in the example above) or may be empty.


<a name="message_frame"></a>
**MESSAGE**  
The `MESSAGE` frame conveys messages from a subscription to the client.
```
MESSAGE
subscription :78
message - id :20
destination :/ topic / a

Hello Topic a
^ @
```
* `destination` - the subscription to which the message is sent.
* `subscription` - a client-unique id that specifies the subscription from which the message was received. This id is supplied by the client.
* `message-id` - a server-unique id for that message.
* The frame body contains the message contents.


<a name="receipt_frame"></a>
**RECEIPT**  
A `RECEIPT` frame is sent from the server to the client once the server has successfully processed a client frame that requests a receipt.
Any frame that the client sends to the server may have a `receipt-id` header, specifying that a `RECEIPT` frame with this id should be sent back to the client once this frame was proccessed.
```
RECEIPT
receipt - id :32

^ @
```
* `receipt-id` - it’s value matches the value specified by the frame that requested the receipt.
* The frame body should be empty
