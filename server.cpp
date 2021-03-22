
// Modules
#include "config.hh"

// Sockets
#include <sys/socket.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

// Backend data structures
#include <vector>
#include <unordered_map>
#include <string>

struct user_info {
    struct sockaddr_in6 addr;
    std::string name;
};

// set of users who all talk to each other
struct chat_room {
    std::string name;
    std::vector<user_info> users;
    std::vector<int> user_socks;
};

// converts a ipv6 address to a std::string.
std::string inaddr_to_string(in6_addr &addr) {
    char addrbuf[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(addr), addrbuf, INET6_ADDRSTRLEN);
    return std::string(addrbuf);
}

// returns a string like so:
/*
"OPENROOMS
rm1
user1
user2
user3

rm2
usr1a
usr2a
usr3a

"
*/
// send these strings to connecting users deciding what room to join
std::string room_string(std::vector<chat_room> &rooms) {
    std::string ans = "OPENROOMS\n";
    for(auto &rm : rooms) {
        ans += rm.name + "\n";
        for(auto &usr : rm.users) {
            ans += usr.name + "\n";
        }
        ans += "\n";
    }
    return ans;
}


// Adds a new user to a room. First tell the new user about the old users and 
// the old users about the new one. Then promote the new user to an old user 
// (add it to the list of users the room keeps track of)
void add_to_room(struct chat_room &rm, struct user_info &usr, int new_sock) {
    const std::string new_addr_msg = "NEWUSER\n";
    auto old_msg = new_addr_msg + inaddr_to_string(usr.addr.sin6_addr);
    for(int i = 0; i < rm.users.size(); ++i) {
        // tell already-existing user about new guy
        send(rm.user_socks[i], old_msg.c_str(), old_msg.length(), 0);
        // tell new guy about already-existing guy
        std::string new_msg = new_addr_msg + inaddr_to_string(rm.users[i].addr.sin6_addr);
        send(new_sock, new_msg.c_str(), new_msg.length(), 0);
    }
    // add to room in server
    rm.users.push_back(usr);
    rm.user_socks.push_back(new_sock);
}

// When a new user requests to be in a room that doesn't exist, make one. Then put them in it.
void create_new_room(std::vector<chat_room> &rooms, struct user_info &usr, std::string name, int new_sock){
    struct chat_room new_room {name, std::vector<user_info>(0), std::vector<int>(0)};
    new_room.user_socks.push_back(new_sock);
    new_room.users.push_back(usr);
    rooms.push_back(new_room);
}

// What to do when someone new arrives?
// 1. ask them what room they want to join, of those open
// 2. if they give a room, add them to it, notify all members of that room and also the new member
//      of each other
// 3. if they give a room that doesn't exist, create a new room with them as the only member
// 4. (last) add this client's socket to the list of sockets.
void handle_new_connection(int new_sock, sockaddr_in6 *new_addr, 
                            std::vector<chat_room> &rooms, std::vector<int> &cli_socks) 
{
    char buffer[MSG_LEN] = {};
    int valread = read(new_sock, buffer, MSG_LEN);
    struct user_info user {*new_addr, std::string(buffer)};
    // step 1.
    std::string rmstr = room_string(rooms);
    send(new_sock, rmstr.c_str(), rmstr.length(), 0);
    valread = read(new_sock, buffer, MSG_LEN);
    std::string room {buffer};
    bool found_room = false;
    // step 2.
    for(auto &rm : rooms) {
        if(rm.name == room) {
            add_to_room(rm, user, new_sock);
            found_room = true;
            break;
        }
    }
    // step 3.
    if(!found_room) {
        create_new_room(rooms, user, room, new_sock);
    }
    // step 4.
    cli_socks.push_back(new_sock);

}

int main() {
    int master_sock;
    struct sockaddr_in6 addr, new_addr; 
    int opt = 1; 
    socklen_t addr_len = sizeof(addr);
    std::vector<int> cli_socks{0};
    std::vector<chat_room> rooms{0};

    // Create connection-accepting socket
    if (master_sock = socket(AF_INET6, SOCK_STREAM, 0) == 0){
        std::cerr << "socket failure";
        return 1;
    }

    // be able to accept multiple incoming connections or something. Just following a 
    // tutorial here.
    if (setsockopt(master_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "setsockopt"; 
        return 1;
    } 
    // my address
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_any;
    addr.sin6_port = htons(SERVER_PORT);

    bind(master_sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(master_sock, SV_QUEUE_CAP);

    // Main loop
    while(1) {
        // Accept incoming connection
        int new_sock = accept(master_sock, (struct sockaddr *)&new_addr, &addr_len);
        handle_new_connection(new_sock, new_addr, rooms, cli_socks);

    }




}