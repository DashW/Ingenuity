#version 150 core

in vec3 pass_Color;
in vec2 pass_TexCoord;

out vec4 out_Color;

uniform sampler2D textureSampler;

void main(void)
{
	out_Color = texture( textureSampler, pass_TexCoord );
}