#include "server_state.hh"
#include <algorithm>

// Constructor
sstate::sstate() :
 rooms{rmls(0)}, next_id{0}, mtx{}
{}

// Destructor
sstate::~sstate(){}


std::string sstate::to_string(){
    std::shared_lock<std::shared_mutex> state_lock(mtx);
    std::string ans = "";
    for(auto &rm : rooms) {
        ans += std::to_string(rm.id) + SEP;
        for(auto &user: rm.users) {
            ans += user.name + SEP;
        }
        ans += '\n';
    }
    return ans;
}

std::string sstate::rm_string(id_t id){
    std::shared_lock<std::shared_mutex> state_lock(mtx);
    std::string ans = "";
    auto rm = std::find_if(rooms.begin(), rooms.end(), [id](auto &x){return x.id == id;});
    for(auto &user : rm->users) {
        char buf[80];
        inet_ntop(AF_INET6, &user.addr.sin6_addr, buf, sizeof(buf));
        ans += user.name + SEP + buf + SEP;
    }
    return ans;
}

sstate::id_t sstate::new_room(sstate::user_info user){
    std::unique_lock<std::shared_mutex> state_lock(mtx);
    rooms.emplace_back(sstate::room{std::vector<sstate::user_info>(1), next_id});
    rooms.back().users.push_back(user);
    return next_id++;
}

void sstate::add_to_room(sstate::id_t id, sstate::user_info user){
    std::unique_lock<std::shared_mutex> state_lock(mtx);
    auto rm = std::find_if(rooms.begin(), rooms.end(), [id](auto &x){return x.id == id;});
    if(rm != rooms.end()) {
        char buf[80];
        inet_ntop(AF_INET6, &user.addr.sin6_addr, buf, sizeof(buf));
        std::string msg = std::string(NEWUSER) + SEP + user.name + SEP + buf;
        for(auto &user: rm->users){
            send(user.sock, msg.c_str(), msg.length(), 0);
        }
        rm->users.push_back(user);
    }
}

void sstate::rem_from_room(sstate::id_t id, sstate::user_info user){
    std::unique_lock<std::shared_mutex> state_lock(mtx);
    auto rm = std::find_if(rooms.begin(), rooms.end(), [id](auto &x){return x.id == id;});
    if(rm != rooms.end()) {
        rm->users.erase(std::find(rm->users.begin(), rm->users.end(), user));
        char buf[80];
        inet_ntop(AF_INET6, &user.addr.sin6_addr, buf, sizeof(buf));
        std::string msg = std::string(LEAVEUSER) + SEP + user.name + SEP + buf;
        for(auto &user: rm->users){
            send(user.sock, msg.c_str(), msg.length(), 0);
        }
    }
}

bool operator==(const sstate::room &a, const sstate::room &b){
    return a.id == b.id;
}

bool operator==(const sstate::user_info &a, const sstate::user_info &b){
    return a.sock == b.sock;
}