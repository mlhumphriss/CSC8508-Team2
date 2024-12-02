#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include "RenderObject.h"


#define COLLISION_MSG 30

struct MessagePacket : public GamePacket {
	short playerID;
	short messageID;

	MessagePacket() {
		type = Message;
		size = sizeof(short) * 2;
	}
};

NetworkedGame::NetworkedGame()	{
	thisServer = nullptr;
	thisClient = nullptr;

	NetworkBase::Initialise();
	timeToNextPacket  = 0.0f;
	packetsToSnapshot = 0;
	playerStates = std::vector<int>();
}

NetworkedGame::~NetworkedGame()	{
	delete thisServer;
	delete thisClient;
}

void NetworkedGame::StartAsServer() 
{
	thisServer = new GameServer(NetworkBase::GetDefaultPort(), 4);
	thisServer->RegisterPacketHandler(Received_State, this);
	StartLevel();
}

void NetworkedGame::StartAsClient(char a, char b, char c, char d) 
{
	thisClient = new GameClient();
	thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort());

	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);

	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);

	StartLevel();
}

void NetworkedGame::UpdateGame(float dt) 
{
	timeToNextPacket -= dt;
	if (timeToNextPacket < 0) {
		if (thisServer) 
			UpdateAsServer(dt);
		else if (thisClient) 
			UpdateAsClient(dt);

		timeToNextPacket += 1.0f / 20.0f; //20hz server/client update
	}

	if (!thisServer && Window::GetKeyboard()->KeyPressed(KeyCodes::O)) 
		StartAsServer();
	if (!thisClient && Window::GetKeyboard()->KeyPressed(KeyCodes::P)) 
		StartAsClient(127,0,0,1);

	TutorialGame::UpdateGame(dt);
}

void NetworkedGame::UpdateAsServer(float dt)
{
	packetsToSnapshot--;
	if (packetsToSnapshot < 0) 
	{
		BroadcastSnapshot(false);
		packetsToSnapshot = 5;
	}
	else 
		BroadcastSnapshot(true);

	thisServer->UpdateServer();
}

void NetworkedGame::UpdateAsClient(float dt) 
{

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {	
		ClientPacket newPacket;

		newPacket.buttonstates[0] = 1;	
		newPacket.lastID = 0; 	
		thisClient->SendPacket(newPacket);	
		std::cout << "Sending shot" << std::endl;
	}		
	thisClient->UpdateClient();
}



void NetworkedGame::BroadcastSnapshot(bool deltaFrame) 
{
	for (const auto& player : thisServer->playerPeers)
	{	
		int playerID = player.first;
		//int lastAcknowledgedState = playerStates[playerID];

		std::vector<GameObject*>::const_iterator first, last;
		world->GetObjectIterators(first, last);

		for (auto i = first; i != last; ++i) 
		{
			NetworkObject* o = (*i)->GetNetworkObject();

			if (!o) 
				continue;

			GamePacket* newPacket = new GamePacket();
			newPacket->type = Full_State;

			if (o->WritePacket(&newPacket, deltaFrame, 0)) //lastAcknowledgedState
			{
				thisServer->SendPacketToPeer(newPacket, playerID);
				std::cout << "sending packet to peer" << std::endl;
				delete newPacket;
			}
		}
	}
}

void NetworkedGame::UpdateMinimumState() 
{
	//Periodically remove old data from the server
	int minID = INT_MAX;
	int maxID = 0; //we could use this to see if a player is lagging behind?

	for (auto i : stateIDs) {
		minID = std::min(minID, i.second);
		maxID = std::max(maxID, i.second);
	}
	//every client has acknowledged reaching at least state minID
	//so we can get rid of any old states!
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) 
	{
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) 
			continue;

		o->UpdateStateHistory(minID); //clear out old states so they arent taking up memory...
	}
}

void NetworkedGame::SpawnPlayer() 
{
	GameObject* play = new GameObject();	
	play->SetRenderObject(new RenderObject(&play->GetTransform(), cubeMesh, basicTex, basicShader));
	NetworkObject* player = new NetworkObject(*play, 0); // Network id!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	world->AddGameObject(static_cast<NetworkObject*>(player));
}

void NetworkedGame::StartLevel() 
{
	SpawnPlayer();
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source)
{
	std::cout << "recieving packages" << std::endl;

	if (thisClient) 
		thisClient->ReceivePacket(type, payload, source);
	else if (thisServer) 
		thisServer->ReceivePacket(type, payload, source);
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
	if (thisServer) 
	{ //detected a collision between players!
		MessagePacket newPacket;
		newPacket.messageID = COLLISION_MSG;
		newPacket.playerID  = a->GetPlayerNum();

		thisClient->SendPacket(newPacket);

		newPacket.playerID = b->GetPlayerNum();
		thisClient->SendPacket(newPacket);
	}
}