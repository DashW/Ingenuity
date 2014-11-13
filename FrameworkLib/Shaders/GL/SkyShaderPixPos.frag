#version 150 core

uniform vec3 cameraPosition;
uniform samplerCube cubeMapSampler;

in vec4 pass_Position;

out vec4 out_Color;

void main(void)
{
	out_Color = texture( cubeMapSampler, pass_Position.xyz - cameraPosition );
}