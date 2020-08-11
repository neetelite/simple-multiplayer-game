void
gl_enable(u32 flag)
{
	glEnable(flag);
}

void
gl_program_bind(u32 handle)
{
	glUseProgram(handle);
}

void
gl_program_unbind(void)
{
	glUseProgram(0);
}


void
gl_vao_gen(u32 count, u32 *vao)
{
	glGenVertexArrays(count, vao);
}

void
gl_vbo_gen(u32 count, u32 *vbo)
{
	glGenBuffers(count, vbo);
}

void
gl_vao_bind(u32 vao)
{
	glBindVertexArray(vao);
}

void
gl_vao_unbind()
{
	glBindVertexArray(0);
}
void
gl_vao_draw(u32 start, u32 count, u32 type)
{
	glDrawArrays(type, start, count);
}

void
gl_vbo_bind(u32 vbo, u32 target)
{
	glBindBuffer(target, vbo);
}

void
gl_vbo_unbind(u32 target)
{
	glBindBuffer(target, 0);
}


void
gl_vbo_data(void *data, u64 size, u32 target, u32 draw)
{
	glBufferData(target, size, data, draw);
}

void
gl_vbo_attrib_set(u32 index, u32 count, u32 type, u32 stride, void *offset)
{
	/* FIXME LEARN(lungu): Do we really need normalized value? */
	bool normalized = GL_FALSE;

	switch(type)
	{
	case GL_FLOAT:
	{
		glVertexAttribPointer(index, count, type, normalized, stride, offset);
	} break;
	case GL_UNSIGNED_INT:
	{
		glVertexAttribIPointer(index, count, type, stride, offset);
	} break;
	}
}

void
gl_vbo_attrib_enable(u32 index)
{
	glEnableVertexAttribArray(index);
}

void
gl_texture_bind(struct Texture *texture, GLenum target)
{
	glBindTexture(target, texture->handle);
}

void
gl_texture_unbind(GLenum target)
{
	glBindTexture(target, 0);
}

void
gl_texture_gen(struct Texture *texture, GLenum target, u32 sampling)
{
	glGenTextures(1, &texture->handle);
	gl_texture_bind(texture, target);

	if(texture->image.channel_count == 4)
	{
		glTexImage2D(target, 0, GL_RGBA, texture->image.width, texture->image.height,
			     0, GL_BGRA, GL_UNSIGNED_BYTE, texture->image.data);
	}
	else if(texture->image.channel_count = 3)
	{
		/* FIXME TEMPORARY */
		#define GL_BGR 0
		glTexImage2D(target, 0, GL_RGBA, texture->image.width, texture->image.height,
			     0, GL_BGR, GL_UNSIGNED_BYTE, texture->image.data);
	}
	else
	{
		//ERROR
	}

	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if(sampling == GL_LINEAR)
	{
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if(sampling == GL_NEAREST)
	{
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	gl_texture_unbind(target);
}

void
gl_viewport_set_dimension(v4i dimension)
{
	glViewport(dimension.min.width, dimension.min.height, dimension.max.width, dimension.max.height);
}

void
gl_viewport_set_color(v4 color)
{
	glClearColor(color.r, color.g, color.b, color.a);
}

void
gl_viewport_set_color_u32(u32 color)
{
	v4 c = v4_from_u32(color);
	c = v4_df(c, 256.0f);
	glClearColor(c.r, c.g, c.b, c.a);
}

void
gl_viewport_clear_color(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void
gl_viewport_clear_depth(void)
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void
gl_viewport_clear(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void
gl_elements_draw(u32 target, u32 count, u32 type, void *indices)
{
	glDrawElements(target, count, type, indices);
}

i32
gl_shader_load(char *shader_path, GLenum shader_type)
{
	/* TODO(lungu): Maybe let shader be a pointer so when it returns null we know it failed */
	u32 result = 0;

	FILE *file = fopen(shader_path, "rb");
	if(!file)
	{
		ERROR("Cannot open file for reading! %s\n", shader_path);
		return(-1);
	}

	u32 size = 0;
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *shader_source = mem_alloc(size+1, false);
	fread(shader_source, size, 1, file);
	shader_source[size] = '\0';
	fclose(file);

	result = glCreateShader(shader_type);
	glShaderSource(result, 1, (char **)&shader_source, NULL);
	glCompileShader(result);

	i32 success;
	char info_log[512];
	glGetShaderiv(result, GL_COMPILE_STATUS, &success);

	if(!success)
	{
		glGetShaderInfoLog(result, 512, NULL, info_log);
		//ERROR("Failed to compile shader (%s): %s\n", shader_path, info_log);
		printf("[ERROR]: Failed to compile shader (%s): %s\n", shader_path, info_log);
		return(-1);
	}

	mem_free(shader_source);

	return(result);
}
