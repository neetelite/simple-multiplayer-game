#define GL_BUFFER_OFFSET(offset) (( void *)(offset))

/* Functions */
void gl_enable(u32 flag);
void gl_program_bind(u32 handle);
void gl_program_unging(void);
void gl_vao_gen(u32 count, u32 *vao);
void gl_vbo_gen(u32 count, u32 *vbo);
void gl_vao_bind(u32 vao);
void gl_vao_unbind();
void gl_vao_draw(u32 start, u32 count, u32 type);
void gl_vbo_bind(u32 vbo, u32 target);
void gl_vbo_unbind(u32 target);
void gl_vbo_data(void *data, u64 size, u32 target, u32 draw);
void gl_vbo_attrib_set(u32 index, u32 count, u32 type, u32 stride, void *offset);
void gl_vbo_attrib_enable(u32 index);
void gl_texture_bind(struct Texture *texture, GLenum target);
void gl_texture_unbind(GLenum target);
void gl_texture_gen(struct Texture *texture, GLenum target, u32 sampling);
void gl_viewport_set_dimension(v4i dimension);
void gl_viewport_set_color(v4 color);
void gl_viewport_set_color_u32(u32 color);
void gl_viewport_clear_color(void);
void gl_viewport_clear_depth(void);
void gl_viewport_clear(void);

void gl_elements_draw(u32 target, u32 count, u32 type, void *indices);

i32 gl_shader_load(char *shader_path, GLenum shader_type);
