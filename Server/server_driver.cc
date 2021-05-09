#include "server_state.hh"

#include <thread>

sstate::sockfd startup(){
    sockaddr_in6 my_addr;
    sstate::sockfd master = socket(AF_INET6, SOCK_STREAM, 0);
    my_addr.sin6_family = AF_INET6;
    my_addr.sin6_port = htons(SERVER_PORT);
    my_addr.sin6_addr = in6addr_any;
    bind(master, (sockaddr *)&my_addr, sizeof(my_addr));
    listen(master, SV_QUEUE_CAP);
    return master;
}

void listen_for_kill(bool &stop) {
    std::string uinput;
    while(!stop){
        std::cout << "Type \"kill\" to turn off the server" << std::endl;
        std::cin >> uinput;
        if (uinput == "kill\n") {
            stop = true;
            break;
        }
    }
}

void handle_client(sstate::sockfd sock, sockaddr_in6 addr, sstate &S, bool &stop) {
    // Receive user's name
    char buf[MSG_LEN];
    recv(sock, buf, MSG_LEN, 0);
    std::string name(buf);
    std::cout << name << std::endl;
    sstate::user_info user{name, addr, sock};
    // Send the list of open rooms
    std::string rmstr = S.to_string();
    while(rmstr.length() >= MSG_LEN) {
        std::cout << rmstr.substr(0,MSG_LEN) << std::endl;
        send(sock, rmstr.substr(0,MSG_LEN).c_str(), MSG_LEN, 0);
        rmstr = rmstr.substr(MSG_LEN);
    }
    std::cout << rmstr.substr(0,MSG_LEN) << std::endl;
    send(sock, (rmstr + END_OF_LIST).c_str(), rmstr.length() + 1, 0);
    // Receive the selected room
    recv(sock, buf, MSG_LEN, 0);
    sstate::id_t id = atoi(buf);
    std::cout << buf << std::endl;
    std::string memberstr = S.rm_string(id);
    // Let everyone know about the new user, and the new guy about everyone else
    S.add_to_room(id, user);
    while(memberstr.length() >= MSG_LEN) {
        send(sock, memberstr.substr(0,MSG_LEN).c_str(), MSG_LEN, 0);
        memberstr = memberstr.substr(MSG_LEN);
    }
    send(sock, (memberstr + END_OF_LIST).c_str(), memberstr.length() + 1, 0);
    // Wait for a "BYE"
    std::string msg = "";
    while(!(stop || msg.starts_with(BYE))){
        recv(sock, buf, MSG_LEN, 0);
    }
    // If I exited and the server is still going, tell about the BYE
    if(!stop){
        S.rem_from_room(id, user);
    }
    close(sock);
}

void listen_for_connections(bool &stop, sstate::sockfd master, sstate &S){
    while(!stop){
        sockaddr_in6 new_addr;
        socklen_t addrsize = sizeof(new_addr);
        sstate::sockfd new_conn = accept(master, (sockaddr *)&new_addr, &addrsize);
        std::thread handler { [new_conn, new_addr, &S, &stop](){ handle_client(new_conn, new_addr, S, stop);}};
    }
}

int main(){
    auto master = startup();
    bool stop = false;
    sstate S{};
    std::thread kill_switch {[&stop](){listen_for_kill(stop);}};
    listen_for_connections(stop, master, S);
    kill_switch.join();
    close(master);
}