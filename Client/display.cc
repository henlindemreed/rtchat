#include "display.hh"
#include <ncurses.h>

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
void display::write(rtclient::window &window, std::string msg, rtclient::peer &owner){
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    std::string clrstr(cols, ' ');
    std::lock_guard<std::mutex> winlock(window.mtx);
    size_t row = 2 * find_peer(window.msgs, owner);
    mvaddstr(row, 0, clrstr.c_str());
    mvaddstr(row, 0, msg.c_str());
    window.msgs[row/2].msg = msg;
}

// writes an extra character to the end of owner's message
// use this for one-char packets
void display::write(rtclient::window &window, char letter, rtclient::peer &owner){
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    std::lock_guard<std::mutex> winlock(window.mtx);
    size_t row = 2 * find_peer(window.msgs, owner);
    mvaddch(row, window.msgs[row/2].msg.length(), letter);
    window.msgs[row/2].msg += letter;
}

// deletes a character from the end of owner's message
// use this for one-char deletes
void display::del(rtclient::window &window, rtclient::peer &owner){

}

// adds a peer to the window
void display::add(rtclient::window &window, rtclient::peer new_peer){

}

// removes a peer from the window
void display::remove(rtclient::window &window, rtclient::peer old_peer){

}

// writes keypresses from user to both the window and the to_send queue
void display::listen_for_keys(rtclient::window &window, rtclient::to_send &to_send){

}