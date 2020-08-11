#define NETCODE_MTU KB(1)
#define NETCODE_CLIENT_MAX 4

#define NETCODE_PORT 9003
//#define NETCODE_ADDRESS net_addr_from_cstr("3.15.45.225")
#define NETCODE_ADDRESS NET_ADDRESS_LOCAL

/* NetCode - Transactional */
enum Netcode_PacketClient
{
	packet_client_join,
	packet_client_leave,
	packet_client_player_input,
	packet_client_max,
};

enum Netcode_PacketServer
{
	packet_server_join_result,
	packet_server_players_state,
	packet_server_max,
};

enum NetCode_PlayerInput
{
	player_input_forward = 0x01,
	player_input_back = 0x02,
	player_input_left = 0x04,
	player_input_right = 0x08,

	player_input_jump = 0x10,
	player_input_shoot = 0x28,
};

void
netcode_serialize_u8(u8 **buffer, u8 value)
{
	u8 *at = (u8 *)*buffer;
	*at = value;
	++at;
	*buffer = at;
}

void
netcode_serialize_u32(u8 **buffer, u32 value)
{
	u32 *at = (u32 *)*buffer;
	*at = value;
	++at;
	*buffer = (u8 *)at;
}

void
netcode_serialize_f32(u8 **buffer, f32 value)
{
	f32 *at = (f32 *)*buffer;
	*at = value;
	++at;
	*buffer = (u8 *)at;
}

void
netcode_serialize_v3(u8 **buffer, v3 value)
{
	netcode_serialize_f32(buffer, value.x);
	netcode_serialize_f32(buffer, value.y);
	netcode_serialize_f32(buffer, value.z);
}

void
netcode_serialize_player_input(u8 **buffer, struct PlayerInput *player_input)
{
	u8 packed_player_input = 0;
	if(player_input->forward)  packed_player_input |= (1 << 0);
	if(player_input->backward) packed_player_input |= (1 << 1);
	if(player_input->left)     packed_player_input |= (1 << 2);
	if(player_input->right)    packed_player_input |= (1 << 3);
	if(player_input->jump)     packed_player_input |= (1 << 4);

	netcode_serialize_u8(buffer, packed_player_input);
	netcode_serialize_v3(buffer, player_input->rot_look);
}

void
netcode_serialize_player_state(u8 **buffer, struct PlayerState *player_state)
{
	netcode_serialize_v3(buffer, player_state->pos);
	netcode_serialize_v3(buffer, player_state->rot);
}

void
netcode_deserialize_u8(u8 **buffer, u8 *value)
{
	u8 *at = (u8 *)*buffer;
	*value = *at;
	++at;
	*buffer = at;
}

void
netcode_deserialize_u32(u8 **buffer, u32 *value)
{
	u32 *at = (u32 *)*buffer;
	*value = *at;
	++at;
	*buffer = (u8 *)at;
}

void
netcode_deserialize_f32(u8 **buffer, f32 *value)
{
	f32 *at = (f32 *)*buffer;
	*value = *at;
	++at;
	*buffer = (u8 *)at;
}

void
netcode_deserialize_v3(u8 **buffer, v3 *value)
{
	v3 *at = (v3 *)*buffer;
	*value = *at;
	++at;
	*buffer = (u8 *)at;
}

void
netcode_deserialize_player_input(u8 **buffer, struct PlayerInput *player_input)
{
	u8 packed_player_input;
	netcode_deserialize_u8(buffer, &packed_player_input);
	netcode_deserialize_v3(buffer, &player_input->rot_look);

	player_input->forward  = (packed_player_input & (1 << 0)) != 0;
	player_input->backward = (packed_player_input & (1 << 1)) != 0;
	player_input->left     = (packed_player_input & (1 << 2)) != 0;
	player_input->right    = (packed_player_input & (1 << 3)) != 0;
	player_input->jump     = (packed_player_input & (1 << 4)) != 0;

	if(packed_player_input != 0)
	{
		u32 breakpoint = 0;
	}
}

void
netcode_deserialize_player_state(u8 **buffer, struct PlayerState *player_state)
{
	netcode_deserialize_v3(buffer, &player_state->pos);
	netcode_deserialize_v3(buffer, &player_state->rot);
}
