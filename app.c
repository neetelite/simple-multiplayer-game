#include "mesh.h"

#define X 0.7
#define Y 0.7
#define Z 2

#define FX 10
#define FY 10

#define cuboid_position_count 8
static v3 cuboid_positions[cuboid_position_count] =
	{
		-X, -Y,  Z,
		 X, -Y,  Z,
		 X,  Y,  Z,
		-X,  Y,  Z,
		-X, -Y, -Z,
		 X, -Y, -Z,
		 X,  Y, -Z,
		-X,  Y, -Z
	};

static u32 cuboid_indices[12 * 3] =
	{
		0, 1, 2,
		0, 2, 3,
		4, 5, 1,
		4, 1, 0,
		5, 6, 2,
		5, 2, 1,
		6, 7, 3,
		6, 3, 2,
		7, 4, 0,
		7, 0, 3,
		7, 6, 5,
		7, 5, 4
	};

#define floor_position_count 4
static v3 floor_positions[floor_position_count] =
	{
		-FX, -FY,  0,
		 FX, -FY,  0,
		 FX,  FY,  0,
		-FX,  FY,  0,
	};

static u32 floor_indices[6] =
	{
		0, 1, 2,
		0, 2, 3,
	};

static struct Mesh cuboid;
static struct Mesh floor_plane;
static struct Camera camera;


void
cam_dir_from_rot(struct Camera *cam)
{
	/* TODO: Genralize this function */

	f32 heading = f32_rad_from_deg(cam->rot.z); //heading
	f32 elevation = f32_rad_from_deg(cam->rot.x); //elevation

	f32 sin_ele = f32_sin(elevation);
	f32 cos_ele = f32_cos(elevation);

	f32 sin_hea = f32_sin(heading);
	f32 cos_hea = f32_cos(heading);

	cam->dir.x = cos_ele * sin_hea;
	cam->dir.y = cos_ele * cos_hea;
	cam->dir.z = sin_ele;

	cam->n = v3_inv(cam->dir);
}

void
cam_control(struct Camera *cam)
{
	//struct InputKeyboard *keyboard = &input.keyboards[0];
	struct Input_Mouse *mouse = &input.mice[0];
	f32 dt = os_state.dt;

	f32 sensitivity = 4;
	//f32 mov_speed = 1.4 * dt; /* Average Walking Speed - 1.4 meters per second */
	f32 mov_speed = 5 * dt; /* Average Walking Speed - 1.4 meters per second */
	f32 rot_speed = 90 * dt; /* 90 degress per second */
	rot_speed = f32_rad_from_deg(rot_speed);

	/* Rotation */
	v3 new_rot = cam->rot;
	new_rot.x += (mouse->d.y * 0.01) * sensitivity;
	new_rot.z += (mouse->d.x * 0.01) * sensitivity;
	new_rot = v3_angle_norm(new_rot);

	f32 cap_up = -89;
	f32 cap_down = 89;
	if(new_rot.x < cap_up) new_rot.x = cap_up;
	if(new_rot.x > cap_down) new_rot.x = cap_down;

	cam->rot = new_rot;
	cam_dir_from_rot(cam);

	/* Translation */
	if(key_down(KEY_SPACE))
	{
		cam->pos = v3_a(cam->pos, v3_mf(cam->v, mov_speed));
	}

	if(key_down(KEY_ESCAPE))
	{
		cam->pos = v3_s(cam->pos, v3_mf(cam->v, mov_speed));
	}
	if(key_down('w'))
	{
		//cam->pos = v3_a(cam->pos, v3_mf(cam->n, mov_speed));
		cam->pos = v3_a(cam->pos, v3_mf(cam->dir, mov_speed));
	}
	if(key_down('a'))
	{
		cam->pos = v3_s(cam->pos, v3_mf(cam->u, mov_speed));
	}
	if(key_down('s'))
	{
		//cam->pos = v3_s(cam->pos, v3_mf(cam->n, mov_speed));
		cam->pos = v3_s(cam->pos, v3_mf(cam->dir, mov_speed));
	}
	if(key_down('d'))
	{
		cam->pos = v3_a(cam->pos, v3_mf(cam->u, mov_speed));
	}
}

void gl_program_create_pass(GLuint *program)
{
	char path[PATH_MAX];

	cstr_cat(path, os_state.path_src, "shader/simple.vs");
	u32 vertex_shader = gl_shader_load(path, GL_VERTEX_SHADER);

	cstr_cat(path, os_state.path_src, "shader/simple.fs");
	u32 fragment_shader = gl_shader_load(path, GL_FRAGMENT_SHADER);

	*program = glCreateProgram();

	glAttachShader(*program, vertex_shader);
	glAttachShader(*program, fragment_shader);
	glLinkProgram(*program);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
}

void
gl_init(void)
{
	/* TODO(lungu): Depth test doesn't work sometimes, it has to do with the graphics card it's using */
	gl_enable(GL_TEXTURE_2D);
	gl_enable(GL_DEPTH_TEST);

	gl_enable(GL_BLEND);
	gl_viewport_set_dimension(V4I(0, 0, os_context.width, os_context.height));
	//gl_viewport_set_color_u32(0x0b0b0bff);
	gl_viewport_set_color_u32(0x000000ff);
	gl_program_create_pass(&game->program_pass);
}

void
player_input(struct Player *player)
{
	struct Input_Mouse *mouse = &input.mice[0];
	struct PlayerInput *input = &player->input;
	struct Camera *cam = &camera;

	f32 sensitivity = 4;

	/* Keyboard */
	input->forward = key_down('w');
	input->backward = key_down('s');
	input->left = key_down('a');
	input->right = key_down('d');
	input->jump = key_press(' ');

	/* Mouse */
	if(!mouse->locked) return;

	v3 new_rot = cam->rot;
	new_rot.x += (mouse->d.y * 0.01) * sensitivity;
	new_rot.z += (mouse->d.x * 0.01) * sensitivity;

	/* TODO: Pull the camera close are you reach these bounds instead of caping */
	f32 cap_up = -85; // -70
	f32 cap_down = 35;
	if(new_rot.x < cap_up) new_rot.x = cap_up;
	if(new_rot.x > cap_down) new_rot.x = cap_down;

	input->rot_look = new_rot;
}

void
player_client_extra(struct Player *player)
{
	struct Input_Mouse *mouse = &input.mice[0];
	struct PlayerInput *input = &player->input;
	struct PlayerState *state = &player->state;
	struct Camera *cam = &camera;
	f32 dt = os_state.dt;

	cam->rot = input->rot_look;
	cam_dir_from_rot(cam);

	if(player->cam_fps)
	{
		cam->pos = v3_a(state->pos, player->eyes_pos);
	}
	else
	{
		f32 behind = 4*Z;
		f32 above = Z;
		cam->pos = v3_a(state->pos, v3_a(v3_mf(cam->dir, -behind), V3(0, 0, above)));
	}
}

void
player_update(struct Player *player)
{
	player_input(player);
	player_control(&player->input, &player->state, os_state.dt);

	if(game->networked) netcode_client_packets_send(&game->net_data);
	if(game->networked) netcode_client_packets_receive(&game->net_data);

	u32 index = game->net_data.client_prediction_id;
	struct Prediction *prediction = &game->net_data.predictions[index];
	struct PredictionResult *result = &game->net_data.prediction_results[index];

	prediction->dt = os_state.dt;
	prediction->input = player->input;
	result->state = player->state;

	game->net_data.player_states[game->net_data.client_id] = player->state;
	++game->net_data.client_prediction_id;

	player_client_extra(player);

}

void
scene_update(void)
{
	struct Player *player = &game->player;
	player_update(player);

	cam_matrix(&camera);
}

mat4
player_transform(struct PlayerState *state)
{
	m4x4 result = MAT4_ID;

	m4x4 matrix_scale = m4x4_scale(1, 1, 1);
	/* TODO: We have the direction not the rotation */
	m4x4 matrix_rotation = m4x4_euler_rot(state->rot.x, state->rot.y, state->rot.z);
	m4x4 matrix_translation = m4x4_trans(state->pos.x, state->pos.y, state->pos.z);

	/* FIXME(lungu): Left to right, instead of right to left multiplication, use the new mat4 functions */
	result = mat4_m(matrix_translation, mat4_m(matrix_scale, matrix_rotation));
	return(result);
}

void
scene_draw(void)
{
	/* Constants */
	u32 program = game->program_pass;
	v4 color_floor_border = V4_COLOR_WHITE;
	v4 color_cuboid_border = V4_COLOR_BLACK;
	mat4 mvp = MAT4_ID;

	gl_program_bind(program);

	/* Locatin */
	GLuint mvp_location = glGetUniformLocation(program, "mvp");
	GLuint color_location = glGetUniformLocation(program, "color");

	/* Floor Matrix */
	mvp = mat4_m(camera.transform, floor_plane.transform);
	glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (f32 *)&mvp);

	/* Floor */
	gl_vao_bind(floor_plane.VAO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUniform4fv(color_location, 1, (f32 *)&floor_plane.color);
	gl_elements_draw(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, (void *)floor_plane.indices);

	#if 0
	/* Floor Wireframe */
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUniform4fv(color_location, 1, (f32 *)&color_floor_border);
	gl_elements_draw(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, (void *)floor_plane.indices);
	#endif

	/* Matrix */
	//struct PlayerState *player_state = NULL;
	//if(game->networked) player_state = &game->net_data.player_states[game->net_data.client_id];
	//else player_state = &game->player.state;

	/* Cuboid */
	gl_vao_bind(cuboid.VAO);
	/* TODO: Activate player if networking is disabled */
	if(game->networked)
	{
		for(i32 i = 0; i < NETCODE_CLIENT_MAX; ++i)
		{
			if(game->net_data.players_active[i] == false) continue;

			if(game->net_data.client_id == i)
			{
				mvp = mat4_m(camera.transform, player_transform(&game->player.state));
			}
			else
			{
				mvp = mat4_m(camera.transform, player_transform(&game->net_data.player_states[i]));
			}
			glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (f32 *)&mvp);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glUniform4fv(color_location, 1, (f32 *)&game->net_data.player_colors[i]);
			gl_elements_draw(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, (void *)cuboid.indices);

			#if 0
			/* Cuboid Wireframe */
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniform4fv(color_location, 1, (f32 *)&color_cuboid_border);
			gl_elements_draw(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, (void *)cuboid.indices);
			#endif
		}
	}
	else
	{
		mvp = mat4_m(camera.transform, player_transform(&game->player.state));
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (f32 *)&mvp);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glUniform4fv(color_location, 1, (f32 *)&cuboid.color);
		gl_elements_draw(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, (void *)cuboid.indices);

#if 0
		/* Cuboid Wireframe */
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4fv(color_location, 1, (f32 *)&color_cuboid_border);
		gl_elements_draw(GL_TRIANGLES, 12 * 3, GL_UNSIGNED_INT, (void *)cuboid.indices);
#endif
	}

	gl_vao_unbind();
	gl_program_unbind();
}

void
netcode_client_init(struct Game *game, struct NetData_Client *data)
{
	if(game->networked == false) return;
	net_init(&data->connection, net_connection_type_client, NET_UDP, NETCODE_ADDRESS, NETCODE_PORT);
	if(net_fail(&data->connection)) ERROR("Client: Failed to initialize socket! %s\n", strerror(errno));

	netcode_client_connect(data);
	data->player = &game->player;

	data->player_colors[0] = V4_COLOR_RED;
	data->player_colors[1] = V4_COLOR_GREEN;
	data->player_colors[2] = V4_COLOR_BLUE;

	net_set_non_blocking(&data->connection);
}

void
netcode_client_term(struct Game *game, struct NetData_Client *data)
{
	if(game->networked == false) return;
	netcode_client_disconnect(data);
}

void
mesh_init(struct Mesh *mesh)
{
	/* Generate */
	gl_vao_gen(1, &mesh->VAO);
	gl_vbo_gen(1, &mesh->VBO);

	/* Bind */
	gl_vao_bind(mesh->VAO);
	gl_vbo_bind(mesh->VBO, GL_ARRAY_BUFFER);

	/* Push data */
	gl_vbo_data(mesh->positions, mesh->position_count * sizeof(v3), GL_ARRAY_BUFFER, GL_STATIC_DRAW);

	/* Postion Coordiantes */
	gl_vbo_attrib_set(0, 3, GL_FLOAT, sizeof(v3), GL_BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	/* Unbind VBO and VAO */
	gl_vao_unbind();
	gl_vbo_unbind(GL_ARRAY_BUFFER);

}

void
app_main(void)
{
	if(app_memory.init == false)
	{
		/* Game */
		game = os_state.app_memory;

		/* OpenGL */
		gl_init();

		/* Meshes */
		cuboid.position_count = cuboid_position_count;
		cuboid.positions = cuboid_positions;
		cuboid.indices = cuboid_indices;
		cuboid.color = V4_COLOR_WHITE;
		mesh_init(&cuboid);

		floor_plane.position_count = floor_position_count;
		floor_plane.positions = floor_positions;
		floor_plane.indices = floor_indices;
		floor_plane.color = v4_df(v4_from_u32(0x111111ff), 256);
		floor_plane.transform = MAT4_ID;
		mesh_init(&floor_plane);

		/* Camera */
		cam_load_default(&camera, 0, "Free Camera");
		camera.pos.y -= 10;

		/* Input */
		input.mice[0].locked = false;

		/* Network */
		game->networked = true;
		netcode_client_init(game, &game->net_data);

		app_memory.init = true;
	}

	if(game->net_data.client_id != 0) input.mice[0].locked = true;

	/* Clear */
	gl_viewport_clear();

	/* Update */
	scene_update();

	/* Draw */
	scene_draw();

	if(os_state.running == false)
	{
		netcode_client_term(game, &game->net_data);
	}
}
