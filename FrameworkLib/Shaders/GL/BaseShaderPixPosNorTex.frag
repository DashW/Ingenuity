#version 150 core

// Forward Declarations
vec4 diffuse(vec3 normalizedNormal, vec3 lightDirection, float lightDistance, vec3 lightColor, vec4 color);
vec4 specular(vec3 normalizedNormal, vec3 cameraVector, vec3 lightDirection, vec3 lightColor, vec4 color);
vec4 spot(vec3 lightDirection, vec3 spotDirection, float spotPower, vec4 color);

uniform vec3 lightPositions[6];
uniform vec3 lightColors[6];
uniform vec4 lightSpotDirPowers[6];
uniform uint numLights;

uniform vec3 cameraPosition;

in vec3 pass_Position;
in vec3 pass_Normal;
in vec2 pass_TexCoord;

out vec4 out_Color;

uniform sampler2D textureSampler;

void main(void)
{
	vec3 normalizedNormal = normalize(pass_Normal);
	vec3 cameraVector = normalize(cameraPosition - pass_Position);
	vec4 texColor = texture( textureSampler, pass_TexCoord );

	out_Color = vec4(0.0);
	
	for(uint i = 0u; i < numLights; ++i)
	{
		vec3 lightDirection = normalize(lightPositions[i] - pass_Position);
		float lightDistance = length(lightPositions[i] - pass_Position);
		vec4 resultColor = vec4(1.0,1.0,1.0,1.0);

		resultColor = diffuse( normalizedNormal, lightDirection, lightDistance, lightColors[i], resultColor );
			// out_Color = cubeReflect(normalizedNormal,vtx.WorldPos,output.color);
		resultColor = specular( normalizedNormal, cameraVector, lightDirection, lightColors[i], resultColor );
		resultColor = spot( lightDirection, lightSpotDirPowers[i].xyz, lightSpotDirPowers[i].w, resultColor );

		out_Color += (resultColor * texColor);
	}

	out_Color.a = texColor.a;
}