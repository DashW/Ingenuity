#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

in vec3 in_Position;

out vec4 pass_Position;

void main(void)
{
	pass_Position = modelMatrix * vec4(in_Position, 1.0);
	gl_Position = (projectionMatrix * viewMatrix) * pass_Position;
}