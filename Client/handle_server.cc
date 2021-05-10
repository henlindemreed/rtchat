#include "handle_server.hh"
#include "display.hh"
#include <ncurses.h>
#include <string.h>



std::string get_server_ip(){
    printw("Please enter the (IPv6) addr of the server you'd like to connect to.");
    mvprintw(1,0,"The default (enter nothing) is %s", SERVER_DEF_ADDR);
    move(2,0);
    refresh();
    char ip[80];
    getstr(ip);
    if(ip[0] == 0){
        return std::string(SERVER_DEF_ADDR);
    }
    return std::string(ip);
}

rtclient::sockfd connect_to_server(std::string ip){
    clear();
    mvprintw(0,0,"Connecting...");
    sleep(1);
    refresh();
    rtclient::sockfd server_socket = socket(AF_INET6, SOCK_STREAM, 0);
    struct sockaddr_in6 server_addr;
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(SERVER_PORT);
    inet_pton(AF_INET6, ip.c_str(), &server_addr.sin6_addr);
    mvprintw(1,0, ">>>");
    printw("%d", connect(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)));
    refresh();
    sleep(1);
    return server_socket;
}

void say_hi(rtclient::sockfd sock, std::string &my_name){
    clear();
    mvprintw(0,0, "Who are you?");
    move(1,0);
    refresh();
    char name[80];
    getstr(name);
    my_name = name;
    mvprintw(2,0,">>>%d",send(sock, name, strlen(name), 0));
    refresh();
    sleep(1);
}

void choose_room(rtclient::sockfd sock) {
    char buf[MSG_LEN];
    // Receive a list of room IDs and the names of their inhabitants
    // Formatted RID\SEP\NAME\SEP\NAME\SEP\nRID\SEP\NAME...\SEP\n
    std::string ls("");
    do {
        recv(sock, buf, MSG_LEN, 0);
        if(buf[0] != END_OF_LIST[0]){
            ls += buf;
        }
    } while(! strcmp(buf, END_OF_LIST));
    clear();
    mvaddstr(0,0,"Please type the number of the room you wish to join; 0 for a new room");
    // pretty print. yuck.
    int row = 1;
    size_t next_nl = ls.find('\n');
    std::vector<std::string> ids(0);
    while(next_nl != std::string::npos){
        move(row, 0);
        printw("[%d] ", row);
        std::string thisln = ls.substr(0,next_nl);
        ls = ls.substr(next_nl + 1);
        size_t next_sep = thisln.find(SEP);
        addstr(thisln.substr(0,next_sep).c_str());
        ids.push_back(thisln.substr(0, next_sep));
        addstr(": ");
        thisln = thisln.substr(next_sep + 1);
        next_sep = thisln.find(SEP);
        while(next_sep != std::string::npos){
            addstr(thisln.substr(0,next_sep).c_str());
            addstr(", ");
            thisln = thisln.substr(next_sep + 1);
            next_sep = thisln.find(SEP);
        }
        row++;
        next_nl = ls.find('\n');
    }
    move(row, 0);
    refresh();
    // get selection and send it back to server
    char selection[5];
    getstr(selection);
    std::string room_id;
    if(atoi(selection) == 0){
        room_id = "0";
    } else {
        room_id = ids[atoi(selection)];
    }
    send(sock, room_id.c_str(), room_id.length(), 0);
}

void learn_friends(rtclient::sockfd sock, rtclient::peer_list &peers){
    clear();
    char buf[1024];
    // Receive a list of members formatted like so:
    // name\SEP\addr\SEP\name\SEP\addr...addr\SEP
    std::string ls("");
    do {
        recv(sock, buf, 1024, 0);
        if(buf[0] != END_OF_LIST[0]){
            ls += buf;
        }
    } while(! strcmp(buf, END_OF_LIST));
    // Compute peers out of this; build in place on vector
    size_t next_sep = ls.find(SEP);
    while(next_sep != std::string::npos){
        std::string name = ls.substr(0,next_sep);
        ls = ls.substr(next_sep + 1);
        next_sep = ls.find(SEP);
        std::string addr = ls.substr(0, next_sep);
        ls = ls.substr(next_sep + 1);
        next_sep = ls.find(SEP);
        peers.emplace_back(rtclient::peer());
        peers.back().name = name;
        peers.back().addr.sin6_family = AF_INET6;
        peers.back().addr.sin6_port = htons(CLIENT_RCV_PORT);
        inet_pton(AF_INET6, addr.c_str(), &peers.back().addr.sin6_addr);
    }
}

// Talks to the server about joining a room
// fills the peer list with peers from the room
// returns the sockfd for the server
rtclient::sockfd server::init(rtclient::peer_list &peers, std::string &my_name){
    std::string ip = get_server_ip();
    rtclient::sockfd sock = connect_to_server(ip);
    say_hi(sock, my_name);
    choose_room(sock);
    learn_friends(sock, peers);
    return sock;
}

// Listens for messages from the server, which are
// notifications of people leaving and entering the
// room. Modifies peers as necessary
void server::listen_for_TCP(rtclient::sockfd sock, rtclient::window &window, 
                            rtclient::peer_list &peers, std::shared_mutex &peer_mtx, bool &stop){
    char buf[1024];
    while(!stop){
        recv(sock, buf, 1024, 0);
        std::string msg(buf);
        if(msg.starts_with(NEWUSER)){
            // parse
            auto peer_str = msg.substr(8);
            size_t sep = peer_str.find(SEP);
            auto name = peer_str.substr(0,sep);
            auto addr = peer_str.substr(sep + 1);
            // construct peer on end of list of peers; add to window
            std::unique_lock<std::shared_mutex> peer_lock(peer_mtx);
            peers.emplace_back(rtclient::peer());
            peers.back().name = name;
            peers.back().addr.sin6_family = AF_INET6;
            peers.back().addr.sin6_port = htons(CLIENT_RCV_PORT);
            inet_pton(AF_INET6, addr.c_str(), &peers.back().addr.sin6_addr);
            display::add(window, peers.back());
        } else if(msg.starts_with(LEAVEUSER)) {
            //parse
            auto peer_str = msg.substr(8);
            size_t sep = peer_str.find(SEP);
            auto name = peer_str.substr(0,sep);
            auto addr = peer_str.substr(sep + 1);
            //construct peer to drop to compare against
            rtclient::peer to_remove{};
            to_remove.name = name;
            to_remove.addr.sin6_family = AF_INET6;
            to_remove.addr.sin6_port = htons(CLIENT_RCV_PORT);
            inet_pton(AF_INET6, addr.c_str(), &to_remove.addr.sin6_addr);
            // remove from window and broadcast list
            display::remove(window, to_remove);
            for(size_t i = 0; i < peers.size(); ++i){
                if(peers[i] == to_remove){
                    std::unique_lock<std::shared_mutex> peer_lock(peer_mtx);
                    peers[i] = peers.back();
                    peers.pop_back();
                }
            }
        }
    }
}

// Tells server am leaving
void server::leave_room(rtclient::sockfd sock){
    auto msg = BYE;
    send(sock, msg, strlen(msg), 0);
    close(sock);
}

