#version 330 core

uniform vec4 color;

out vec4 out_fragment;

void main()
{
	out_fragment = color;
}
