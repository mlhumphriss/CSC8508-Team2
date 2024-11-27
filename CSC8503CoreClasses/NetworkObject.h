#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"

namespace NCL::CSC8503 {
	class GameObject;

	struct FullPacket : public GamePacket {
		int		objectID = -1;
		NetworkState fullState;

		FullPacket() {
			type = Full_State;
			size = sizeof(FullPacket) - sizeof(GamePacket);
		}
	};

	struct DeltaPacket : public GamePacket {
		int		fullID		= -1;
		int		objectID	= -1;
		char	pos[3];
		char	orientation[4];

		DeltaPacket() {
			type = Delta_State;
			size = sizeof(DeltaPacket) - sizeof(GamePacket);
		}
	};

	struct ClientPacket : public GamePacket {
		int		lastID;
		char	buttonstates[8];

		ClientPacket() {
			size = sizeof(ClientPacket);
		}
	};

	class FullStateReceiver : public PacketReceiver {
	public:
		FullStateReceiver() : lastAcknowledgedStateID(0) {}

		void ReceivePacket(int type, GamePacket* payload, int source) override 
		{
			if (type == Full_State) {
				FullPacket* packet = (FullPacket*)payload;
				lastAcknowledgedStateID = packet->fullState.stateID;
			}
		}

		int GetLastAcknowledgedStateID() const {
			return lastAcknowledgedStateID;
		}

	private:
		int lastAcknowledgedStateID;
	};


	class NetworkObject		
	{
	public:
		NetworkObject(GameObject& o, int id);
		virtual ~NetworkObject();
		virtual bool ReadPacket(GamePacket& p);
		virtual bool WritePacket(GamePacket** p, bool deltaFrame, int stateID);

		void UpdateStateHistory(int minID);

	protected:

		NetworkState& GetLatestNetworkState();

		bool GetNetworkState(int frameID, NetworkState& state);

		virtual bool ReadDeltaPacket(DeltaPacket &p);
		virtual bool ReadFullPacket(FullPacket &p);

		virtual bool WriteDeltaPacket(GamePacket**p, int stateID);
		virtual bool WriteFullPacket(GamePacket**p);

		GameObject& object;
		NetworkState lastFullState;
		std::vector<NetworkState> stateHistory;

		int deltaErrors;
		int fullErrors;
		int networkID;
	};
}