#include "GameServer.h"
#include "GameWorld.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

GameServer::GameServer(int onPort, int maxClients)	{
	port		= onPort;
	clientMax	= maxClients;
	clientCount = 0;
	netHandle	= nullptr;
	Initialise();
}

GameServer::~GameServer()	{
	Shutdown();
}

void GameServer::Shutdown() {
	SendGlobalPacket(BasicNetworkMessages::Shutdown);
	enet_host_destroy(netHandle);
	netHandle = nullptr;
}

bool GameServer::Initialise() {
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = port;

	netHandle = enet_host_create(&address, clientMax, 1, 0, 0);

	if (!netHandle) {
		std::cout << __FUNCTION__ << " failed to create network handle!" << std::endl;
		return false;
	}
	return true;
}

bool GameServer::SendGlobalPacket(int msgID) 
{
	GamePacket packet;
	packet.type = msgID;
	return SendGlobalPacket(packet);
}

bool GameServer::SendGlobalPacket(GamePacket& packet) 
{
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
	enet_host_broadcast(netHandle, 0, dataPacket);
	return true;
}

void GameServer::ProcessPacket(GamePacket* packet, int playerID) {
	if (packet->type == Acknowledge_State) 
	{
		AcknowledgePacket* ackPacket = (AcknowledgePacket*) packet;
		playerStates[playerID] = ackPacket->stateID;
	}
}

void GameServer::UpdateServer() {

	if (!netHandle)
		return;

	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0) 
	{
		int type = event.type;
		ENetPeer* p = event.peer;
		int peer = p->incomingPeerID;

		if (type == ENET_EVENT_TYPE_CONNECT) 
		{
			int playerID = playerPeers.size();
			playerPeers[playerID] = p;
			playerStates[playerID] = 0;
		}
		else if (type == ENET_EVENT_TYPE_DISCONNECT) 
		{
			auto it = std::find_if(playerPeers.begin(), playerPeers.end(), [&](const auto& pair) { return pair.second == event.peer; });

			if (it != playerPeers.end()) 
				playerPeers.erase(it);
		}		
		else if (type == ENetEventType::ENET_EVENT_TYPE_RECEIVE) 
		{
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet, peer);
		}		
		enet_packet_destroy(event.packet);

	}
}

void GameServer::SetGameWorld(GameWorld &g) 
{
	gameWorld = &g;
}

bool GameServer::SendPacketToPeer(GamePacket* packet, int playerID) 
{
	auto it = playerPeers.find(playerID);
	if (it != playerPeers.end()) {
		ENetPacket* dataPacket = enet_packet_create(packet, packet->GetTotalSize(), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(it->second, 0, dataPacket);
	}
	return true;
}




