# rtchat
real-time chat app

compile simply with 

    make
    
you may need to install ncurses, the c package I used for the UI. In `apt` it is `libncurses-dev`.
Then we need to run an instance of the server before any clients will work:

    ./server_drive
    
and then clients with

    ./client_drive
   
The clients will prompt you for the information they need. Note that we're using IPv6 addresses here if you are giving it an address.
Then you should be able to communicate.

That said, this is still very buggy and with very high probability (>.9 I'd say) it doesn't work at all. But hey, it compiles! Anyway...

### Design

So the idea is that we have chat rooms for members, and the server manages it all. It was at this point that I realized how much simpler it would be if we only had 2 people talking to each other, but oh well I guess. So a client will connect to the server, and ask what rooms are open; upon receiving that list back from the server the client will choose to either join one of those rooms or start a new one. If the user starts a new one, we allocate a little more space in the server to hold the new room and subsequent clients will see it. If we join an already-constructed room, the server will tell each member of that room about us and will give us a list of members of the room, which abounts to a list of <name, ipaddr> pairs. We then can talk to those members.

I was trying to be organised, so I decomposed the client code into several namespaces:
- display (`display.hh/.cc`): handles all the display stuff, also in charge of listening for keystrokes.
- server (`handle_server.hh/.cc`): handles the setup and connection with the server, also in charge of listening for new/leaving members and notifying the server of our leaving
- rtclient (`client_base.hh/.cc`): some type definitions and classes that the client relies on, like a `window` class that remembers what everyone has said, and a `peer`, which is just the username and address of another person in the room.

There is also `broadcast.hh/.cc` which provides a broadcast function that, well, broadcasts.

And it is all held together by `client_driver.cc` which, after using `server` to talk to the server and get set up, simply spawns a couple threads to listen for all the various things that might occur: we get a thread for keystrokes, we get a thread for server handling, we get a thread to listen for incoming messages from peers, and we get a thread to update every `TIME_INTERVAL`. 

---

There wasn't quite as much to do on the server side of things. I just invented a class, `sstate` to hold all the `room`s and `user_info`, and provided some handy methods to do the things I wanted. It's also mutithreaded, mostly because the man page onf linux's `select` is confusing.

---

In the actual client-client communication, there are 2 things happening: 
1. Every time I hit a key, I broadcast what key I hit.
2. Every 1 second (can be adjusted in `config.hh`) I broadcast a sync message which consists of my entire message as it currently stands.

I send these over UDP because I don't want to have to set up TCP streams between everyone. Also, I feel like the "real-time"-ness of this makes the app more akin to a videocaller like Zoom than your standard IM app like Whatsapp, and so part of the challenge was to do this on UDP. I would have tried to put it on QUIC had I time to finish this and also read up on QUIC, but alas, I started in earnest on Saturday. I also tried to implement this over IPv6 for a couple of reasons: I was worried about NAT interfering with my ease of testing, and if you're writing a new network app on IPv4 these days you're a sucker. Of course the drawback is that the lower-level packets get a good deal bigger since their actual payloads are so small, often just a few characters (like, a typical message might be "Henry\C\h").

### Protocol

There's I guess a couple of protocols in this:

###### Client-Server

  Speaker | Meaning | Format 
  ---------|---------|--------
  Client | "Hi, I'm $name" | "name" 
  Server | "Here's a list of rooms" | "roomid\name\name\name\\nroomid\name\name\\n\EOF"
  Client | "Thanks, I'd like to join room $id" | "roomid"
  Server | "Okay, here's the members of that room" | "name\addr\name\addr\name\addr\EOF"
  Server | "Hey, a new user ($name, $addr) has joined your room" | "NEWUSER\nname\addr"
  Server | "Hey ($name, $addr) left your room" | "GOODBYE\nname\addr"
  Client | "Ciao, it's time for me to go" | "BYE"
  
And the \'s are a special character ('\037')

###### Client-Client

There's no chronology for this one, all the messages are of one of 2 forms:
1. "name\S\my_message" for sync packets
2. "name\C\my_ch" for key packets

And of course, there is no guarantee that S packets can get through the network, so they are not guaranteed to fix errors from dropped C packets, but they will likely do this, and we need some way of fixing past errors, because try as we might, we simply can't get text information to be as instantaneous and fleeting as vocal or video information. So I think I've just shown that the whole idea behind the project is a bad one, though relatively interesting. Oh well, too late to change course now.

### Testing

I hardly tested this at all. And the testing that I did do was either incredibly frustrating or not very helpful. To be clear: I got the opening sequence with the server interaction working fine, so long as the server is running on the same machine as the client. When trying to connect to it from other computers, I had difficulties. Namely, I couldn't. I think it may have something to do with the way WSL interacts with networks (it seems to forward packets through Windows or something, and so it's hard to get Windows to forward incoming packets back). I tried running this on patty, but was met with little success, same with my housemate's linux laptop. I don't know. But I can localhost-connect. The problem is then that since I'm using UDP, if I have more than one client on the same machine, I run into demultiplexing issues. And I probably could fix this by giving every client a unique id number and then putting sequence numbers on my packets as well as these ids and then ther could be some logic to not accept packets that I've already seen or that I have passed but it seems like it would take a lot of hackery to get working. So longs story short, I ended up having basically no good way to test this and so it is totally broken. But, the framework is there and I don't want to take the exam so I'm submitting this anyway.
  
 ### Improvements
 
 (besides making what I currently have at all functional). If this were currently functional, the next things to do would be:
 1. Implement this over QUIC. Would fix my multiplexing bug, would allow me to get rid of the sync packets, drastically simplifying evverything. (To be fair, TCP would do this to, but meh, QUIC has a cooler name)
 2. Encrypt. Well, encryption is good, eh? Though I guess I then worry about how the app would get used given memoryless and encrypted...
 3. It would be nice if you didn't have to kill the program to leave your room and could even join another room.
 4. Can always have a better UI
 5. Links to rooms instead of picking from a list

Idk, there's a lot that could be done to this.
