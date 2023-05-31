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
                            break;
                        }
                        else
                        {
                            sendPacket(&odpowiedz, pakiet.src, NACK);
                            break;
                        }
                    }
                    else
                    {
                        sendPacket(&odpowiedz, pakiet.src, ACK);
                        break;
                    }
                    break;
                case searchingForPartner:
                    changeSearchForPartnerBuffer(pakiet.src, pakiet.position);
                    break;
                }
            }
            else if (pakiet.progress == searchingForPartner)
            {
                changeSearchForPartnerBuffer(pakiet.src, pakiet.position);
                break;
            }

            break;

        case ACK:
            if (pakiet.progress == progressState)
            {
                switch (pakiet.progress)
                {
                case checkingPosition:
                    changeHandsomeness(maxi(handsomeness, pakiet.position)); // TODO zmienić
                    changeAckCount(1);
                    break;
                case searchingForPartner:
                    dancePartner = pakiet.src;
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
                    changeHandsomeness(maxi(handsomeness, pakiet.position));
                    posUp++;
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
