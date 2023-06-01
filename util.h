#ifndef UTILH
#define UTILH
#include "main.h"

typedef enum
{
    checkingPosition,
    searchingForPartner,
    checkingPositionForCritic,
    searchingForCritic,
    searchingForRoom,
    dancing
} progressStates;

typedef enum
{
    InFree,
    InSend,
    InMonitor,
    InWait
} state_t;

typedef enum
{
    Gitarzysta,
    Tancerka,
    Krytyk,
    Unknown
} roles;

/* typ pakietu */
typedef struct
{
    int ts; /* timestamp (zegar lamporta) */
    int src;
    progressStates progress;
    int position;
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 4

/* Typy wiadomości */
/* TYPY PAKIETÓW */
#define ACK 1
#define NACK 2
#define REQUEST 3
#define RELEASE 4

extern MPI_Datatype MPI_PAKIET_T;

const char *tag2string(int tag);
const char *role2string(roles);

void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);
void sendPacketToRole(packet_t *pkt, int tag, roles packetrole);
int maxPos(int, int);

extern state_t stan;

extern pthread_mutex_t stateMut;
extern pthread_mutex_t progressStateMut;
extern pthread_mutex_t lamportMut;
extern pthread_mutex_t ackCountMut;
extern pthread_mutex_t priorityMut;
extern pthread_mutex_t handsomenessMut;
extern pthread_mutex_t searchForPartnerCriticBufferMut;
extern pthread_mutex_t criticPositionMut;
extern pthread_mutex_t wantRoomMut;

/* zmiana stanu, obwarowana muteksem */
void changeState(state_t);
void changeProgressState(progressStates);
void setPriority();
void changeLamport(int);
void changeAckCount(int);
void resetAckCount();
void changeSearchForPartnerCriticBuffer(int, int);
void changeHandsomeness(int);
void changeCriticPosition(int);
void changeWantRoomBuffer(int, int);
#endif
