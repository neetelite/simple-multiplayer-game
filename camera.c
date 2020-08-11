void
cam_load(struct Camera *cam, u32 id, char *name,
	    v3 pos, v3 tar, v3 dir, v3 rot,
	    f32 fov, f32 nc, f32 fc, bool locked, bool persp)
{
	/* Camera 0 */
	cam->id = id;

	if(cam->name) mem_free(cam->name);
	cam->name = name;

	cam->pos = pos;
	cam->tar = tar;
	cam->dir = dir;
	cam->rot = rot;

	cam->fov = fov; /* NOTE(lungu): Less than 180, greater than ?? */
	cam->near_clip = nc;
	cam->far_clip = fc;
	cam->target_locked = locked;
	cam->perspective = persp;
}

void
cam_load_default(struct Camera *cam, u32 id, char *name)
{
	cam_load(cam, id, name,
		 V3_ZERO, V3_FRONT, /* pos, tar */
		 V3_FRONT, V3_ZERO, /* dir, rot */
		 90.0f, 0.01f, 1000.0f, false, true); /* fov, nc, fc, locked, persps */
}

void
cam_matrix_target(struct Camera *cam)
{
	/* "Look At" Matrix */
	/* pos, tar, up */
	cam->n = v3_norm(v3_s(cam->pos, cam->tar));

	cam->u = v3_norm(v3_cross(cam->v, cam->n));
	cam->v = v3_cross(cam->n, cam->u);

	m4x4 look_at = MAT4
		(
			cam->u.x, cam->u.y, cam->u.z, 0,
			cam->v.x,    cam->v.y,    cam->v.z,    0,
			cam->n.x,   cam->n.y,   cam->n.z,   0,
			0,            0,            0,            1
		);

	m4x4 pos_neg = MAT4
		(
			1, 0, 0, -cam->pos.x,
			0, 1, 0, -cam->pos.y,
			0, 0, 1, -cam->pos.z,
			0, 0, 0,           1
		);

	cam->transform = m4x4_m(look_at, pos_neg);
}

void
cam_matrix_free(struct Camera *cam)
{
	/* STEP 1: n = <target position - view reference point> */
	if(cam->target_locked)
	{
		cam->n = v3_s(cam->tar, cam->pos);
	}
	else
	{
		cam->n = v3_inv(cam->dir);
	}

	/* STEP 2: let v = up */
	cam->v = V3_UP;

	/* STEP 3: u = (v x n) */
	v3p_cross(&cam->u, &cam->v, &cam->n);

	/* STEP 4: v = (n x u) */
	v3p_cross(&cam->v, &cam->n, &cam->u);

	/* STEP 5: normalize all vectors */
	v3_norm_out(&cam->u, &cam->u);
	v3_norm_out(&cam->v, &cam->v);
	v3_norm_out(&cam->n, &cam->n);

	m4x4 look_at = MAT4
		(
			cam->u.x, cam->u.y, cam->u.z, 0,
			cam->v.x, cam->v.y, cam->v.z, 0,
			cam->n.x, cam->n.y, cam->n.z, 0,
			0,        0,        0,        1
		);

	m4x4 pos_neg = MAT4
		(
			1, 0, 0, -cam->pos.x,
			0, 1, 0, -cam->pos.y,
			0, 0, 1, -cam->pos.z,
			0, 0, 0,           1
		);

	cam->transform = m4x4_m(look_at, pos_neg);
}

void
cam_matrix(struct Camera *cam)
{
	/* View */
	if(cam->target_locked) cam_matrix_target(cam);
	else cam_matrix_free(cam);

	/* Projection */
	m4x4 matrix_projection;

	f32 n = cam->near_clip; /* NOTE(lungu): Can't be negative or zero */
	f32 f = cam->far_clip;
	if(cam->perspective)
	{
		f64 fov = 1.0 / tan(f32_rad_from_deg(cam->fov) / 2);

		f32 A = fov * 1.0f;
		/* TEMPORARY */
		f32 B = fov * ((f32)os_context.width / os_context.height);
		f32 C = (n+f) / (n-f);
		f32 D = (2*n*f) / (n-f);

		matrix_projection = MAT4
			(
				A, 0,  0, 0,
				0, B,  0, 0,
				0, 0,  C, D,
				0, 0, -1, 0
			);
	}
	else
	{
		/* TODO(lungu): Scale down to screen size before rendering */
		/* TODO(lungu): I don't think this is the correct matrix */
		f32 A = 1.0f;
		f32 B = ((f32)os_context.width / os_context.height);
		f32 C = 2.0f / (n-f);
		f32 D = (n+f) / (n-f);

		matrix_projection = MAT4
			(
				A, 0, 0, 0,
				0, B, 0, 0,
				0, 0, C, D,
				0, 0, 0, 1
			);
	}

	cam->transform = m4x4_m(matrix_projection, cam->transform);
	cam->transform = mat4_transpose(cam->transform);
}
