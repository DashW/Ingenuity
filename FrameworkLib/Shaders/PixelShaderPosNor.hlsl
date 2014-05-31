cbuffer cbPerScreen
{
	float3 lightDirection;
	float4 lightColor;
	float3 cameraPosition;
	float specularPower;
} : BUFFER[1];

struct VertexOut
{
	float3 WorldPos : TEXCOORD0;
	float4 ProjPos  : POSITION;
	float3 Normal : TEXCOORD1;
    float4 Color : COLOR;
};

struct PixelOut 
{
  float4 color : COLOR;
};

float4 diffuse(VertexOut vtx)
{
	float3 normalizedNormal = normalize(vtx.Normal);
	float4 diffuseMagnitude = max(dot(lightDirection, normalizedNormal),0.0f);
	float4 diffuseColor = vtx.Color * lightColor;
	return float4((diffuseMagnitude * diffuseColor).rgb, vtx.Color.a);
}

float4 specular(VertexOut vtx)
{
	float3 cameraVector = normalize(cameraPosition - vtx.WorldPos);
	float3 specularReflection = normalize(reflect(-lightDirection,vtx.Normal));
	float specularMagnitude = max(dot(cameraVector,specularReflection),0.0f);
	float specularPoweredMagnitude = pow(specularMagnitude,16.0f);
//	float4 specular = float4((specularMagnitude * lightColor).rgb, 1.0f);
	return specularPoweredMagnitude * lightColor;   
}

PixelOut main(VertexOut vtx)
{
	PixelOut output;
	output.color = diffuse(vtx) + specular(vtx);
	return output;
}