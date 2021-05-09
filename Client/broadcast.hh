#include "client_base.hh"

// broadcasts this message to everyone in my peer group
void broadcast(rtclient::peer_list peers, std::string msg, rtclient::sockfd out_sock, std::shared_mutex peer_mtx);