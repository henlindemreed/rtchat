#include "client_base.hh"

// broadcasts this message to everyone in my peer group
void broadcast(rtclient::peer_list peers, std::string msg, rtclient::sockfd out_sock, std::shared_mutex &peer_mtx);

// constructs a message in this format:
// name\SEP\(SYNC/CHAR)\SEP\payload
std::string construct_msg(std::string msg, const char* verb, std::string my_name);