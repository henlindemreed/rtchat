#include "client_base.hh"


namespace server {
    // Talks to the server about joining a room
    // fills the peer list with peers from the room
    // returns the sockfd for the server
    rtclient::sockfd init(rtclient::peer_list &peers);

    // Listens for messages from the server, which are
    // notifications of people leaving and entering the
    // room. Modifies peers as necessary
    void listen_for_TCP(rtclient::window &window, rtclient::peer_list &peers, std::shared_mutex peer_mtx);

    // Tells server am leaving
    void leave_room();
}