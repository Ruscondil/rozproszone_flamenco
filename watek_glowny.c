#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
	srandom(rank);
	int tag;
	int perc;
	while (TRUE)
	{
		switch (role)
		{
		case Gitarzysta:
		{
			checkPosition();
			searchForPartner();
			// checkPosition();
			// searchForCritic();
			// searchForRoom();
			dance();
			break;
		}
		case Tancerka:
		{
			checkPosition();
			searchForPartner();
			waitForDanceEnd();
			//  czekanie na zakończenie roboty
			break;
		}
		case Krytyk:
		{
			break;
		}
		}
	}
}

void checkPosition()
{
	setPriority();
	changeProgressState(checkingPosition);
	println("Sprawdzam swoja pozycje");
	packet_t *pkt = malloc(sizeof(packet_t));
	pkt->ts = priority;
	pkt->position = handsomeness;
	pkt->progress = checkingPosition;

	posUp = 1;
	dancePartner = -1;

	changeHandsomeness(lastHandsomeness);
	// int prior_zapyt = lamport;
	// pkt->data = prior_zapyt;
	resetAckCount();

	changeState(InSend);
	sendPacketToRole(pkt, REQUEST, role);
	changeState(InMonitor);
	int maxAck = 0;
	if (role == Gitarzysta)
		maxAck = gitarzysci;
	else if (role == Tancerka)
	{
		maxAck = tancerki;
	}
	while (stan != InFree)
	{
		if (ackCount >= maxAck - 1)
		{
			changeState(InFree);
			changeHandsomeness(handsomeness + posUp);
		}
		sleep(SEC_IN_STATE);
	}
	free(pkt);
}

void searchForPartner()
{
	setPriority();
	changeProgressState(searchingForPartner);
	println("Ubiegam się o partnera"); // TODO w zależności od roli

	packet_t *pkt = malloc(sizeof(packet_t));

	pkt->ts = priority;
	pkt->position = handsomeness;
	pkt->progress = searchingForPartner;

	resetAckCount();

	changeState(InSend);
	if (role == Gitarzysta)
	{
		sendPacketToRole(pkt, REQUEST, Tancerka);
	}
	else if (role == Tancerka)
	{
		sendPacketToRole(pkt, REQUEST, Gitarzysta);
	}
	changeState(InMonitor);

	int minSend = 0, maxSend = 0;
	if (role == Tancerka)
	{
		maxSend = gitarzysci;
	}
	else if (role == Gitarzysta)
	{
		minSend = gitarzysci;
		maxSend = gitarzysci + tancerki;
	}

	do
	{
		for (int i = minSend; i < maxSend; i++)
		{
			if (searchForPartnerBuffer[i] != -1)
			{
				if (searchForPartnerBuffer[i] == handsomeness)
				{
					sendPacket(pkt, i, ACK);
					changeSearchForPartnerBuffer(i, -1);
				}
				else if (searchForPartnerBuffer[i] < handsomeness)
				{
					changeSearchForPartnerBuffer(i, -1);
				}
			}
		}
		if (ackCount != 0)
		{
			lastHandsomeness = handsomeness;
			changeState(InFree);
		}
		sleep(SEC_IN_STATE);
	} while (stan != InFree);

	println("Dobrałem się z %d", dancePartner);
	free(pkt);
}

void searchForCritic()
{
}

void searchForRoom()
{
	/* debug("Ubiegam się o salę");
	packet_t *pkt = malloc(sizeof(packet_t));
	int prior_zapyt = lamport;
	pkt->data = prior_zapyt;

	// changeState(InSend);
	for (int i = 0; i < size; i++)
	{
		if (i == rank)
			continue;
		sendPacket(pkt, i, REQUEST);
	}
	changeState(InMonitor);

	while (stan != InFree)
	{
		if (ackCount == gitarzysci - sale)
		{
			changeState(InSection);
		}
		sleep(SEC_IN_STATE);
	}

	free(pkt); */
}

void dance()
{
	setPriority();
	changeProgressState(dancing);
	packet_t *pkt = malloc(sizeof(packet_t));
	println("Tańczę z %d", dancePartner);

	pkt->progress = dancing;
	pkt->ts = priority;

	changeState(InSend);
	sendPacket(pkt, dancePartner, RELEASE);
	changeState(InFree);

	sleep(SEC_IN_STATE);
	free(pkt);
}

void waitForDanceEnd()
{
	println("Czekam na koniec tańca z %d", dancePartner);
	changeProgressState(dancing);
	setPriority();

	changeState(InMonitor);
	while (!endedDancing)
	{
		sleep(SEC_IN_STATE);
	};
	println("Kończę taniec z %d", dancePartner);
	endedDancing = FALSE;
}