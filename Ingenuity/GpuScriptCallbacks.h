#pragma once

#include "ScriptInterpreter.h"
#include "AssetMgr.h"
#include <RealtimeApp.h>
#include <vector>

namespace Ingenuity {

struct LocalMesh;

class GpuScriptCallbacks
{
	GpuScriptCallbacks();

	static ScriptParam VertexBufferToFloats(ScriptInterpreter * interpreter, IVertexBuffer * vertexBuffer);
	static LocalMesh * FloatsToLocalMesh(ScriptInterpreter * interpreter, ScriptParam type, ScriptParam vtx, ScriptParam idx);
	static Gpu::FloatArray * ScriptToGpuFloats(ScriptFloatArray * floats);

public:
	static void RegisterWith(ScriptInterpreter * interpreter)
	{

		// GPU
		interpreter->RegisterCallback("CreateModel", &GpuScriptCallbacks::CreateModel,
			"(type,vertices,indices) Creates a model with the given vertex/index data");
		interpreter->RegisterCallback("CreateSpriteModel", &GpuScriptCallbacks::CreateSpriteModel,
			"(texture) Creates a textured quad model, scaled to the match the texture size");
		interpreter->RegisterCallback("DecomposeModel", &GpuScriptCallbacks::DecomposeModel,
			"(model) Returns an array of complex models, each with one sub-model");

		interpreter->RegisterCallback("DrawComplexModel", &GpuScriptCallbacks::DrawModel,
			"(model,camera[,lights,surface,instances,effect]) Draws a model");
		interpreter->RegisterCallback("GetFont", &GpuScriptCallbacks::GetFont,
			"(size,family[,style,isPixelSpace]) Loads a font [with style \"regular\", \"bold\" or \"italic\"]");
		interpreter->RegisterCallback("DrawText", &GpuScriptCallbacks::DrawText,
			"(font,text,x,y[,center=false])");
		interpreter->RegisterCallback("SetFontColor", &GpuScriptCallbacks::SetFontColor,
			"(font,r,g,b[,a]) Sets the color of a font. All text using this font will be drawn with this color");
		interpreter->RegisterCallback("CreateEffect", &GpuScriptCallbacks::CreateEffect,
			"(name) Creates an effect with the given shader");
		interpreter->RegisterCallback("SetClearColor", &GpuScriptCallbacks::SetClearColor,
			"(r,g,b[,a]) Sets the background color to which the screen should be cleared");
		interpreter->RegisterCallback("SetBlendMode", &GpuScriptCallbacks::SetBlendMode,
			"(mode) Sets the given blend mode to the GPU ('alpha','additive','none')");
		interpreter->RegisterCallback("SetWireframe", &GpuScriptCallbacks::SetWireframe,
			"(wireframe) Sets whether the GPU should draw all 3D models in wireframe mode");
		interpreter->RegisterCallback("SetMultisampling", &GpuScriptCallbacks::SetMultisampling,
			"(level) Sets the level of multisampling to be used when drawing to the backbuffer, 0-16");
		interpreter->RegisterCallback("SetAnisotropy", &GpuScriptCallbacks::SetAnisotropy,
			"(level) Sets the level of anisotropic filtering for the default texture sampler");
		interpreter->RegisterCallback("GetBaseEffect", &GpuScriptCallbacks::GetBaseEffect,
			"() Gets the default (base) effect for all GPU model drawing");
		interpreter->RegisterCallback("GetMonitorSize", &GpuScriptCallbacks::GetScreenSize,
			"() returns the width and height of the screen, in pixels");
		interpreter->RegisterCallback("GetBackbufferSize", &GpuScriptCallbacks::GetBackbufferSize,
			"([window]) returns the width and height of the [main, otherwise given] window, in pixels");

		// GPU OBJECTS
		interpreter->RegisterCallback("CreateCamera", &GpuScriptCallbacks::CreateCamera,
			"([orthographic]) Creates a new perspective (default) or orthographic camera");
		interpreter->RegisterCallback("CreateSpriteCamera", &GpuScriptCallbacks::CreateSpriteCamera,
			"(pixelSpace,centerOrigin,yUpwards,surface) Creates an orthographic camera for sprite drawing");
		interpreter->RegisterCallback("SetCameraPosition", &GpuScriptCallbacks::SetCameraPosition,
			"(camera,x,y,z) Sets the world position of a camera");
		interpreter->RegisterCallback("SetCameraTarget", &GpuScriptCallbacks::SetCameraTarget,
			"(camera,x,y,z) Sets the world position of the camera's look target");
		interpreter->RegisterCallback("SetCameraUp", &GpuScriptCallbacks::SetCameraUp,
			"(camera,x,y,z) Sets the UP vector of the given camera");
		interpreter->RegisterCallback("SetCameraClipFov", &GpuScriptCallbacks::SetCameraClipFovOrHeight,
			"(camera,nearclip,farclip,fov) Sets the near and far clip panes and fov of a perspective camera");
		interpreter->RegisterCallback("SetCameraClipHeight", &GpuScriptCallbacks::SetCameraClipFovOrHeight,
			"(camera,nearclip,farclip,height) Sets the near and far clip panes and height of an orthographic camera");
		interpreter->RegisterCallback("GetCameraRay", &GpuScriptCallbacks::GetCameraRay,
			"(camera,x,y[,surface]) Returns the vector4 ray from the camera for the given point [on the surface]");
		interpreter->RegisterCallback("GetCameraMatrix", &GpuScriptCallbacks::GetCameraMatrix,
			"(camera[,surface,isTex]) Returns [texture] projection matrix for the camera [with surface's aspect ratio]");

		interpreter->RegisterCallback("CreateLight", &GpuScriptCallbacks::CreateLight,
			"(type) Creates a new light of type \"directional\", \"point\", or \"spot\"");
		interpreter->RegisterCallback("SetLightColor", &GpuScriptCallbacks::SetLightColor,
			"(light,r,g,b) Sets the color of a light, components between 0.0 and 1.0");
		interpreter->RegisterCallback("SetLightPosition", &GpuScriptCallbacks::SetLightPosition,
			"(light,x,y,z) Sets the world position of a light (point and spot only)");
		interpreter->RegisterCallback("SetLightDirection", &GpuScriptCallbacks::SetLightDirection,
			"(light,x,y,z) Sets the direction of a light (directional and spot only)");
		interpreter->RegisterCallback("SetLightAttenuation", &GpuScriptCallbacks::SetLightAttenuation,
			"(light,atten) Sets the attenuation value of a light (point and spot only)");
		interpreter->RegisterCallback("SetLightSpotPower", &GpuScriptCallbacks::SetLightSpotPower,
			"(light,power) Sets the spot power (narrowness of a spotlight's beam), 0.0 to infinity");

		interpreter->RegisterCallback("SetModelPosition", &GpuScriptCallbacks::SetModelPosition,
			"(model,x,y,z) Sets the world position of a model");
		interpreter->RegisterCallback("SetModelRotation", &GpuScriptCallbacks::SetModelRotation,
			"(model,x,y,z) Sets the world rotation of a model");
		interpreter->RegisterCallback("SetModelScale", &GpuScriptCallbacks::SetModelScale,
			"(model,scale[,scaleY,scaleZ]) Sets the local size of a model, optionally in each axis.");
		interpreter->RegisterCallback("SetModelMatrix", &GpuScriptCallbacks::SetModelMatrix,
			"(model,matrix) Sets the world transformation matrix of a model");
		interpreter->RegisterCallback("SetMeshPosition", &GpuScriptCallbacks::SetMeshPosition,
			"(model,meshnum,x,y,z) Sets the position of a mesh relative to its parent model");
		interpreter->RegisterCallback("SetMeshRotation", &GpuScriptCallbacks::SetMeshRotation,
			"(model,meshnum,x,y,z) Sets the rotation of a mesh relative to its parent model");
		interpreter->RegisterCallback("SetMeshScale", &GpuScriptCallbacks::SetMeshScale,
			"(model,meshnum,x[,y,z]) Sets the scale of a mesh relative to its parent model");
		interpreter->RegisterCallback("SetMeshMatrix", &GpuScriptCallbacks::SetMeshMatrix,
			"(model,meshnum,matrix) Sets the transform matrix to the given mesh of the model");
		interpreter->RegisterCallback("SetMeshEffect", &GpuScriptCallbacks::SetMeshEffect,
			"(model,meshnum,effect) Sets an effect to a specific mesh in a complex model");
		interpreter->RegisterCallback("SetMeshTexture", &GpuScriptCallbacks::SetMeshTexture,
			"(model,meshnum,texture) Sets a texture to a specific mesh in a complex model");
		interpreter->RegisterCallback("SetMeshNormal", &GpuScriptCallbacks::SetMeshNormal,
			"(model,meshnum,normal) Sets a normal texture to a specific mesh in a complex model");
		interpreter->RegisterCallback("SetMeshCubeMap", &GpuScriptCallbacks::SetMeshCubeMap,
			"(model,meshnum,cubemap) Sets a cube map to a specific mesh in a complex model");
		interpreter->RegisterCallback("SetMeshColor", &GpuScriptCallbacks::SetMeshColor,
			"(model,meshnum,r,g,b[,a]) Sets the color of a specific mesh in a complex model");
		interpreter->RegisterCallback("SetMeshSpecular", &GpuScriptCallbacks::SetMeshSpecular,
			"(model,meshnum,specular) Sets the specular power of a mesh in a complex model");
		interpreter->RegisterCallback("SetMeshFactors", &GpuScriptCallbacks::SetMeshFactors,
			"(model,meshnum,diffuse,specular) Sets multipliers for the diffuse/specular lighting components");
		interpreter->RegisterCallback("GetMeshBounds", &GpuScriptCallbacks::GetMeshBounds,
			"(model,meshnum) Returns the x,y,z origin and radius of the mesh's bounding sphere");
		interpreter->RegisterCallback("GetNumMeshes", &GpuScriptCallbacks::GetNumMeshes,
			"(model) Gets the number of meshes in a complex model");

		interpreter->RegisterCallback("SetEffectParam", &GpuScriptCallbacks::SetEffectParam,
			"(effect,paramnum,value) Sets the value of an effect parameter");
		interpreter->RegisterCallback("SetSamplerParam", &GpuScriptCallbacks::SetSamplerParam,
			"(effect,key,value[,paramnum]) Sets a sampler parameter for a given effect [texture parameter]");

		interpreter->RegisterCallback("CreateSurface", &GpuScriptCallbacks::CreateSurface,
			"([width,height,window,format]) Creates a draw surface of the given dimensions and format, default mimics backbuffer");
		interpreter->RegisterCallback("DrawSurface", &GpuScriptCallbacks::DrawSurface,
			"(surface[,effect,outSurface]) Draws a surface [to another surface with the given effect]");
		interpreter->RegisterCallback("ClearSurface", &GpuScriptCallbacks::ClearSurface,
			"(surface[,r,g,b,a]) Clears a surface to black [or to the given color values]");

		// Without having a central bank of pointers for reference-counting, this call is DANGEROUS
		// (i.e. if we're not careful, a pointer could be deleted more than once!)
		interpreter->RegisterCallback("GetSurfaceTexture", &GpuScriptCallbacks::GetSurfaceTexture,
			"(surface) Gets the texture object from a draw surface");

		interpreter->RegisterCallback("CreateIsoSurface", &GpuScriptCallbacks::CreateIsoSurface,
			"(length) Creates a cube IsoSurface with the given side length");
		interpreter->RegisterCallback("AddIsoSurfaceBall", &GpuScriptCallbacks::AddIsoSurfaceBall,
			"(iso,x,y,z,r) Adds a metaball to the IsoSurface with the given position and radius");
		interpreter->RegisterCallback("AddIsoSurfacePlane", &GpuScriptCallbacks::AddIsoSurfacePlane,
			"(iso,x,y,z,nx,ny,nz) Adds a plane to the IsoSurface with the given position and normal");
		interpreter->RegisterCallback("ClearIsoSurface", &GpuScriptCallbacks::ClearIsoSurface,
			"(iso) Clears all meta-objects from the IsoSurface");
		interpreter->RegisterCallback("GetIsoSurfaceModel", &GpuScriptCallbacks::GetIsoSurfaceModel,
			"(iso) Updates and gets the ComplexModel for the IsoSurface");

		interpreter->RegisterCallback("CreateInstanceBuffer", &GpuScriptCallbacks::CreateInstanceBuffer,
			"(type,size) Creates an instance buffer of the given type with the given capacity");
		interpreter->RegisterCallback("UpdateInstanceBuffer", &GpuScriptCallbacks::UpdateInstanceBuffer,
			"(ibuf,floats,size) Updates an instance buffer with the given FloatArray");

		// 2D DRAWING
		interpreter->RegisterCallback("CreateRect", &GpuScriptCallbacks::CreateRect,
			"(x,y,w,h) Creates a rectangle model of the specified dimensions");
		interpreter->RegisterCallback("CreateRectStroke", &GpuScriptCallbacks::CreateRectStroke,
			"(x,y,w,h,weight) Creates a rectangle stroke model of the specified dimensions & thickness");
		interpreter->RegisterCallback("CreateEllipse", &GpuScriptCallbacks::CreateEllipse,
			"(x,y,rx,ry) Creates an ellipse model of the specified dimensions");
		interpreter->RegisterCallback("CreateEllipseStroke", &GpuScriptCallbacks::CreateEllipseStroke,
			"(x,y,rx,ry,weight) Creates an ellipse stroke model of the specified dimensions & thickness");

		// SCENES
		interpreter->RegisterCallback("CreateScene", &GpuScriptCallbacks::CreateScene,
			"([type]) Creates a rendering scene. Valid types are \"linear\"(default) or \"instanced\"");
		interpreter->RegisterCallback("AddToScene", &GpuScriptCallbacks::AddToScene,
			"(scene,object) Add a mesh, texture or light to the scene");
		interpreter->RegisterCallback("ClearScene", &GpuScriptCallbacks::ClearScene,
			"(scene) Removes all objects from the scene");
		interpreter->RegisterCallback("DrawScene", &GpuScriptCallbacks::DrawScene,
			"(scene,camera[,surface]) Draws the scene with the given camera [to the given suurface]");

		// PROFILING
		interpreter->RegisterCallback("BeginTimestamp", &GpuScriptCallbacks::BeginTimestamp,
			"(name,cpu,gpu) Begins a GPU and/or CPU profiling timestamp with the given name");
		interpreter->RegisterCallback("EndTimestamp", &GpuScriptCallbacks::EndTimestamp,
			"(name,cpu,gpu) Ends the GPU and/or CPU profiling timestamp with the given name");
		interpreter->RegisterCallback("GetTimestampData", &GpuScriptCallbacks::GetTimestampData,
			"(name,cpu[,metric]) Returns the given timestamp's time [or its 'drawcalls','polys' or 'statechanges']");

	}

	static void CreateCamera(ScriptInterpreter*);
	static void CreateSpriteCamera(ScriptInterpreter*);
	static void SetCameraPosition(ScriptInterpreter*);
	static void SetCameraTarget(ScriptInterpreter*);
	static void SetCameraUp(ScriptInterpreter*);
	static void SetCameraClipFovOrHeight(ScriptInterpreter*);
	static void GetCameraRay(ScriptInterpreter*);
	static void GetCameraMatrix(ScriptInterpreter*);

	static void SetModelPosition(ScriptInterpreter*);
	static void SetModelRotation(ScriptInterpreter*);
	static void SetModelScale(ScriptInterpreter*);
	static void SetModelMatrix(ScriptInterpreter*);
	static void DrawModel(ScriptInterpreter*);

	static void GetFont(ScriptInterpreter*);
	static void DrawText(ScriptInterpreter*);
	static void SetFontColor(ScriptInterpreter*);

	static void CreateLight(ScriptInterpreter*);         // LIGHTING
	static void SetLightColor(ScriptInterpreter*);       // LIGHTING
	static void SetLightPosition(ScriptInterpreter*);    // LIGHTING
	static void SetLightDirection(ScriptInterpreter*);   // LIGHTING
	static void SetLightAttenuation(ScriptInterpreter*); // LIGHTING
	static void SetLightSpotPower(ScriptInterpreter*);   // LIGHTING

	static void SetMeshPosition(ScriptInterpreter*);
	static void SetMeshRotation(ScriptInterpreter*);
	static void SetMeshScale(ScriptInterpreter*);
	static void SetMeshMatrix(ScriptInterpreter*);
	static void SetMeshTexture(ScriptInterpreter*);
	static void SetMeshNormal(ScriptInterpreter*);       // LIGHTING
	static void SetMeshCubeMap(ScriptInterpreter*);      // LIGHTING
	static void SetMeshColor(ScriptInterpreter*);
	static void SetMeshSpecular(ScriptInterpreter*);     // LIGHTING
	static void SetMeshFactors(ScriptInterpreter*);      // LIGHTING
	static void GetMeshBounds(ScriptInterpreter*);
	static void GetNumMeshes(ScriptInterpreter*);

	static void CreateEffect(ScriptInterpreter*);
	static void SetMeshEffect(ScriptInterpreter*);
	static void SetEffectParam(ScriptInterpreter*);
	static void SetSamplerParam(ScriptInterpreter*);
	static void CreateFloatArray(ScriptInterpreter*);
	static void SetFloatArray(ScriptInterpreter*);
	static void GetFloatArray(ScriptInterpreter*);

	static void CreateSurface(ScriptInterpreter*);
	static void DrawSurface(ScriptInterpreter*);
	static void ClearSurface(ScriptInterpreter*);
	static void GetSurfaceTexture(ScriptInterpreter*);

	static void CreateModel(ScriptInterpreter*);
	static void CreateSpriteModel(ScriptInterpreter*);
	static void DecomposeModel(ScriptInterpreter*);

	static void SetClearColor(ScriptInterpreter*);
	static void SetBlendMode(ScriptInterpreter*);
	static void SetWireframe(ScriptInterpreter*);
	static void SetMultisampling(ScriptInterpreter*);
	static void SetAnisotropy(ScriptInterpreter*);

	static void GetBaseEffect(ScriptInterpreter*);
	static void GetScreenSize(ScriptInterpreter*);
	static void GetBackbufferSize(ScriptInterpreter*);

	static void CreateRect(ScriptInterpreter*);
	static void CreateRectStroke(ScriptInterpreter*);
	static void CreateEllipse(ScriptInterpreter*);
	static void CreateEllipseStroke(ScriptInterpreter*);

	static void CreateIsoSurface(ScriptInterpreter*);
	static void AddIsoSurfaceBall(ScriptInterpreter*);
	static void AddIsoSurfacePlane(ScriptInterpreter*);
	static void ClearIsoSurface(ScriptInterpreter*);
	static void GetIsoSurfaceModel(ScriptInterpreter*);

	static void CreateScene(ScriptInterpreter*);
	static void AddToScene(ScriptInterpreter*);
	static void ClearScene(ScriptInterpreter*);
	static void DrawScene(ScriptInterpreter*);

	static void BeginTimestamp(ScriptInterpreter*);
	static void EndTimestamp(ScriptInterpreter*);
	static void GetTimestampData(ScriptInterpreter*);

	static void CreateInstanceBuffer(ScriptInterpreter*);
	static void UpdateInstanceBuffer(ScriptInterpreter*);

};

} // namespace Ingenuity
