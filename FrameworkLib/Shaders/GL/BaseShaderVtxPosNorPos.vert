#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 inverseTransposeMatrix;

uniform vec4 materialColor;

in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_InstPos;

out vec3 pass_Position;
out vec4 pass_Color;
out vec3 pass_Normal;

void main(void)
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);
	pass_Position = vec3(modelMatrix * vec4(in_Position,1.0)) + in_InstPos;
	pass_Normal = (inverseTransposeMatrix * vec4(in_Normal,0.0)).xyz;
	pass_Color = materialColor;
}