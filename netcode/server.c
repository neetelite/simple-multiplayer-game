#include "../../../library/standard.h"
#include "../../../library/base.h"

#include "../player.c"
#include "shared.c"

#define NETCODE_SERVER_TICK_RATE 60

struct NetData_Server
{
	/* Connection */
	struct Net_Connection connection;

	/* Temporary/Transitional */
	u32 client_id;
	bool success;
	u32 prediction_id;

	/* Client */
	f32 client_dt;
	u32 client_count;
	struct Net_Node client_nodes[NETCODE_CLIENT_MAX];

	/* TODO: Save data with player state and input  */
	struct PlayerState client_states[NETCODE_CLIENT_MAX];
	struct PlayerInput client_inputs[NETCODE_CLIENT_MAX];
	u32 client_prediction_ids[NETCODE_CLIENT_MAX];

	f32 client_heartbeats[NETCODE_CLIENT_MAX];
	f32 client_timeout;

	/* Server */
	u32 server_tick;
	u32 server_tick_rate;
	f32 server_seconds_per_tick;
	f32 server_dt;
};

#include "server_engine.c"

/* PACKETS - read client, write server */
/* Read */
void
netcode_packet_client_leave_read(struct NetData_Server *data)
{
	u8 *at = data->connection.buffer_receive;

	u8 message_type;
	netcode_deserialize_u8(&at, &message_type);
	ASSERT(message_type == (u8)packet_client_leave);

	netcode_deserialize_u32(&at, &data->client_id);
}

void
netcode_packet_client_player_input_read(struct NetData_Server *data)
{
	u8 *at = data->connection.buffer_receive;

	u8 message_type;
	netcode_deserialize_u8(&at, &message_type);
	ASSERT(message_type == (u8)packet_client_player_input)

	netcode_deserialize_u32(&at, &data->client_id);
	netcode_deserialize_u32(&at, &data->prediction_id);
	netcode_deserialize_f32(&at, &data->client_dt);
	netcode_deserialize_player_input(&at, &data->client_inputs[data->client_id]);
}

/* Write */
u32
netcode_packet_server_join_result_write(struct NetData_Server *data)
{
	u8 *at = data->connection.buffer_send;
	netcode_serialize_u8(&at, (u8)packet_server_join_result);
	netcode_serialize_u8(&at, data->success);

	if(data->success) netcode_serialize_u32(&at, data->client_id);

	data->connection.buffer_send_size = at - data->connection.buffer_send;
}

u32
netcode_packet_server_players_state_write(struct NetData_Server *data)
{
	u8 *at = data->connection.buffer_send;
	netcode_serialize_u8(&at, (u8)packet_server_players_state);

	netcode_serialize_u32(&at, data->client_prediction_ids[data->client_id]);
	/* TODO: State extra */

	u8 *at_player_count = at;
	++at;

	u8 player_count = 0;
	for(u8 client_id = 0; client_id < NETCODE_CLIENT_MAX; ++client_id)
	{
		struct Net_Node *client_node = data->client_nodes + client_id;
		if(!client_node->address) continue;

		++player_count;
		netcode_serialize_u8(&at, client_id);
		netcode_serialize_player_state(&at, &data->client_states[client_id]);
	}
	netcode_serialize_u8(&at_player_count, player_count);

	data->connection.buffer_send_size = at - data->connection.buffer_send;
}

/* Receive */
void
netcode_packet_client_join_receive(struct NetData_Server *data, struct Net_Node *source_node)
{
	char source_addr[22];
	//cstr_from_addr(source_addr, source_node);

	/* Generate new id */
	data->client_id = (u32)-1;
	for(i32 i = 0; i < NETCODE_CLIENT_MAX; ++i)
	{
		struct Net_Node *client_node = data->client_nodes + i;
		if(client_node->address == 0)
		{
			data->client_id = i;
			break;
		}
	}

	/* If couldn't generate a new id */
	if(data->client_id == (u32)-1)
	{
		ERROR("Server: Could not find a slot for player\n");

		data->success = false;
		netcode_packet_server_join_result_write(data);
		net_send_to(&data->connection, source_node);
		return;
	}

	/* Reserve the slot */
	DEBUG("Server: Client [%hu] connected.\n", data->client_id);

	data->success = true;
	netcode_packet_server_join_result_write(data);
	net_send_to(&data->connection, source_node);
	if(net_fail(&data->connection))
	{
		ERROR("Server: Couldn't set packet server_join_result!");
		return;
	}

	data->client_nodes[data->client_id] = *source_node;
	data->client_inputs[data->client_id] = (struct PlayerInput){};
	data->client_states[data->client_id] = (struct PlayerState){};
	data->client_prediction_ids[data->client_id] = 0;
	data->client_heartbeats[data->client_id] = 0;
}

void
netcode_packet_client_leave_receive(struct NetData_Server *data, struct Net_Node *source_node)
{
	netcode_packet_client_leave_read(data);

	struct Net_Node *client_node = &data->client_nodes[data->client_id];

	char cstr_source_addr[22];
	net_cstr_addr_from_node(cstr_source_addr, source_node);

	/* ID doesn't match source address */
	if(!net_node_eql(source_node, client_node))
	{
		char cstr_client_addr[22];
		net_cstr_addr_from_node(cstr_client_addr, client_node);

		DEBUG("Server: packet_client_leave from %hu(%s), expected (%s)",
		      data->client_id, cstr_source_addr, cstr_client_addr);
		return;
	}

	/* Successfull leave */
	*client_node = (struct Net_Node){};
	DEBUG("Server: Client [%hu] disconnected.\n", data->client_id);
}

void
netcode_packet_client_player_input_receive(struct NetData_Server *data, struct Net_Node *source_node)
{
	netcode_packet_client_player_input_read(data);

	struct Net_Node *client_node = &data->client_nodes[data->client_id];
	if(!net_node_eql(source_node, client_node))
	{
		DEBUG("Server: packet_client_player_input discarded. From %u:%hu, Expected %u:%hu\n",
		      source_node->address, source_node->port, client_node->address, client_node->port);
		return;
	}

	/* TODO: */
	u32 client_id = data->client_id;
	player_control(&data->client_inputs[client_id], &data->client_states[client_id], data->client_dt);
	data->client_prediction_ids[client_id] = data->prediction_id;
	data->client_heartbeats[data->client_id] = 0.0f;
}

void
netcode_server_packets_receive(struct NetData_Server *data)
{
	/* Reads the received packets, and acts depending on type */

	struct Net_Node source_node;
	while(true)
	{
		net_receive_out(&data->connection, &source_node);
		if(net_fail(&data->connection)) break;

		enum Netcode_PacketClient packet_type = data->connection.buffer_receive[0];
		//printf("Packet Type: %d\n", packet_type);
		switch(packet_type)
		{
		case packet_client_join:
		{
			netcode_packet_client_join_receive(data, &source_node);
		} break;
		case packet_client_leave:
		{
			netcode_packet_client_leave_receive(data, &source_node);
		} break;
		case packet_client_player_input:
		{
			netcode_packet_client_player_input_receive(data, &source_node);
		} break;
		}
	}
}

void
netcode_server_timeout_check(struct NetData_Server *data)
{
	/* Check if any client has timed out */
	for(u32 i = 0; i < NETCODE_CLIENT_MAX; ++i)
	{
		struct Net_Node *client_node = &data->client_nodes[i];
		if(!client_node->address) continue;

		data->client_heartbeats[i] += data->server_seconds_per_tick;
		if(data->client_heartbeats[i] > data->client_timeout)
		{
			/* TODO */
			DEBUG("Server: Client [%d] timed out.\n", i);
			data->client_nodes[i] = (struct Net_Node){0};
		}
	}
}

void
netcode_server_packets_send(struct NetData_Server *data)
{
	/* Sends the players state to all the connected clients */

	for(u32 i = 0; i < NETCODE_CLIENT_MAX; ++i)
	{
		struct Net_Node *client_node = &data->client_nodes[i];
		if(!client_node->address) continue;

		data->client_id = i;
		netcode_packet_server_players_state_write(data);
		net_send_to(&data->connection, client_node);
		if(net_fail(&data->connection)) ERROR("NetCode: send_to failed\n");
	}
}


void
netcode_server_init(struct NetData_Server *data)
{
	net_init(&data->connection, net_connection_type_host, NET_UDP, NET_ADDRESS_ANY, NETCODE_PORT);
	net_bind(&data->connection);

	data->server_tick = 0;
	data->server_tick_rate = NETCODE_SERVER_TICK_RATE;
	data->server_seconds_per_tick = 1.0f / data->server_tick_rate;
	data->client_timeout = 2.0f;

	net_set_non_blocking(&data->connection);
}

void
netcode_server_term(struct NetData_Server *data)
{

}

i32
main(void)
{
	printf("Server: Running.\n");

	struct NetData_Server data = {0};
	netcode_server_init(&data);

	struct TimeLimiter limiter = {0};
	time_limiter_init(&limiter, TIME_NS_FROM_S(1) / data.server_tick_rate);

	bool running = true;
	while(running)
	{
		netcode_server_packets_receive(&data);
		//netcode_server_engine_players_update(&data);

		netcode_server_timeout_check(&data);
		netcode_server_packets_send(&data);


		time_limiter_tick(&limiter);
		data.server_dt = limiter.dt_s;
		++data.server_tick;
	}

	netcode_server_term(&data);
}
