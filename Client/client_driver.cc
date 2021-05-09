#include "broadcast.hh"
#include "handle_server.hh"
#include "display.hh"
#include <ncurses.h>
#include <chrono>


void sync_timer(rtclient::window &window, rtclient::peer_list &peers, 
                rtclient::sockfd out_sock, std::shared_mutex &peer_mtx, bool &stop) {
    while(!stop){
        std::this_thread::sleep_for(std::chrono::milliseconds(SYNC_INTVL));
        std::string msg = construct_msg(window.my_msg, SYNC, window.my_name);
        broadcast(peers, msg, out_sock, peer_mtx);
    }
}

void listen_for_chat(rtclient::window &window, bool &stop) {
    rtclient::sockfd in_sock = socket(AF_INET6, SOCK_DGRAM, 0);
    struct sockaddr_in6 my_addr, peer_addr; 
    my_addr.sin6_family = AF_INET6;
    my_addr.sin6_addr = in6addr_any;
    my_addr.sin6_port = htons(CLIENT_RCV_PORT);
    bind(in_sock, (struct sockaddr *)&my_addr, sizeof(my_addr));
    char buf[1024];
    socklen_t addr_size = sizeof(peer_addr);
    while(!stop ){
        recvfrom(in_sock, buf, 1024, 0, (struct sockaddr*)&peer_addr, &addr_size);
        std::string msg(buf);
        std::string name = msg.substr(0,msg.find(SEP));
        msg = msg.substr(msg.find(SEP) + 1);
        std::string verb = msg.substr(0,msg.find(SEP));
        std::string payload = msg.substr(msg.find(SEP));
        rtclient::peer sender{peer_addr, msg.substr(0,msg.find(SEP))};
        if(verb == SYNC){
            display::write(window, payload, sender);
        } else if(verb == CH){
            if(payload[0] == DELCH) {
                display::del(window, sender);
            } else {
                display::write(window, payload[0], sender);
            }
        }
    }
    close(in_sock);
}


int main() {
    // Start the screen
    initscr();
    keypad(stdscr, TRUE);
    rtclient::peer_list peers(0);
    std::shared_mutex peer_mtx;
    rtclient::window window;
    rtclient::sockfd server = server::init(peers, window.my_name);
    rtclient::sockfd out_sock = socket(AF_INET6, SOCK_DGRAM, 0);
    rtclient::to_send to_send;
    display::init(window, peers);
    bool stop = false;
    // Spawn a thread to handle each part
    std::thread keys_thread{ [&window, &peers, &peer_mtx, out_sock, &stop](){
                                    display::listen_for_keys(window, peers, peer_mtx, out_sock, stop);}};
    std::thread server_thread{ [server, &window, &peers, &peer_mtx, &stop](){
                                    server::listen_for_TCP(server, window, peers, peer_mtx, stop);}};
    std::thread sync_timer_thread{ [&window, &peers, out_sock, &peer_mtx, &stop](){
                                    sync_timer(window, peers, out_sock, peer_mtx, stop);}};
    std::thread chat_listen_thread{ [&window, &stop](){listen_for_chat(window, stop);}};
    keys_thread.join();
    server_thread.join();
    sync_timer_thread.join();
    chat_listen_thread.join();
    server::leave_room(server);
}