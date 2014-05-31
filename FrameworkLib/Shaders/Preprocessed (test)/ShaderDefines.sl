// ----------------------------------------
// DirectX9 - Shader Model 2
// ----------------------------------------

#ifdef GPUAPI_DX9

#define BEGIN_PARAMS 
#define END_PARAMS(N) 

#define DEFAULT_VTX_PARAMS \
uniform extern float4x4 worldViewProjection;\
uniform extern float4x4 world;\
uniform extern float4x4 worldInverseTranspose;\
uniform extern float4 materialColor;

#define DEFAULT_PIX_PARAMS \
uniform extern float3 lightPosition;\
uniform extern float specularPower;\
uniform extern float4 lightColor;\
uniform extern float3 cameraPosition;\
uniform extern float ambient;\
uniform extern float3 spotDirection;\
uniform extern float spotPower;\
uniform extern float cubeMapAlpha;\
uniform extern float attenuation;

#define DEFAULT_TEX_PARAMS \
uniform extern texture2D tex;\
sampler2D textureSampler = sampler_state{\
	Texture = <tex>;\
	MinFilter = Anisotropic;\
	MagFilter = LINEAR;\
	MipFilter = LINEAR;\
	MaxAnisotropy = 8;\
	AddressU = Wrap;\
	AddressV = Wrap;\
};\
uniform extern texture cubeMap;\
samplerCUBE cubeMapSampler = sampler_state{\
	Texture = <cubeMap>;\
	MinFilter = LINEAR;\
	MagFilter = LINEAR;\
	MipFilter = LINEAR;\
	AddressU  = WRAP;\
    AddressV  = WRAP;\
};\
uniform extern texture2D normalMap;\
sampler2D normalSampler = sampler_state{\
	Texture = <normalMap>;\
	MinFilter = Anisotropic;\
	MagFilter = LINEAR;\
	MipFilter = LINEAR;\
	MaxAnisotropy = 8;\
	AddressU = Wrap;\
	AddressV = Wrap;\
};

#define BEGIN_VARYING(N) struct VertexOut ## N {
#define END_VARYING };
#define DEFAULT_VARYING float3 WorldPos : TEXCOORD0;\
float4 ProjPos : POSITION;\
float4 Color : COLOR;
#define VARYING_NORMAL float3 Normal : TEXCOORD1;

#define PARAM_MATRIX(N) uniform extern float4x4 param ## N;
#define PARAM_VECTOR(N) uniform extern float4 param ## N;
#define PARAM_FLOAT(N)  uniform extern float param ## N;

#define TECHNIQUE(VTX) technique VTX { \
pass p0 { \
VertexShader = compile vs_2_0 vtx ## VTX ## (); \
PixelShader = compile ps_2_0 pix ## VTX ## (); \
}}

#endif

// ----------------------------------------
// DirectX11 - Shader Model 5
// ----------------------------------------

#ifdef GPUAPI_DX11

#define BEGIN_PARAMS uniform {
#define END_PARAMS(N) } : BUFFER[ N ];

#define DEFAULT_VTX_PARAMS \
float4x4 worldViewProjection;\
float4x4 world;\
float4x4 worldInverseTranspose;\
float4 materialColor;

#define DEFAULT_PIX_PARAMS \
float3 lightPosition;\
float specularPower;\
float4 lightColor;\
float3 cameraPosition;\
float ambient;\
float3 spotDirection;\
float spotPower;\
float cubeMapAlpha;\
float attenuation;\
float2 filler;

#define DEFAULT_TEX_PARAMS \
sampler2D textureSampler : register(s0) = sampler_state{\
	MinFilter = Anisotropic;\
	MagFilter = LINEAR;\
	MipFilter = LINEAR;\
	MaxAnisotropy = 8;\
	AddressU = Wrap;\
	AddressV = Wrap;
};\
samplerCUBE cubeMapSampler : register(s1) = sampler_state{\
	MinFilter = LINEAR;\
	MagFilter = LINEAR;\
	MipFilter = LINEAR;\
	AddressU  = WRAP;\
    AddressV  = WRAP;\
};\
sampler2D normalSampler : register(s2) = sampler_state{\
	MinFilter = Anisotropic;\
	MagFilter = LINEAR;\
	MipFilter = LINEAR;\
	MaxAnisotropy = 8;\
	AddressU = Wrap;\
	AddressV = Wrap;\
};

#define PARAM_MATRIX(N) float4x4 param ## N;
#define PARAM_VECTOR(N) float4 param ## N;
#define PARAM_FLOAT(N)  float param ## N;

#define BEGIN_VARYING(N) struct VertexOut ## N {
#define END_VARYING };
#define DEFAULT_VARYING float3 WorldPos : TEXCOORD0;\
float4 ProjPos : POSITION;\
float4 Color : COLOR;
#define VARYING_NORMAL float3 Normal : TEXCOORD1;

#define TECHNIQUE(VTX)

#endif

// ----------------------------------------
// WebGL - GLSL ES
// ----------------------------------------

#ifdef GPUAPI_WEB

#define BEGIN_PARAMS 
#define END_PARAMS(N) 

#define DEFAULT_VTX_PARAMS
#ifdef USE_COLOR
#define DEFAULT_VTX_PARAMS varying vec3 vColor;
#endif

// NOT ALLOWING PHONG_PER_PIXEL == false !!!

#define PIX_PARAMS_DIRLIGHT
#define PIX_PARAMS_HEMLIGHT
#define PIX_PARAMS_PNTLIGHT
#define PIX_PARAMS_SPTLIGHT
#define PIX_PARAMS_WORLDPOS
#define PIX_PARAMS_WRAPRGB
#define PIX_PARAMS_MAP
#define PIX_PARAMS_BUMPMAP
#define PIX_PARAMS_NORMALMAP
#define PIX_PARAMS_ENVMAP

#if MAX_DIR_LIGHTS > 0
#define PIX_PARAMS_DIRLIGHT \
uniform vec3 directionalLightColor[ MAX_DIR_LIGHTS ]; \
uniform vec3 directionalLightDirection[ MAX_DIR_LIGHTS ];
#endif
#if MAX_HEMI_LIGHTS > 0
#define PIX_PARAMS_HEMLIGHT \
uniform vec3 hemisphereLightSkyColor[ MAX_HEMI_LIGHTS ]; \
uniform vec3 hemisphereLightGroundColor[ MAX_HEMI_LIGHTS ]; \
uniform vec3 hemisphereLightDirection[ MAX_HEMI_LIGHTS ];
#endif
#if MAX_POINT_LIGHTS > 0
#define PIX_PARAMS_PNTLIGHT \
uniform vec3 pointLightColor[ MAX_POINT_LIGHTS ]; \
uniform vec3 pointLightPosition[ MAX_POINT_LIGHTS ]; \
uniform float pointLightDistance[ MAX_POINT_LIGHTS ];
#endif
#if MAX_SPOT_LIGHTS > 0
#define PIX_PARAMS_SPTLIGHT \
uniform vec3 spotLightColor[ MAX_SPOT_LIGHTS ]; \
uniform vec3 spotLightPosition[ MAX_SPOT_LIGHTS ]; \
uniform vec3 spotLightDirection[ MAX_SPOT_LIGHTS ]; \
uniform float spotLightAngleCos[ MAX_SPOT_LIGHTS ]; \
uniform float spotLightExponent[ MAX_SPOT_LIGHTS ]; \
uniform float spotLightDistance[ MAX_SPOT_LIGHTS ];
#endif
#if MAX_SPOT_LIGHTS > 0 || defined( USE_BUMPMAP )
#define PIX_PARAMS_WORLDPOS varying vec3 vWorldPosition;
#endif
#ifdef WRAP_AROUND
#define PIX_PARAMS_WRAPRGB uniform vec3 wrapRGB;
#endif

#if defined( USE_MAP ) || defined( USE_BUMPMAP ) || defined( USE_NORMALMAP ) || defined( USE_SPECULARMAP )
#define PIX_PARAMS_MAP varying vec2 vUv;
#endif

#ifdef USE_BUMPMAP
#define PIX_PARAMS_BUMPMAP uniform float bumpScale;\
vec2 dHdxy_fwd() {\
vec2 dSTdx = dFdx( vUv );\
vec2 dSTdy = dFdy( vUv );\
float Hll = bumpScale * texture2D( bumpMap, vUv ).x;\
float dBx = bumpScale * texture2D( bumpMap, vUv + dSTdx ).x - Hll;\
float dBy = bumpScale * texture2D( bumpMap, vUv + dSTdy ).x - Hll;\
return vec2( dBx, dBy );\
}\
vec3 perturbNormalArb( vec3 surf_pos, vec3 surf_norm, vec2 dHdxy ) {\
vec3 vSigmaX = dFdx( surf_pos );\
vec3 vSigmaY = dFdy( surf_pos );\
vec3 vN = surf_norm;\
vec3 R1 = cross( vSigmaY, vN );\
vec3 R2 = cross( vN, vSigmaX );\
float fDet = dot( vSigmaX, R1 );\
vec3 vGrad = sign( fDet ) * ( dHdxy.x * R1 + dHdxy.y * R2 );\
return normalize( abs( fDet ) * surf_norm - vGrad );\
}
#endif

#ifdef USE_NORMALMAP
#define PIX_PARAMS_NORMALMAP uniform vec2 normalScale;\
vec3 perturbNormal2Arb( vec3 eye_pos, vec3 surf_norm ) {\
vec3 q0 = dFdx( eye_pos.xyz );\
vec3 q1 = dFdy( eye_pos.xyz );\
vec2 st0 = dFdx( vUv.st );\
vec2 st1 = dFdy( vUv.st );\
vec3 S = normalize(  q0 * st1.t - q1 * st0.t );\
vec3 T = normalize( -q0 * st1.s + q1 * st0.s );\
vec3 N = normalize( surf_norm );\
vec3 mapN = texture2D( normalMap, vUv ).xyz * 2.0 - 1.0;\
mapN.xy = normalScale * mapN.xy;\
mat3 tsn = mat3( S, T, N );\
return normalize( tsn * mapN );\
}
#endif

#define ENVMAP_PARAMS_NORMALMAP varying vec3 vReflect;
#if defined( USE_BUMPMAP ) || defined( USE_NORMALMAP )
#define ENVMAP_PARAMS_NORMALMAP \
uniform bool useRefract; \
uniform float refractionRatio;
#endif

#ifdef USE_ENVMAP
#define PIX_PARAMS_ENVMAP \
uniform float reflectivity;\
uniform samplerCube envMap;\
uniform float flipEnvMap;\
uniform int combine;\
ENVMAP_PARAMS_NORMALMAP
#endif

#define DEFAULT_PIX_PARAMS \
PIX_PARAMS_AMBLIGHT \
PIX_PARAMS_DIRLIGHT \
PIX_PARAMS_HEMLIGHT \
PIX_PARAMS_PNTLIGHT \
PIX_PARAMS_SPTLIGHT \
PIX_PARAMS_WORLDPOS \
PIX_PARAMS_WRAPRGB \
PIX_PARAMS_MAP \
PIX_PARAMS_BUMPMAP \
PIX_PARAMS_NORMALMAP \
PIX_PARAMS_SPECULARMAP \
PIX_PARAMS_ENVMAP \
uniform vec3 ambientLightColor; \
varying vec3 vViewPosition; \
varying vec3 vNormal;
// THE LAST TWO SHOULD BE VERTEX ATTRIBUTES

#define DEFAULT_MAP
#ifdef USE_MAP
#define DEFAULT_MAP uniform sampler2D map;
#endif

#define DEFAULT_BUMPMAP
#ifdef USE_BUMPMAP
#define DEFAULT_BUMPMAP uniform sampler2D bumpMap;
#endif

#define DEFAULT_NORMALMAP
#ifdef USE_NORMALMAP
#define DEFAULT_NORMALMAP uniform sampler2D normalMap;
#endif

#define DEFAULT_SPECULARMAP
#ifdef USE_SPECULARMAP
#define DEFAULT_SPECULARMAP uniform sampler2D specularMap;
#endif

#define DEFAULT_ENVMAP
#ifdef USE_ENVMAP
#define DEFAULT_ENVMAP uniform samplerCube envMap;
#endif

#define DEFAULT_TEX_PARAMS \
DEFAULT_MAP \
DEFAULT_BUMPMAP \
DEFAULT_NORMALMAP \
DEFAULT_SPECULARMAP \
DEFAULT_ENVMAP

#define PARAM_MATRIX(N) uniform mat4 param ## N;
#define PARAM_VECTOR(N) uniform vec4 param ## N;
#define PARAM_FLOAT(N)  uniform float param ## N;

#define VARYING_WORLDPOS
#if MAX_SPOT_LIGHTS > 0 || defined( USE_BUMPMAP )
#define VARYING_WORLDPOS varying vec3 vWorldPosition;
#endif

#define BEGIN_VARYING(N)
#define END_VARYING
#define DEFAULT_VARYING VARYING_WORLDPOS \
varying vec3 vViewPosition; \
varying vec3 vNormal;
#define VARYING_NORMAL

#define TECHNIQUE(VTX)

#endif