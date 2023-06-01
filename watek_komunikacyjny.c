#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet, odpowiedz;

    odpowiedz.src = rank;
    odpowiedz.ts = priority;

    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while (TRUE)
    {
        // debug("czekam na recv");
        MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        changeLamport(pakiet.ts);
        switch (status.MPI_TAG) // w sensie TYPY PAKIETÓW z util.h
        {
        case REQUEST:
            odpowiedz.progress = pakiet.progress;
            if (pakiet.progress == checkingPosition)
            {
                odpowiedz.position = handsomeness;
            }
            else if (pakiet.progress == checkingPositionForCritic)
            {
                odpowiedz.position = criticPosition;
            }

            if (stan == InMonitor)
            {
                switch (pakiet.progress)
                {
                case checkingPosition:
                    if (progressState == pakiet.progress)
                    {
                        if (priority > pakiet.ts || (priority == pakiet.ts && pakiet.src < rank))
                        {
                            sendPacket(&odpowiedz, pakiet.src, ACK);
                        }
                        else
                        {
                            sendPacket(&odpowiedz, pakiet.src, NACK);
                        }
                    }
                    else
                    {
                        sendPacket(&odpowiedz, pakiet.src, ACK);
                    }
                    break;
                case searchingForPartner:
                    changeSearchForPartnerBuffer(pakiet.src, pakiet.position);
                    break;
                case checkingPositionForCritic:
                    if (progressState == pakiet.progress)
                    {
                        if (priority > pakiet.ts || (priority == pakiet.ts && pakiet.src < rank))
                        {
                            sendPacket(&odpowiedz, pakiet.src, ACK);
                        }
                        else
                        {
                            sendPacket(&odpowiedz, pakiet.src, NACK);
                        }
                    }
                    else
                    {
                        sendPacket(&odpowiedz, pakiet.src, ACK);
                    }
                    break;
                case searchingForCritic:
                    changeSearchForPartnerCriticBuffer(pakiet.src, pakiet.position);
                    break;
                case searchingForRoom:
                    if (progressState == pakiet.progress)
                    {
                        if (priority < pakiet.ts || (priority == pakiet.ts && pakiet.src > rank))
                        {
                            changeWantRoomBuffer(pakiet.src, TRUE);
                        }
                        else
                        {
                            sendPacket(&odpowiedz, pakiet.src, ACK);
                        }
                    }
                    else if (progressState == dancing)
                    {
                        changeWantRoomBuffer(pakiet.src, TRUE);
                    }
                    else
                    {
                        sendPacket(&odpowiedz, pakiet.src, ACK);
                    }
                    break;
                }
            }
            else if (pakiet.progress == searchingForPartner || pakiet.progress == searchingForCritic)
            {
                changeSearchForPartnerCriticBuffer(pakiet.src, pakiet.position);
            }
            else if (pakiet.progress == searchingForRoom && foundRoom)
            {
                changeWantRoomBuffer(pakiet.src, TRUE);
            }
            else
            {
                sendPacket(&odpowiedz, pakiet.src, ACK);
                break;
            }
            break;

        case ACK:
            if (pakiet.progress == progressState)
            {
                switch (pakiet.progress)
                {
                case checkingPosition:
                    changeHandsomeness(maxPos(handsomeness, pakiet.position));
                    changeAckCount(1);
                    break;
                case searchingForPartner:
                    dancePartner = pakiet.src;
                    changeAckCount(1);
                    break;
                case checkingPositionForCritic:
                    changeCriticPosition(maxPos(criticPosition, pakiet.position));
                    changeAckCount(1);
                    break;
                case searchingForCritic:
                    danceCritic = pakiet.src;
                    changeAckCount(1);
                    break;
                case searchingForRoom:
                    changeAckCount(1);
                    break;
                }
            }
            break;
        case NACK:
            if (pakiet.progress == progressState)
            {
                switch (pakiet.progress)
                {
                case checkingPosition:
                    changeHandsomeness(maxPos(handsomeness, pakiet.position));
                    changeAckCount(1);
                    lessHandsomeBy++; // Kiedy dostajemy NACK to znaczy, że ktoś jest od nas przystojniejszy
                    break;
                case checkingPositionForCritic:
                    changeCriticPosition(maxPos(criticPosition, pakiet.position));
                    changeAckCount(1);
                    worseInCriticPosition++;
                    break;
                case searchingForRoom:
                    changeAckCount(1);
                    break;
                }
            }
            break;
        case RELEASE:
            endedDancing = TRUE;
            break;
        }
    }
}
