#version 150 core

const float EPSILON = 0.00001;

vec4 diffuse(vec3 normalizedNormal, vec3 lightDirection, float lightDistance, vec3 lightColor, vec4 color)
{
	float diffuseMagnitude = max(dot(lightDirection, normalizedNormal), EPSILON);
	// diffuseMagnitude *= exp(-attenuation*lightDistance);
	vec3 diffuseColor = color.rgb * lightColor;
	return vec4(diffuseMagnitude * diffuseColor, color.a);
}

vec4 specular(vec3 normalizedNormal, vec3 cameraVector, vec3 lightDirection, vec3 lightColor, vec4 color)
{
	vec3 specularReflection = normalize(reflect(-lightDirection, normalizedNormal));
	float specularMagnitude = max(dot(cameraVector, specularReflection), EPSILON);
	float specularPoweredMagnitude = pow(specularMagnitude, 16.0);
	return color + vec4(specularPoweredMagnitude * lightColor, 0.0);
}

vec4 spot(vec3 lightDirection, vec3 spotDirection, float spotPower, vec4 color)
{
    float spotTheta = dot(normalize(-spotDirection), lightDirection);
    spotTheta = max(spotTheta, EPSILON);
    float spotFactor = pow(spotTheta, spotPower);

    return vec4((color.rgb * spotFactor), color.a);
}