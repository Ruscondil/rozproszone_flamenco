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
			checkPositionCritic();
			searchForCritic();
			searchForRoom();
			dance();
			break;
		}
		case Tancerka:
		{
			checkPosition();
			searchForPartner();
			waitForDanceEnd();
			break;
		}
		case Krytyk:
		{
			checkPositionCritic();
			searchForCritic();
			waitForDanceEnd();
			break;
		}
		}
	}
}

void checkPosition()
{
	changeProgressState(checkingPosition);
	setPriority();
	changeHandsomeness(lastHandsomeness);

	println("Sprawdzam swoja pozycje");
	packet_t *pkt = malloc(sizeof(packet_t));
	pkt->ts = priority;
	pkt->position = handsomeness;
	pkt->progress = checkingPosition;

	lessHandsomeBy = 1;
	dancePartner = -1;

	resetAckCount();

	changeState(InSend);
	sendPacketToRole(pkt, REQUEST, role);

	changeState(InMonitor);
	int maxAck = (role == Gitarzysta) ? gitarzysci : tancerki;
	while (stan != InFree)
	{
		if (ackCount >= maxAck - 1)
		{
			changeState(InFree);
			changeHandsomeness(handsomeness + lessHandsomeBy);
		}
		sleep(SEC_IN_STATE);
	}
	free(pkt);
}

void searchForPartner()
{

	changeProgressState(searchingForPartner);
	setPriority();
	packet_t *pkt = malloc(sizeof(packet_t));

	pkt->ts = priority;
	pkt->position = handsomeness;
	pkt->progress = searchingForPartner;

	resetAckCount();
	int minSend = 0, maxSend = 0;
	changeState(InSend);

	if (role == Gitarzysta)
	{
		println("Ubiegam się o partnerkę");
		sendPacketToRole(pkt, REQUEST, Tancerka);
		minSend = gitarzysci;
		maxSend = gitarzysci + tancerki;
	}
	else if (role == Tancerka)
	{
		println("Ubiegam się o partnera");
		sendPacketToRole(pkt, REQUEST, Gitarzysta);
		maxSend = gitarzysci;
	}

	changeState(InMonitor);
	do
	{
		for (int i = minSend; i < maxSend; i++)
		{
			if (searchForPartnerCriticBuffer[i] != -1)
			{
				if (searchForPartnerCriticBuffer[i] == handsomeness)
				{
					sendPacket(pkt, i, ACK);
					changeSearchForPartnerCriticBuffer(i, -1);
				}
				else if (searchForPartnerCriticBuffer[i] < handsomeness)
				{
					changeSearchForPartnerCriticBuffer(i, -1);
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

	println("Będę tańczyć z %d", dancePartner);
	free(pkt);
}

void checkPositionCritic()
{
	changeProgressState(checkingPositionForCritic);
	setPriority();
	changeCriticPosition(lastCriticPosition);

	println("Sprawdzam swoja pozycje dla krytyka");
	packet_t *pkt = malloc(sizeof(packet_t));
	pkt->ts = priority;
	pkt->position = criticPosition;
	pkt->progress = checkingPositionForCritic;

	worseInCriticPosition = 1;
	danceCritic = -1;

	resetAckCount();

	changeState(InSend);
	sendPacketToRole(pkt, REQUEST, role);
	changeState(InMonitor);

	int maxAck = (role == Gitarzysta) ? gitarzysci : krytycy;

	while (stan != InFree)
	{
		if (ackCount >= maxAck - 1)
		{
			changeState(InFree);
			changeCriticPosition(criticPosition + worseInCriticPosition);
		}
		sleep(SEC_IN_STATE);
	}
	free(pkt);
}

void searchForCritic()
{
	changeProgressState(searchingForCritic);
	setPriority();

	packet_t *pkt = malloc(sizeof(packet_t));

	pkt->ts = priority;
	pkt->position = criticPosition;
	pkt->progress = searchingForCritic;

	resetAckCount();
	int minSend = 0, maxSend = 0;
	changeState(InSend);

	if (role == Gitarzysta)
	{
		println("Ubiegam się o krytyka");
		sendPacketToRole(pkt, REQUEST, Krytyk);
		minSend = gitarzysci + tancerki;
		maxSend = size;
	}
	else if (role == Krytyk)
	{
		println("Ubiegam się o gitarzystę do krytykowania");
		sendPacketToRole(pkt, REQUEST, Gitarzysta);
		maxSend = gitarzysci;
	}
	changeState(InMonitor);

	do
	{
		for (int i = minSend; i < maxSend; i++)
		{
			if (searchForPartnerCriticBuffer[i] != -1)
			{
				if (searchForPartnerCriticBuffer[i] == criticPosition)
				{
					sendPacket(pkt, i, ACK);
					changeSearchForPartnerCriticBuffer(i, -1);
				}
				else if (searchForPartnerCriticBuffer[i] < criticPosition)
				{
					changeSearchForPartnerCriticBuffer(i, -1);
				}
			}
		}
		if (ackCount != 0)
		{
			lastCriticPosition = criticPosition;
			changeState(InFree);
		}
		sleep(SEC_IN_STATE);
	} while (stan != InFree);

	println("Będę krytykować razem z %d", danceCritic);
	free(pkt);
}

void searchForRoom()
{
	changeProgressState(searchingForRoom);
	setPriority();
	println("Ubiegam się o salę");

	packet_t *pkt = malloc(sizeof(packet_t));
	pkt->progress = searchingForRoom;
	pkt->ts = priority;

	resetAckCount();

	changeState(InSend);
	sendPacketToRole(pkt, REQUEST, Gitarzysta);

	changeState(InMonitor);
	while (stan != InFree)
	{
		if (sale - (gitarzysci - ackCount - 1) > 0)
		{
			println("Zarezerwowałem salę");
			foundRoom = TRUE;
			changeState(InFree);
		}
		sleep(SEC_IN_STATE);
	}

	free(pkt);
}

void dance()
{
	changeProgressState(dancing);
	setPriority();
	println("Tańczę z %d", dancePartner);
	packet_t *pkt = malloc(sizeof(packet_t));

	pkt->ts = priority;

	changeState(InSend);
	foundRoom = FALSE;
	println("Już nie zajmuję sali");
	pkt->progress = searchingForRoom;
	for (int i = 0; i < gitarzysci; i++)
	{
		if (wantRoomBuffer[i])
		{
			changeWantRoomBuffer(i, FALSE);
			sendPacket(pkt, i, ACK);
		}
	}

	pkt->progress = dancing;
	sendPacket(pkt, dancePartner, RELEASE);
	sendPacket(pkt, danceCritic, RELEASE);

	changeState(InFree);
	println("Kończę taniec z %d", dancePartner);

	sleep(SEC_IN_STATE);
	free(pkt);
}

void waitForDanceEnd()
{
	if (role == Tancerka)
	{
		println("Czekam na koniec tańca z %d", dancePartner);
	}
	else
	{
		println("Czekam na krytykowanie tańca %d", danceCritic);
	}

	changeProgressState(dancing);
	setPriority();

	changeState(InMonitor);
	while (!endedDancing)
	{
		sleep(SEC_IN_STATE);
	};
	if (role == Tancerka)
	{
		println("Kończę taniec z %d", dancePartner);
	}
	else
	{
		println("Kończę krytykować taniec %d", danceCritic);
	}
	endedDancing = FALSE;
	changeState(InFree);
}