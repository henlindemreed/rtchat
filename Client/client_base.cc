#include "client_base.hh"
#include <cstring>

bool operator==(const rtclient::peer &a, const rtclient::peer &b) {
    if(a.name != b.name){
        return false;
    }
    return memcmp(&a.addr.sin6_addr, &b.addr.sin6_addr, sizeof(a.addr.sin6_addr)) == 0;
}