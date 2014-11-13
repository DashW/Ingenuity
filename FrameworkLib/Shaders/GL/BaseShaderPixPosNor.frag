#version 150 core

// Forward Declarations
vec4 diffuse( vec3 normalizedNormal, vec3 lightDirection, float lightDistance, vec3 lightColor, vec4 color );
vec4 cubeReflect( vec3 normalizedNormal, vec3 cameraVector, vec3 worldPos, samplerCube cubeMapSampler, float cubeMapAlpha, vec4 color );
vec4 specular( vec3 normalizedNormal, vec3 cameraVector, vec3 lightDirection, vec3 lightColor, vec4 color );
vec4 spot( vec3 lightDirection, vec3 spotDirection, float spotPower, vec4 color );

uniform vec3 lightPositions[6];
uniform vec3 lightColors[6];
uniform vec4 lightSpotDirPowers[6];
uniform uint numLights;

uniform vec3 cameraPosition;
uniform float cubeMapAlpha;

in vec3 pass_Position;
in vec4 pass_Color;
in vec3 pass_Normal;

out vec4 out_Color;

uniform samplerCube cubeMapSampler;

void main(void)
{
	vec3 normalizedNormal = normalize(pass_Normal);
	vec3 cameraVector = normalize(cameraPosition - pass_Position);

	out_Color = vec4(0.0);
	
	for(uint i = 0u; i < numLights; ++i)
	{
		vec3 lightDirection = normalize(lightPositions[i] - pass_Position);
		float lightDistance = length(lightPositions[i] - pass_Position);
		vec4 resultColor = pass_Color;

		resultColor = diffuse( normalizedNormal, lightDirection, lightDistance, lightColors[i], resultColor );
		resultColor = cubeReflect( normalizedNormal, cameraVector, pass_Position, cubeMapSampler, cubeMapAlpha, resultColor );
		resultColor = specular( normalizedNormal, cameraVector, lightDirection, lightColors[i], resultColor );
		resultColor = spot( lightDirection, lightSpotDirPowers[i].xyz, lightSpotDirPowers[i].w, resultColor );

		out_Color += resultColor;
	}

	out_Color.a = pass_Color.a;
}