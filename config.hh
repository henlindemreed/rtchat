
// Ports
#define SERVER_PORT 0x5243
#define CLIENT_SEND_PORT 0x5244
#define CLIENT_RCV_PORT 0x5245

// Server constants
#define SV_QUEUE_CAP 3
#define MSG_LEN 1024

// Client-server message verbs
#define NEWUSER "NEWUSER\n"
#define BYE "BYE"
#define LEAVEUSER "GOODBYE\n"
#define ROOMS "OPENROOMS\n"