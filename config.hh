
// Ports & addrs
#define SERVER_PORT 0x5243
#define CLIENT_SEND_PORT 0x5244
#define CLIENT_RCV_PORT 0x5245
#define SERVER_DEF_ADDR "::1"

// Server constants
#define SV_QUEUE_CAP 3
#define MSG_LEN 512


// Client-server message verbs
#define NEWUSER "NEWUSER\n"
#define BYE "BYE"
#define LEAVEUSER "GOODBYE\n"
#define ROOMS "OPENROOMS\n"
#define SEP '\037'
#define END_OF_LIST "\004"

// Client constants
#define DELCH '\010'
#define EXIT_STR "\\leave"
#define SYNC_INTVL 1000         // # of milliseconds between sync broadcasts

// Client verbs
#define SYNC "S"
#define CH "C"