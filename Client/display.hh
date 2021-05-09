#include "client_base.hh"

namespace display {
    // overwrites a message onto the screen. This may be the same message.
    // call this one for sync messages
    void write(rtclient::window &window, std::string &msg, rtclient::peer owner);

    // writes an extra character to the end of owner's message
    // use this for one-char packets
    void write(rtclient::window &window, char letter, rtclient::peer owner);

    // deletes a character from the end of owner's message
    // use this for one-char deletes
    void del(rtclient::window &window, rtclient::peer owner);

    // adds a peer to the window
    void add(rtclient::window &window, rtclient::peer new_peer);

    // removes a peer from the window
    void remove(rtclient::window &window, rtclient::peer old_peer);

    // writes keypresses from user to both the window and the to_send queue
    void listen_for_keys(rtclient::window &window, rtclient::to_send &to_send);
}