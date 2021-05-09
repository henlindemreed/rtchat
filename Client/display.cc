#include "display.hh"
#include <ncurses.h>
#include "broadcast.hh"

size_t find_peer(std::vector<rtclient::window::msg_state> &states, rtclient::peer &target){
    for(size_t i = 0; i < states.size(); ++i) {
        if(states[i].owner == target) {
            return i;
        }
    }
    return -1;
}

// overwrites a message onto the screen. This may be the same message.
// call this one for sync messages
void display::write(rtclient::window &window, std::string msg, rtclient::peer owner){
    int cols = getmaxx(stdscr);
    std::string clrstr(cols, ' ');
    std::lock_guard<std::mutex> winlock(window.mtx);
    size_t row = 2 * (find_peer(window.msgs, owner) + 1) + 1;
    mvaddstr(row, 0, clrstr.c_str());
    mvaddstr(row, 0, msg.c_str());
    window.msgs[row/2].msg = msg;
    refresh();
}

// writes an extra character to the end of owner's message
// use this for one-char packets
void display::write(rtclient::window &window, char letter, rtclient::peer owner){
    std::lock_guard<std::mutex> winlock(window.mtx);
    size_t row = 2 * (find_peer(window.msgs, owner) + 1) + 1;
    mvaddch(row, window.msgs[row/2].msg.length(), letter);
    window.msgs[row/2].msg += letter;
    refresh();
}

// deletes a character from the end of owner's message
// use this for one-char deletes
void display::del(rtclient::window &window, rtclient::peer owner){
    std::lock_guard<std::mutex> winlock(window.mtx);
    size_t row = 2 * (find_peer(window.msgs, owner) + 1) + 1;
    mvaddch(row, window.msgs[row/2].msg.length()-1, ' ');
    window.msgs[row/2].msg.pop_back();
    refresh();
}

// adds a peer to the window
void display::add(rtclient::window &window, rtclient::peer new_peer){
    std::lock_guard<std::mutex> winlock(window.mtx);
    mvaddstr(2 * window.msgs.size(), 0, "> ");
    addstr(new_peer.name.c_str());
    addch(':');
    window.msgs.emplace_back(rtclient::window::msg_state{"", new_peer});
    refresh();
}

// removes a peer from the window
void display::remove(rtclient::window &window, rtclient::peer old_peer){
    std::lock_guard<std::mutex> winlock(window.mtx);
    size_t i = find_peer(window.msgs, old_peer) + 1;
    move(2 * i, 0);
    deleteln();
    deleteln();
    window.msgs.erase(window.msgs.begin() + i);
    refresh();
}

// writes keypresses from user to both the window and the to_send queue
void display::listen_for_keys(rtclient::window &window, rtclient::peer_list &peers, std::shared_mutex &peer_mtx, 
                                rtclient::sockfd out_sock, bool &stop){
    while(! stop) {
        int c = getch();
        if (c == KEY_BACKSPACE || c == KEY_DC || c == 127 || c == 7 || c == 8) {
            {   // Scope for winlock
                std::lock_guard<std::mutex> winlock(window.mtx);
                mvdelch(0, window.my_msg.length() - 1);
                window.my_msg.pop_back();
            }{  // Scope for to_send lock
                std::string msg = construct_msg(std::string(1,DELCH), CH, window.my_name);
                broadcast(peers, msg, out_sock, peer_mtx);
            }
        } else {
            {   // Scope for winlock
                std::lock_guard<std::mutex> winlock(window.mtx);
                mvaddch(0, window.my_msg.length(), c);
                window.my_msg += c;
            }{  // Scope for to_send lock
                std::string msg = construct_msg(std::string(1,c), CH, window.my_name);
                broadcast(peers, msg, out_sock, peer_mtx);
            }
        }
        refresh();
        if (window.my_msg.ends_with(EXIT_STR)) {
            stop = true;
        }
    }
}

void display::init(rtclient::window &window, rtclient::peer_list &peers){
    clear();
    mvaddstr(0,0,"Type away");
    window.my_msg = "";
    for(size_t i = 0; i < peers.size(); ++i){
        window.msgs.emplace_back(rtclient::window::msg_state{"", peers[i]});
        mvaddstr(2 * (i + 1), 0, peers[i].name.c_str());
    }
    refresh();
}