#include "broadcast.hh"

// broadcasts this message to everyone in my peer group
void broadcast(rtclient::peer_list peers, std::string msg, rtclient::sockfd out_sock, std::shared_mutex &peer_mtx){
    std::shared_lock<std::shared_mutex> read_lock(peer_mtx);
    for (auto &peer : peers) {
        // simply send it to each peer
        sendto(out_sock, msg.c_str(), msg.size(), 0, (struct sockaddr *)&peer.addr, sizeof(peer.addr));
    }
}

std::string construct_msg(std::string msg, const char* verb, std::string my_name){
    return my_name + SEP + verb + SEP + msg;
}