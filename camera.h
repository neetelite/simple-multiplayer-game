struct Camera
{
	u32 id;
	char *name;

	v3 pos;
	v3 tar;
	v3 offset;

	v3 dir;
	v3 rot;

	v3 u; /* right */
	v3 v; /* up */
	v3 n; /* backward */

	f32 fov;
	f32 near_clip;
	f32 far_clip;

	bool target_locked;
	bool perspective;

	m4x4 transform;
};

void cam_load(struct Camera *cam, u32 id, char *name, v3 pos, v3 tar, v3 dir, v3 rot,
		 f32 fov, f32 nc, f32 fc, bool locked, bool persp);

void cam_load_default(struct Camera *cam, u32 id, char *name);
