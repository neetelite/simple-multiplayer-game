void
netcode_server_engine_players_update(struct NetData_Server *data)
{
	for(u32 i = 0; i < NETCODE_CLIENT_MAX; ++i)
	{
		struct Net_Node *client_node = &data->client_nodes[i];
		if(!client_node->address) continue;

		struct PlayerInput *input = &data->client_inputs[i];
		struct PlayerState *state = &data->client_states[i];

		/* NOTE: The client and the server are operating on different "dt" values */
		player_control(input, state, data->client_dt);
	}
}
