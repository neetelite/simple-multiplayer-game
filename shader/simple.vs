#version 330 core

layout (location = 0) in vec3 in_pos;

uniform mat4 mvp;

void main()
{
	vec4 position = vec4(in_pos, 1.0f);
	gl_Position = mvp * position;
}
