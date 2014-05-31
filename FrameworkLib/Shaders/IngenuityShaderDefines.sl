// ----------------------------------------
// DirectX9 - Shader Model 2
// ----------------------------------------

#ifdef INGENUITY_DX9

#define BEGIN_PARAMS 
#define END_PARAMS(N) 

#define DEFAULT_VTX_PARAMS \
uniform extern float4x4 worldViewProjection;\
uniform extern float4x4 world;\
uniform extern float4x4 worldInverseTranspose;\
uniform extern float4 materialColor;

#define DEFUALT_PIX_PARAMS \
uniform extern float3 lightPosition;\
uniform extern float specularPower;\
uniform extern float4 lightColor;\
uniform extern float3 cameraPosition;\
uniform extern float ambient;\
uniform extern float3 spotDirection;\
uniform extern float spotPower;\
uniform extern float cubeMapAlpha;

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

#ifdef INGENUITY_DX11

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
float3 filler;

#define PARAM_MATRIX(N) float4x4 param ## N;
#define PARAM_VECTOR(N) float4 param ## N;
#define PARAM_FLOAT(N)  float param ## N;

#define TECHNIQUE(VTX)

#endif

// ----------------------------------------
// WebGL - GLSL ES
// ----------------------------------------

#ifdef INGENUITY_WEB

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

#define DEFAULT_PIX_PARAMS \
PIX_PARAMS_AMBLIGHT \
PIX_PARAMS_DIRLIGHT \
PIX_PARAMS_HEMLIGHT \
PIX_PARAMS_PNTLIGHT \
PIX_PARAMS_SPTLIGHT \
PIX_PARAMS_WORLDPOS \
PIX_PARAMS_WRAPRGB \
uniform vec3 ambientLightColor; \
varying vec3 vViewPosition; \
varying vec3 vNormal;
// THE LAST TWO SHOULD BE VERTEX ATTRIBUTES

#define PARAM_MATRIX(N) uniform mat4 param ## N;
#define PARAM_VECTOR(N) uniform vec4 param ## N;
#define PARAM_FLOAT(N)  uniform float param ## N;

#define TECHNIQUE(VTX)

#endif