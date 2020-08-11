struct PlayerInput
{
	bool forward;
	bool backward;
	bool left;
	bool right;
	bool jump;

	v3 rot_look;
};

struct PlayerState
{
	v3 pos;
	v3 rot;

	//u8 action;
};

struct Player
{
	struct PlayerInput input;
	struct PlayerState state;

	/* Other state */
	f32 rot_z_t;
	f32 rot_z_target;

	v3 dir_looking;
	v3 dir_walking;

	bool cam_fps;
	v3 eyes_pos;

	mat4 transform;
};

#define FLOOR_HEIGHT 2

void
player_control(struct PlayerInput *input, struct PlayerState *state, f32 dt)
{
	/* Constants */
	f32 speed_walk = 1.4 * dt;
	//f32 speed_run = 3.7 * dt;
	f32 speed_run = 10 * dt;
	f32 speed_jump = 1.4 * dt;
	f32 speed_gravity = 1.4 * dt;
	f32 sensitivity = 4;

	/* Variables */
	bool jump = false;
	v3 new_pos = V3_ZERO;
	v3 new_dir_walking = V3_ZERO;
	v3 new_dir_looking = V3_ZERO;
	v3 new_dir_input = V3_ZERO;
	f32 new_rot_forward = 0.0f;
	f32 new_rot_z = 0;
	bool free_look = false;
	bool move = true;

	new_rot_forward = input->rot_look.z;

	//if(player->rot.z != player->rot_z_target) player->rot_z_t += 0.1 * dt;
	//else player->rot_z_t = 0;

	/* Keyboard */
	if(input->jump) jump = true;
	if(input->forward) new_dir_input.y += 1;
	if(input->left) new_dir_input.x -= 1;
	if(input->backward) new_dir_input.y -= 1;
	if(input->right) new_dir_input.x += 1;

	/* TODO INCOMPLETE(lungu): Interpolate between rotations */
	/* NOTE(lungu): Change the rotation to quaternion */
	if(new_dir_input.x && !new_dir_input.y) new_rot_z = new_rot_forward + 90 * new_dir_input.x;
	else if(new_dir_input.x && new_dir_input.y == 1) new_rot_z = new_rot_forward + (45 * new_dir_input.x);
	else if(!new_dir_input.x && new_dir_input.y == 1) new_rot_z = new_rot_forward + 0;
	else if(new_dir_input.x && new_dir_input.y == -1) new_rot_z = new_rot_forward + (135 * new_dir_input.x);
	else if(!new_dir_input.x && new_dir_input.y == -1) new_rot_z = new_rot_forward + 180;
	else
	{
		new_rot_z = state->rot.z;
		move = false;
	}
	new_rot_z = f32_mod(new_rot_z, 360);

	if(move)
	{
		/* Update state */
		new_dir_walking = v3_dir_from_rot(V3(0, 0, new_rot_z));
		new_pos = v3_mf(new_dir_walking, speed_run);
	}
	state->rot.z = new_rot_z;

	/* Physics */
	new_pos = v3_a(state->pos, new_pos);
	new_pos.z -= speed_gravity;

	f32 floor_height = FLOOR_HEIGHT;
	if(new_pos.z < floor_height) new_pos.z = floor_height;

	/* NOTE(lungu): TERM */
	state->pos = new_pos;
}
