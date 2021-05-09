#include <vector>
#include "../config.hh"
#include <mutex>
#include <shared_mutex>
#include <string>

#include <sys/socket.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>


class sstate {
    public:
        using sockfd = int;
        using id_t = int;
        struct user_info {
            std::string name;
            sockaddr_in6 addr;
            sockfd sock;
        };
        struct room {
            std::vector<user_info> users;
            id_t id;
        };
        using rmls = std::vector<room>;
    private:
        rmls rooms;
        id_t next_id;
        std::shared_mutex mtx;
    public:
        sstate();
        ~sstate();
        std::string to_string();
        std::string rm_string(id_t id);
        id_t new_room(user_info user);
        void add_to_room(id_t id, user_info user);
        void rem_from_room(id_t id, user_info user);
};