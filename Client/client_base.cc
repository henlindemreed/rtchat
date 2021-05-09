#include "client_base.hh"
#include <cstring>

bool operator==(const rtclient::peer &a, const rtclient::peer &b) {
    return std::memcmp(&a, &b, sizeof(rtclient::peer)) == 0;
}