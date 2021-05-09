// Base file for imports and type definitions and things

#pragma once

#include <vector>
#include <string>
#include <deque>

// parallelism
#include <mutex>
#include <shared_mutex>
#include <thread>

// config file
#include "../config.hh"

// Sockets
#include <sys/socket.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

namespace rtclient {

    using sockfd = int;

    // peers: the peeps I be talkin to
    struct peer {
        struct sockaddr_in6 addr;
        std::string name;
    };
    using peer_list = std::vector<struct peer>;

    // Window: info to display to the user
    class window {
        public:
            class msg_state {
                public:
                    std::string msg;
                    struct peer owner;
            };
            std::vector<msg_state> msgs;    // msgs from peers
            std::string my_msg;             // msg from me
            std::string my_name;
            std::mutex mtx;                // since multithreading
    };

    // A thread-safe (hopefully) queue of letters to send
    struct to_send {
        std::deque<char> letters;
        std::mutex mtx;
    };
}

bool operator==(const rtclient::peer &a, const rtclient::peer &b);