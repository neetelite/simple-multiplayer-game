/* This file is not used, it's purpose is to give a visual look of how the packets are structures */
struct NetCode_PacketClientJoin
{
	u8 packet_type;
};

struct NetCode_PacketClientLeave
{
	u8 packet_type;
	u32 client_id;
};

struct NetCode_PacketClientPlayerInput
{
	u8 packet_type;
	u32 client_id;
	u32 prediction_id;
	f32 dt;
	struct PlayerInput input;
};

struct NetCode_PacketServerJoinResult
{
	u8 packet_type;
	bool success;
	u32 client_id;
};

struct NetCode_PacketServerPlayerState
{
	u8 packet_type;
	u32 prediction_id;
	u32 player_count;

	struct
	{
		u32 client_id;
		u32 client_state;
	} states[player_count];
};
