#include "main.h"
#include "util.h"
MPI_Datatype MPI_PAKIET_T;

/*
 * w util.h extern state_t stan (czyli zapowiedź, że gdzieś tam jest definicja
 * tutaj w util.c state_t stan (czyli faktyczna definicja)
 */
state_t stan = InFree;

/* zamek wokół zmiennej współdzielonej między wątkami.
 * Zwróćcie uwagę, że każdy proces ma osobą pamięć, ale w ramach jednego
 * procesu wątki współdzielą zmienne - więc dostęp do nich powinien
 * być obwarowany muteksami
 */
pthread_mutex_t stateMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lamportMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t progressStateMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ackCountMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t priorityMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t handsomenessMut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t searchForPartnerBufferMut = PTHREAD_MUTEX_INITIALIZER;

struct tagNames_t
{
    const char *name;
    int tag;
} tagNames[] = {{"ACK", ACK}, {"NACK", NACK}, {"REQUEST", REQUEST}, {"RELEASE", RELEASE}};
//} tagNames[] = {{"potwierdzenie", ACK}, {"odrzucenie", NACK}, {"prośbę o sekcję krytyczną", REQUEST}, {"zwolnienie sekcji krytycznej", RELEASE}};

const char *const tag2string(int tag)
{
    for (int i = 0; i < sizeof(tagNames) / sizeof(struct tagNames_t); i++)
    {
        if (tagNames[i].tag == tag)
            return tagNames[i].name;
    }
    return "<unknown>";
}
/* tworzy typ MPI_PAKIET_T
 */
void inicjuj_typ_pakietu()
{
    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    int blocklengths[NITEMS] = {1, 1, 1, 1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint offsets[NITEMS];

    offsets[0] = offsetof(packet_t, ts);
    offsets[1] = offsetof(packet_t, src);
    offsets[2] = offsetof(packet_t, progress);
    offsets[3] = offsetof(packet_t, position);

    MPI_Type_create_struct(NITEMS, blocklengths, offsets, typy, &MPI_PAKIET_T);

    MPI_Type_commit(&MPI_PAKIET_T);
}

/* opis patrz util.h */
void sendPacket(packet_t *pkt, int destination, int tag)
{
    changeLamport(0);
    pkt->src = rank;
    MPI_Send(pkt, 1, MPI_PAKIET_T, destination, tag, MPI_COMM_WORLD);
    debug("Wysyłam %s do %d\n", tag2string(tag), destination);
}

void sendPacketToRole(packet_t *pkt, int tag, roles packetrole)
{
    int minSend = 0, maxSend = 0;
    if (packetrole == Gitarzysta)
    {
        maxSend = gitarzysci;
    }
    else if (packetrole == Tancerka)
    {
        minSend = gitarzysci;
        maxSend = gitarzysci + tancerki;
    }
    else
    {
        minSend = gitarzysci + tancerki;
        maxSend = size;
    }

    for (int i = minSend; i < maxSend; i++)
    {
        if (i != rank)
            sendPacket(pkt, i, tag);
    }
}

void changeState(state_t newState)
{
    pthread_mutex_lock(&stateMut);
    /*     if (stan == InFinish)
        {
            pthread_mutex_unlock(&stateMut);
            return;
        } */
    stan = newState;
    pthread_mutex_unlock(&stateMut);
}

void changeProgressState(progressStates newProgressState)
{
    pthread_mutex_lock(&progressStateMut);
    progressState = newProgressState;
    pthread_mutex_unlock(&progressStateMut);
}

void setPriority()
{
    pthread_mutex_lock(&priorityMut);
    pthread_mutex_lock(&lamportMut);
    priority = lamport;
    pthread_mutex_unlock(&lamportMut);
    pthread_mutex_unlock(&priorityMut);
}

void changeLamport(int newLamportClock)
{
    pthread_mutex_lock(&lamportMut);
    if (newLamportClock > lamport)
    {
        lamport = newLamportClock + 1;
    }
    else
    {
        lamport++;
    }

    pthread_mutex_unlock(&lamportMut);
}

void changeAckCount(int incAckCount)
{
    pthread_mutex_lock(&ackCountMut);
    ackCount += incAckCount;
    pthread_mutex_unlock(&ackCountMut);
}

void resetAckCount()
{
    pthread_mutex_lock(&ackCountMut);
    ackCount = 0;
    pthread_mutex_unlock(&ackCountMut);
}

void changeSearchForPartnerBuffer(int index, int value)
{
    pthread_mutex_lock(&searchForPartnerBufferMut);
    searchForPartnerBuffer[index] = value;
    pthread_mutex_unlock(&searchForPartnerBufferMut);
}

void changeHandsomeness(int newHandsomeness)
{
    pthread_mutex_lock(&handsomenessMut);
    handsomeness = newHandsomeness;
    pthread_mutex_unlock(&handsomenessMut);
}
