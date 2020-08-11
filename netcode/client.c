/* NetData - Stored */
struct Prediction
{
	f32 dt;
	u32 prediction_id;
	struct PlayerInput input;
};

struct PredictionResult
{
	u32 prediction_id;
	struct PlayerState state;
};

#define NETCODE_PREDICTION_MAX 256

struct NetData_Client
{
	bool connected;

	/* Temporary */
	bool success;
	u32 server_prediction_id;

	/* Connection */
	struct Net_Connection connection;

	/* Client */
	u32 client_id;
	struct Player *player;

	struct PlayerState player_states[NETCODE_CLIENT_MAX];
	mat4 player_matrices[NETCODE_CLIENT_MAX];
	bool players_active[NETCODE_CLIENT_MAX];
	v4 player_colors[NETCODE_CLIENT_MAX];

	u32 client_ticks_ahead;
	u32 client_prediction_id;
	struct Prediction predictions[NETCODE_PREDICTION_MAX];
	struct PredictionResult prediction_results[NETCODE_PREDICTION_MAX];
};

/* PACKETS */
/* Write */
void
netcode_packet_client_join_write(struct NetData_Client *data)
{
	u8 *at = data->connection.buffer_send;
	netcode_serialize_u8(&at, (u8)packet_client_join);

	data->connection.buffer_send_size = at - data->connection.buffer_send;
}

void
netcode_packet_client_leave_write(struct NetData_Client *data)
{
	u8 *at = data->connection.buffer_send;

	netcode_serialize_u8(&at, (u8)packet_client_leave);
	netcode_serialize_u32(&at, data->client_id);

	data->connection.buffer_send_size = at - data->connection.buffer_send;
}

u32
netcode_packet_client_player_input_write(struct NetData_Client *data)
{
	u8 *at = data->connection.buffer_send;

	netcode_serialize_u8(&at, (u8)packet_client_player_input);

	netcode_serialize_u32(&at, data->client_id);
	netcode_serialize_u32(&at, data->client_prediction_id);
	netcode_serialize_f32(&at, os_state.dt);
	netcode_serialize_player_input(&at, &data->player->input);

	data->connection.buffer_send_size = at - data->connection.buffer_send;
}

/* Read */
void
netcode_packet_server_join_result_read(struct NetData_Client *data)
{
	u8 *at = data->connection.buffer_receive;

	u8 packet_type;
	netcode_deserialize_u8(&at, &packet_type);
	ASSERT(packet_type == (u8)packet_server_join_result);

	netcode_deserialize_u8(&at, &data->success);
	if(data->success) netcode_deserialize_u32(&at, &data->client_id);
	//data->players_active[data->client_id] = true;
}

void
netcode_packet_server_players_state_read(struct NetData_Client *data)
{
	u8 *at = data->connection.buffer_receive;

	u8 packet_type;
	netcode_deserialize_u8(&at, &packet_type);
	ASSERT(packet_type == (u8)packet_server_players_state);

	netcode_deserialize_u32(&at, &data->server_prediction_id);

	/* SPEED TODO: We clear players_active each packet */
	for(i32 i = 0; i < NETCODE_CLIENT_MAX; ++i) data->players_active[i] = false;

	u8 player_count = 0;
	netcode_deserialize_u8(&at, &player_count);
	for(u8 i = 0; i < player_count; ++i)
	{
		u8 player_id;
		netcode_deserialize_u8(&at, &player_id);
		ASSERT(player_id < NETCODE_CLIENT_MAX);

		netcode_deserialize_player_state(&at, &data->player_states[player_id]);
		data->players_active[player_id] = true;
	}
}

/* Receive */
void
netcode_packet_server_join_result_receive(struct NetData_Client *data)
{
	netcode_packet_server_join_result_read(data);
	if(!data->success)
	{
		ERROR("Client: Server didn't let us in.");
		return;
	}
	data->connected = true;
}

void
netcode_packet_server_players_state_receive(struct NetData_Client *data)
{
	netcode_packet_server_players_state_read(data);

	data->client_ticks_ahead = data->client_prediction_id - data->server_prediction_id;
	ASSERT(data->client_ticks_ahead >= 0);
	ASSERT(data->client_ticks_ahead <= NETCODE_PREDICTION_MAX);

	struct PlayerState *server_player_state = &data->player_states[data->client_id];

	/* INDEX?? */
	u32 index = data->server_prediction_id;
	v3 dt_pos = v3_s(server_player_state->pos, data->prediction_results[index].state.pos);

	/* TODO: Can we completly get rid of these? */
	f32 dt_max_error = 0.001f; /* 1cm */
	f32 dt_max_error_sq = dt_max_error * dt_max_error; /* 1cm */

	if(v3_len(dt_pos) > dt_max_error_sq)
	{
		/* TODO: Correct preditcion */
		//DEBUG("Client: Correcting a prediction, id %d\n", data->server_prediction_id);

		data->player->state = *server_player_state;
		for(u32 i = data->server_prediction_id + 1; i < data->client_prediction_id; ++i)
		{
			/* TODO: Mask */
			u32 replay_index = i;

			struct Prediction *prediction = &data->predictions[replay_index];
			struct PredictionResult *prediction_result = &data->prediction_results[replay_index];

			player_control(&data->player->input, &data->player->state, prediction->dt);
			prediction_result->state = data->player->state;
		}
	}
}

void
netcode_client_packets_receive(struct NetData_Client *data)
{
	struct Net_Node server;
	loop
	{
		net_receive_out(&data->connection, &server);
		if(net_fail(&data->connection)) break;

		switch((enum Netcode_PacketServer)data->connection.buffer_receive[0])
		{
		case packet_server_join_result: netcode_packet_server_join_result_receive(data); break;
		case packet_server_players_state: netcode_packet_server_players_state_receive(data); break;
		}
	}
}

/* Send */
void
netcode_packet_client_join_send(struct NetData_Client *data)
{
	netcode_packet_client_join_write(data);
	net_send(&data->connection);
}

void
netcode_packet_client_player_input_send(struct NetData_Client *data)
{
	netcode_packet_client_player_input_write(data);
	net_send(&data->connection);
}

void
netcode_client_packets_send(struct NetData_Client *data)
{
	if(data->connected == false) return;

	netcode_packet_client_player_input_send(data);
}

/* Main */
i32
netcode_client_connect(struct NetData_Client *data)
{
	netcode_packet_client_join_send(data);
	if(net_fail(&data->connection)) ERROR("Client: Failed to connect to server.");
}

i32
netcode_client_disconnect(struct NetData_Client *data)
{
	netcode_packet_client_leave_write(data);
	net_send(&data->connection);
	data->connected = false;
}
