#include "client_base.hh"


namespace server {
    // Talks to the server about joining a room
    // fills the peer list with peers from the room
    // returns the sockfd for the server.
    rtclient::sockfd init(rtclient::peer_list &peers, std::string &my_name);

    // Listens for messages from the server, which are
    // notifications of people leaving and entering the
    // room. Modifies peers as necessary
    void listen_for_TCP(rtclient::sockfd sock, rtclient::window &window, 
                        rtclient::peer_list &peers, std::shared_mutex &peer_mtx, bool &stop);

    // Tells server am leaving
    void leave_room(rtclient::sockfd sock);
}