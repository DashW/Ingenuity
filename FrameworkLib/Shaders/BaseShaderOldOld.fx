uniform extern float4x4 world;
uniform extern float4x4 worldInverseTranspose;
uniform extern float4x4 worldViewProjection;

uniform extern float4 materialColor;
uniform extern float4 lightColor;
uniform extern float3 lightVector;
uniform extern float3 lightPosition;
uniform extern float3 attenuation;
uniform extern float specularPower;
uniform extern float spotPower;
uniform extern bool positionalLight;
uniform extern bool useLighting;

uniform extern float3 cameraPosition;

uniform extern texture tex;

struct VSOutput
{
	float4 position : POSITION0;
	float4 color : COLOR0;
};

struct VSComplexOutput
{
	float3 worldPosition : TEXCOORD0;
	float4 projectedPosition : POSITION0;
	float3 normal : TEXCOORD1;
	float4 color : COLOR0;
};

struct VSTexOutput
{
	float3 worldPosition : TEXCOORD0;
	float4 projectedPosition : POSITION0;
	float3 normal : TEXCOORD1;
	float2 texCoord : TEXCOORD2;
};

sampler textureSampler = sampler_state
{
	Texture = <tex>;
	MinFilter = Anisotropic;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	MaxAnisotropy = 8;
	AddressU  = WRAP;
    AddressV  = WRAP;
};

VSOutput TransformVS_PosCol(float3 inputPosition : POSITION0, float4 inputColor : COLOR0)
{
	VSOutput output = (VSOutput) 0;
	output.position = mul(float4(inputPosition, 1.0f), worldViewProjection);
	output.color = inputColor;
	
	return output;
}

VSComplexOutput TransformVS_PosNor(float3 inputPosition : POSITION0, float3 inputNormal : NORMAL0)
{
	VSComplexOutput output = (VSComplexOutput) 0;
	output.worldPosition =     mul(float4(inputPosition, 1.0f), world).xyz;
	output.projectedPosition = mul(float4(inputPosition, 1.0f), worldViewProjection);
	output.normal =            mul(float4(inputNormal,   0.0f), worldInverseTranspose).xyz;
	output.color =             materialColor;

	return output;
}

VSTexOutput TransformVS_PosNorTex(float3 inputPosition : POSITION0, float3 inputNormal : NORMAL0, float2 texcoord : TEXCOORD0)
{
	VSTexOutput output = (VSTexOutput) 0;
	output.worldPosition =     mul(float4(inputPosition, 1.0f), world).xyz;
	output.projectedPosition = mul(float4(inputPosition, 1.0f), worldViewProjection);
	output.normal =            mul(float4(inputNormal,   0.0f), worldInverseTranspose).xyz;
	output.texCoord = texcoord;

	return output;
}


float4 TransformPS(float4 inputColor : COLOR0) : COLOR
{
	return inputColor;
}

float4 TransformPS_PixelLighting(float3 inputPosition : TEXCOORD0, 
				   float4 inputProjected : POSITION0,
				   float3 inputNormal : TEXCOORD1, 
				   float4 inputColor : COLOR0) : COLOR
{

	if(useLighting)
	{
		float3 normalizedNormal = normalize(inputNormal);

		float3 lightDirection;
		float attenuationFactor;
		float spotFactor;
		if(positionalLight) // POINTLIGHT or SPOTLIGHT
		{
			lightDirection = normalize(lightPosition - inputPosition);
		
			float distanceToLight = distance(lightPosition, inputPosition);
			attenuationFactor = attenuation.x 
				+ (attenuation.y * distanceToLight) 
				+ (attenuation.z * distanceToLight * distanceToLight);
		
			spotFactor = pow(max(dot(-lightVector, lightDirection), 0.0f), spotPower);
		}
		else // DIRECTIONALLIGHT
		{
			attenuationFactor = 1.0f;
			spotFactor = 1.0f;
			lightDirection = lightVector;
		}
		
		float4 diffuseMagnitude = max(dot(lightDirection, normalizedNormal), 0.0f);
		float4 diffuseColor = inputColor * lightColor;
		float4 diffuse = float4((diffuseMagnitude * diffuseColor).rgb, inputColor.a);

		float3 cameraVector = normalize(cameraPosition - inputPosition);
		float3 specularReflection = reflect(-lightDirection,normalizedNormal);
		float specularMagnitude = pow(max(dot(cameraVector,specularReflection),0.0f),specularPower);
		float4 specular = float4((specularMagnitude * lightColor).rgb, 1.0f);

		return ((diffuse + specular) / attenuationFactor) * spotFactor;
	}
	else
	{
		return inputColor;
	}
}

float4 TransformPS_Tex(float3 inputPosition : TEXCOORD0,
					   float4 inputProjected : POSITION0,
					   float3 inputNormal : TEXCOORD1,
					   float2 texCoord : TEXCOORD2) : COLOR
{
	float4 color = tex2D(textureSampler, texCoord);

	return TransformPS_PixelLighting(inputPosition, inputProjected, inputNormal, color);
}

technique TransfromTech_PosCol
{
	pass P0
	{
		vertexShader = compile vs_2_0 TransformVS_PosCol();
		pixelShader = compile ps_2_0 TransformPS();
	}
}

technique TransformTech_PosNor
{
	pass p0
	{
		vertexShader = compile vs_2_0 TransformVS_PosNor();
		pixelShader = compile ps_2_0 TransformPS_PixelLighting();
	}
}

technique TransformTech_PosNorTex
{
	pass p0
	{
		vertexShader = compile vs_2_0 TransformVS_PosNorTex();
		pixelShader = compile ps_2_0 TransformPS_Tex();
	}
}