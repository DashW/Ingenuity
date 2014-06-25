#pragma once

#include "ScriptInterpreter.h"
#include "AssetMgr.h"
#include <RealtimeApp.h>
#include <vector>

namespace Ingenuity {

class ScriptCallbacks
{
	ScriptCallbacks();

	static std::wstring projectDirPath;

	static Files::Directory * GetDirectory(Files::Api * files, const char * name);
	static void LoadAsset(AssetMgr * assets, AssetBatch & batch, const char * directory, const char * file, const char * type, const char * name);
	//static ScriptParam VertexBufferToMap(ScriptInterpreter * interpreter, IVertexBuffer * vertexBuffer);
	//static ScriptParam IndexBufferToMap(ScriptInterpreter * interpreter, unsigned * indexBuffer, unsigned numTriangles);
	static ScriptParam VertexBufferToFloats(ScriptInterpreter * interpreter, IVertexBuffer * vertexBuffer);
	static ScriptParam IndexBufferToFloats(ScriptInterpreter * interpreter, unsigned * indexBuffer, unsigned numTriangles);

public:
	static void SetSubDirectory(const wchar_t * subPath) { projectDirPath = subPath ? subPath : L""; }

	static void RegisterWith(ScriptInterpreter * interpreter)
	{
		// GPU
		interpreter->RegisterCallback("DrawSprite", &ScriptCallbacks::DrawSprite,
			"(texture,x,y[,scale=1]) Draws a texture at the given screen location");
		interpreter->RegisterCallback("CreateModel", &ScriptCallbacks::CreateModel,
			"(type,vertices,indices) Creates a model with the given vertex/index data");
		interpreter->RegisterCallback("DrawComplexModel", &ScriptCallbacks::DrawModel,
			"(model,camera[,lights,surface]) Draws a model with the given camera [and list of lights]");
		interpreter->RegisterCallback("GetFont", &ScriptCallbacks::GetFont,
			"(size,family[,style,isPixelSpace]) Loads a font [with style \"regular\", \"bold\" or \"italic\"]");
		interpreter->RegisterCallback("DrawText", &ScriptCallbacks::DrawText,
			"(font,text,x,y[,center=false])");
		interpreter->RegisterCallback("SetFontColor", &ScriptCallbacks::SetFontColor,
			"(font,r,g,b[,a]) Sets the color of a font. All text using this font will be drawn with this color");
		interpreter->RegisterCallback("CreateEffect", &ScriptCallbacks::CreateEffect,
			"(name) Creates an effect with the given shader");
		interpreter->RegisterCallback("SetClearColor", &ScriptCallbacks::SetClearColor,
			"(r,g,b[,a]) Sets the background color to which the screen should be cleared");
		interpreter->RegisterCallback("SetBlendMode", &ScriptCallbacks::SetBlendMode,
			"(mode) Sets the given blend mode to the GPU ('alpha','additive','none')");
		interpreter->RegisterCallback("SetWireframe", &ScriptCallbacks::SetWireframe,
			"(wireframe) Sets whether the GPU should draw all 3D models in wireframe mode");
		interpreter->RegisterCallback("SetMultisampling", &ScriptCallbacks::SetMultisampling,
			"(level) Sets the level of multisampling to be used when drawing to the backbuffer, 0-16");
		interpreter->RegisterCallback("SetAnisotropy", &ScriptCallbacks::SetAnisotropy,
			"(level) Sets the level of anisotropic filtering for the default texture sampler");
		interpreter->RegisterCallback("GetScreenSize", &ScriptCallbacks::GetScreenSize,
			"() returns the width and height of the screen, in pixels");

		// GPU OBJECTS
		interpreter->RegisterCallback("CreateCamera", &ScriptCallbacks::CreateCamera,
			"([orthographic]) Creates a new perspective (default) or orthographic camera");
		interpreter->RegisterCallback("SetCameraPosition", &ScriptCallbacks::SetCameraPosition,
			"(camera,x,y,z) Sets the world position of a camera");
		interpreter->RegisterCallback("SetCameraTarget", &ScriptCallbacks::SetCameraTarget,
			"(camera,x,y,z) Sets the world position of the camera's look target");
		interpreter->RegisterCallback("SetCameraUp", &ScriptCallbacks::SetCameraUp,
			"(camera,x,y,z) Sets the UP vector of the given camera");
		interpreter->RegisterCallback("SetCameraClipFov", &ScriptCallbacks::SetCameraClipFovOrHeight,
			"(camera,nearclip,farclip,fov) Sets the near and far clip panes and fov of a perspective camera");
		interpreter->RegisterCallback("SetCameraClipHeight", &ScriptCallbacks::SetCameraClipFovOrHeight,
			"(camera,nearclip,farclip,height) Sets the near and far clip panes and height of an orthographic camera");
		interpreter->RegisterCallback("CreateLight", &ScriptCallbacks::CreateLight,
			"(type) Creates a new light of type \"directional\", \"point\", or \"spot\"");
		interpreter->RegisterCallback("SetLightColor", &ScriptCallbacks::SetLightColor,
			"(light,r,g,b) Sets the color of a light, components between 0.0 and 1.0");
		interpreter->RegisterCallback("SetLightPosition", &ScriptCallbacks::SetLightPosition,
			"(light,x,y,z) Sets the world position of a light (point and spot only)");
		interpreter->RegisterCallback("SetLightDirection", &ScriptCallbacks::SetLightDirection,
			"(light,x,y,z) Sets the direction of a light (directional and spot only)");
		interpreter->RegisterCallback("SetLightAttenuation", &ScriptCallbacks::SetLightAttenuation,
			"(light,atten) Sets the attenuation value of a light (point and spot only)");

		interpreter->RegisterCallback("CreateGrid", &ScriptCallbacks::CreateGrid,
			"(width,depth,cols,rows[,texX,texY,texW,texH]) Returns vertex and index buffers for a plane");
		interpreter->RegisterCallback("CreateCube", &ScriptCallbacks::CreateCube,
			"() Returns vertex and index buffers for a standard 1x1x1 cube at (0,0,0)");
		interpreter->RegisterCallback("CreateSphere", &ScriptCallbacks::CreateSphere,
			"(texture,tangent) Returns vertex and index buffers for a sphere");

		interpreter->RegisterCallback("SetModelPosition", &ScriptCallbacks::SetModelPosition,
			"(model,x,y,z) Sets the world position of a model");
		interpreter->RegisterCallback("SetModelRotation", &ScriptCallbacks::SetModelRotation,
			"(model,x,y,z) Sets the world rotation of a model");
		interpreter->RegisterCallback("SetModelScale", &ScriptCallbacks::SetModelScale,
			"(model,scale[,scaleY,scaleZ]) Sets the local size of a model, optionally in each axis.");
		interpreter->RegisterCallback("SetMeshPosition", &ScriptCallbacks::SetMeshPosition,
			"(model,meshnum,x,y,z) Sets the position of a mesh relative to its parent model");
		interpreter->RegisterCallback("SetMeshEffect", &ScriptCallbacks::SetMeshEffect,
			"(model,meshnum,effect) Sets an effect to a specific mesh in a complex model");
		interpreter->RegisterCallback("SetMeshTexture", &ScriptCallbacks::SetMeshTexture,
			"(model,meshnum,texture) Sets a texture to a specific mesh in a complex model");
		interpreter->RegisterCallback("SetMeshNormal", &ScriptCallbacks::SetMeshNormal,
			"(model,meshnum,normal) Sets a normal texture to a specific mesh in a complex model");
		interpreter->RegisterCallback("SetMeshCubeMap", &ScriptCallbacks::SetMeshCubeMap,
			"(model,meshnum,cubemap) Sets a cube map to a specific mesh in a complex model");
		interpreter->RegisterCallback("SetMeshColor", &ScriptCallbacks::SetMeshColor,
			"(model,meshnum,r,g,b[,a]) Sets the color of a specific mesh in a complex model");
		interpreter->RegisterCallback("SetMeshSpecular", &ScriptCallbacks::SetMeshSpecular,
			"(model,meshnum,specular) Sets the specular power of a mesh in a complex model");
		interpreter->RegisterCallback("SetMeshFactors", &ScriptCallbacks::SetMeshFactors,
			"(model,meshnum,diffuse,specular) Sets multipliers for the diffuse/specular lighting components");
		interpreter->RegisterCallback("GetMeshBounds", &ScriptCallbacks::GetMeshBounds,
			"(model,meshnum) Returns the x,y,z origin and radius of the mesh's bounding sphere");
		interpreter->RegisterCallback("GetNumMeshes", &ScriptCallbacks::GetNumMeshes,
			"(model) Gets the number of meshes in a complex model");
		interpreter->RegisterCallback("GetHeightmapModel", &ScriptCallbacks::GetHeightmapModel,
			"(heightmap) Creates a model from a heightmap's height information");
		interpreter->RegisterCallback("GetHeightmapHeight", &ScriptCallbacks::GetHeightmapHeight,
			"(heightmap,x,z) Gets the y coordinate for a specific x,z position on a heightmap");
		interpreter->RegisterCallback("SetHeightmapScale", &ScriptCallbacks::SetHeightmapScale,
			"(heightmap,x,y,z) Sets a heightmap's scale in each axis. Use before getting model or height values");
		interpreter->RegisterCallback("GetSVGModel", &ScriptCallbacks::GetSVGModel,
			"(svg[,stroke,anim]) Gets a model from an SVG parser[, optionally an animated stroke]");

		interpreter->RegisterCallback("SetEffectParam", &ScriptCallbacks::SetEffectParam,
			"(effect,paramnum,value) Sets the value of an effect parameter");
		interpreter->RegisterCallback("SetSamplerParam", &ScriptCallbacks::SetSamplerParam,
			"(effect,paramnum,key,value) Sets a sampler parameter for a given effect parameter");
		interpreter->RegisterCallback("CreateFloatArray", &ScriptCallbacks::CreateFloatArray,
			"(length) Creates a shader parameter float array of the given length");
		interpreter->RegisterCallback("SetFloatArray", &ScriptCallbacks::SetFloatArray,
			"(floatArray,index,value[,value...]) Sets the given value[s] to the given index of the array");
		interpreter->RegisterCallback("GetFloatArray", &ScriptCallbacks::GetFloatArray,
			"(floatArray,index[,numValues]) Gets one [or more] values from the given index of the array");

		interpreter->RegisterCallback("CreateSurface", &ScriptCallbacks::CreateSurface,
			"([width,height]) Creates a draw surface of the given dimensions, or fullscreen");
		interpreter->RegisterCallback("ShadeSurface", &ScriptCallbacks::ShadeSurface,
			"(inSurface,effect,outSurface) Shades a draw surface with the given texture shader");
		interpreter->RegisterCallback("DrawSurface", &ScriptCallbacks::DrawSurface,
			"(surface) Draws a surface to the screen");
		interpreter->RegisterCallback("ClearSurface", &ScriptCallbacks::ClearSurface,
			"(surface[,r,g,b,a]) Clears a surface to black [or to the given color values]");

		// Without having a central bank of pointers for reference-counting, this call is DANGEROUS
		// (i.e. if we're not careful, a pointer could be deleted more than once!)
		interpreter->RegisterCallback("GetSurfaceTexture", &ScriptCallbacks::GetSurfaceTexture,
			"(surface) Gets the texture object from a draw surface");

		interpreter->RegisterCallback("CreateIsoSurface", &ScriptCallbacks::CreateIsoSurface,
			"(length) Creates a cube IsoSurface with the given side length");
		interpreter->RegisterCallback("AddIsoSurfaceBall", &ScriptCallbacks::AddIsoSurfaceBall,
			"(iso,x,y,z,r) Adds a metaball to the IsoSurface with the given position and radius");
		interpreter->RegisterCallback("AddIsoSurfacePlane", &ScriptCallbacks::AddIsoSurfacePlane,
			"(iso,x,y,z,nx,ny,nz) Adds a plane to the IsoSurface with the given position and normal");
		interpreter->RegisterCallback("ClearIsoSurface", &ScriptCallbacks::ClearIsoSurface,
			"(iso) Clears all meta-objects from the IsoSurface");
		interpreter->RegisterCallback("GetIsoSurfaceModel", &ScriptCallbacks::GetIsoSurfaceModel,
			"(iso) Updates and gets the ComplexModel for the IsoSurface");

		interpreter->RegisterCallback("CreateInstanceBuffer", &ScriptCallbacks::CreateInstanceBuffer,
			"(type,size) Creates an instance buffer of the given type with the given capacity");
		interpreter->RegisterCallback("UpdateInstanceBuffer", &ScriptCallbacks::UpdateInstanceBuffer,
			"(ibuf,floats,size) Updates an instance buffer with the given FloatArray");

		// ASSETS
		interpreter->RegisterCallback("LoadAssets", &ScriptCallbacks::LoadAssets,
			"(directory,path,type,name[,...]) Starts loading the given assets, returns a ticket (see GetProgress, Unload)");
		interpreter->RegisterCallback("GetLoadProgress", &ScriptCallbacks::GetLoadProgress,
			"(ticket) Returns loading progress of the ticket's assets (between 0 and 1)");
		interpreter->RegisterCallback("IsLoaded", &ScriptCallbacks::IsLoaded,
			"(ticket) Returns whether all the assets of a ticket have finished loading");
		interpreter->RegisterCallback("UnloadAssets", &ScriptCallbacks::UnloadAssets,
			"(ticket) Unloads assets associated with the given ticket to free up memory");
		interpreter->RegisterCallback("GetAsset", &ScriptCallbacks::GetAsset,
			"(path) Gets a loaded asset by its path, returns null if not loaded or load failed");

		// CONSOLE
		interpreter->RegisterCallback("ClearConsole", &ScriptCallbacks::ClearConsole,
			"() Clears the console output lines");
		interpreter->RegisterCallback("Help", &ScriptCallbacks::Help,
			"() Displays help information for all Ingenuity functions");
		interpreter->RegisterCallback("Require", &ScriptCallbacks::Require,
			"(directory,path) Loads and runs a dependent script file");

		// INPUT
		interpreter->RegisterCallback("GetMousePosition", &ScriptCallbacks::GetMousePosition,
			"() Returns the absolute X and Y positions of the mouse within the window");
		interpreter->RegisterCallback("GetMouseDelta", &ScriptCallbacks::GetMouseDelta,
			"() Returns the relative X and Y movements of the mouse within the window this frame");
		interpreter->RegisterCallback("GetMouseLeft", &ScriptCallbacks::GetMouseLeft,
			"() Returns current press state, pressed this frame, and released this frame");
		interpreter->RegisterCallback("GetMouseRight", &ScriptCallbacks::GetMouseRight,
			"() Returns current press state, pressed this frame, and released this frame");
		interpreter->RegisterCallback("GetKeyState", &ScriptCallbacks::GetKeyState,
			"(key) Returns pressed, down this frame, and up this frame booleans for the given key");
		interpreter->RegisterCallback("GetTypedText", &ScriptCallbacks::GetTypedText,
			"() Returns any text that has been input by the user, keyboard or otherwise, in this frame");

		// 2D DRAWING
		interpreter->RegisterCallback("CreateRect", &ScriptCallbacks::CreateRect,
			"(x,y,w,h) Creates a rectangle model of the specified dimensions");
		interpreter->RegisterCallback("CreateRectStroke", &ScriptCallbacks::CreateRectStroke,
			"(x,y,w,h,weight) Creates a rectangle stroke model of the specified dimensions & thickness");
		interpreter->RegisterCallback("CreateEllipse", &ScriptCallbacks::CreateEllipse,
			"(x,y,rx,ry) Creates an ellipse model of the specified dimensions");
		interpreter->RegisterCallback("CreateEllipseStroke", &ScriptCallbacks::CreateEllipseStroke,
			"(x,y,rx,ry,weight) Creates an ellipse stroke model of the specified dimensions & thickness");

		// SCENES
		interpreter->RegisterCallback("CreateScene", &ScriptCallbacks::CreateScene,
			"([type]) Creates a rendering scene. Valid types are \"linear\"(default) or \"instanced\"");
		interpreter->RegisterCallback("AddToScene", &ScriptCallbacks::AddToScene,
			"(scene,object) Add a mesh, texture or light to the scene");
		interpreter->RegisterCallback("ClearScene", &ScriptCallbacks::ClearScene,
			"(scene) Removes all objects from the scene");
		interpreter->RegisterCallback("DrawScene", &ScriptCallbacks::DrawScene,
			"(scene,camera[,surface]) Draws the scene with the given camera [to the given suurface]");

		// IMAGES
		interpreter->RegisterCallback("GetImageSize", &ScriptCallbacks::GetImageSize,
			"(image) Returns the pixel dimensions of the given image, in the form x,y");
		interpreter->RegisterCallback("GetImagePixel", &ScriptCallbacks::GetImagePixel,
			"(image,u,v) Returns the color of the given pixel in the image, in the form r,g,b,a");

		// FILES
		interpreter->RegisterCallback("EnumerateDirectory", &ScriptCallbacks::EnumerateDirectory,
			"(directory) Instructs a directory to enumerate its contents, to be gotten by GetDirectoryFiles");
		interpreter->RegisterCallback("GetDirectoryFiles", &ScriptCallbacks::GetDirectoryFiles,
			"(directory) Returns a table containing all filenames in the directory, or None if not yet enumerated");
		interpreter->RegisterCallback("LoadFile", &ScriptCallbacks::LoadFile,
			"(directory,path,callback) Invokes the callback with the raw contents of the given file");
		interpreter->RegisterCallback("PickFile", &ScriptCallbacks::PickFile,
			"(directory,extension,callback) Creates a file picker, then invokes the callback with the file path");

		// AUDIO
		interpreter->RegisterCallback("PlaySound", &ScriptCallbacks::PlaySound,
			"(sound[,seek,loop]) Plays the given sound file [from the seek time, on repeat if 'loop' is true]");
		interpreter->RegisterCallback("PauseSound", &ScriptCallbacks::PauseSound,
			"(sound) Pauses the given sound file");
		interpreter->RegisterCallback("GetAmplitude", &ScriptCallbacks::GetAmplitude,
			"([sound]) Returns the global amplitude [or, if provided, the amplitude of the given sound]");
		interpreter->RegisterCallback("GetSoundDuration", &ScriptCallbacks::GetSoundDuration,
			"(sound) Returns the duration, in seconds, of the given sound");
		interpreter->RegisterCallback("GetSoundProgress", &ScriptCallbacks::GetSoundProgress,
			"(sound) Returns the progress, in seconds, of playback through the given sound");

		// PROFILING
		interpreter->RegisterCallback("BeginTimestamp", &ScriptCallbacks::BeginTimestamp,
			"(name,cpu,gpu) Begins a GPU and/or CPU profiling timestamp with the given name");
		interpreter->RegisterCallback("EndTimestamp", &ScriptCallbacks::EndTimestamp,
			"(name,cpu,gpu) Ends the GPU and/or CPU profiling timestamp with the given name");
		interpreter->RegisterCallback("GetTimestampData", &ScriptCallbacks::GetTimestampData,
			"(name,cpu[,metric]) Returns the given timestamp's time [or its 'drawcalls','polys' or 'statechanges']");
	}

	static void ClearConsole(ScriptInterpreter*);
	static void Help(ScriptInterpreter*);
	static void Require(ScriptInterpreter*);

	static void DrawSprite(ScriptInterpreter*);

	static void CreateCamera(ScriptInterpreter*);
	static void SetCameraPosition(ScriptInterpreter*);
	static void SetCameraTarget(ScriptInterpreter*);
	static void SetCameraUp(ScriptInterpreter*);
	static void SetCameraClipFovOrHeight(ScriptInterpreter*);

	static void CreateGrid(ScriptInterpreter*);
	static void CreateCube(ScriptInterpreter*);
	static void CreateSphere(ScriptInterpreter*);

	static void SetModelPosition(ScriptInterpreter*);
	static void SetModelRotation(ScriptInterpreter*);
	static void SetModelScale(ScriptInterpreter*);
	static void DrawModel(ScriptInterpreter*);

	static void GetFont(ScriptInterpreter*);
	static void DrawText(ScriptInterpreter*);
	static void SetFontColor(ScriptInterpreter*);

	static void CreateLight(ScriptInterpreter*);
	static void SetLightColor(ScriptInterpreter*);
	static void SetLightPosition(ScriptInterpreter*);
	static void SetLightDirection(ScriptInterpreter*);
	static void SetLightAttenuation(ScriptInterpreter*);

	static void SetMeshPosition(ScriptInterpreter*);
	static void SetMeshTexture(ScriptInterpreter*);
	static void SetMeshNormal(ScriptInterpreter*);
	static void SetMeshCubeMap(ScriptInterpreter*);
	static void SetMeshColor(ScriptInterpreter*);
	static void SetMeshSpecular(ScriptInterpreter*);
	static void SetMeshFactors(ScriptInterpreter*);
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
	static void ShadeSurface(ScriptInterpreter*);
	static void DrawSurface(ScriptInterpreter*);
	static void ClearSurface(ScriptInterpreter*);
	static void GetSurfaceTexture(ScriptInterpreter*);

	static void LoadAssets(ScriptInterpreter*);
	static void UnloadAssets(ScriptInterpreter*);
	static void GetLoadProgress(ScriptInterpreter*);
	static void IsLoaded(ScriptInterpreter*);
	static void GetAsset(ScriptInterpreter*);
	static void EnumerateDirectory(ScriptInterpreter*);
	static void GetDirectoryFiles(ScriptInterpreter*);

	static void CreateModel(ScriptInterpreter*);
	static void SetHeightmapScale(ScriptInterpreter*);
	static void GetHeightmapHeight(ScriptInterpreter*);
	static void GetHeightmapModel(ScriptInterpreter*);
	static void GetSVGModel(ScriptInterpreter*);

	static void SetClearColor(ScriptInterpreter*);
	static void SetBlendMode(ScriptInterpreter*);
	static void SetWireframe(ScriptInterpreter*);
	static void SetMultisampling(ScriptInterpreter*);
	static void SetAnisotropy(ScriptInterpreter*);

	static void GetScreenSize(ScriptInterpreter*);
	static void GetMousePosition(ScriptInterpreter*);
	static void GetMouseDelta(ScriptInterpreter*);
	static void GetMouseLeft(ScriptInterpreter*);
	static void GetMouseRight(ScriptInterpreter*);
	static void GetKeyState(ScriptInterpreter*);
	static void GetTypedText(ScriptInterpreter*);

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

	static void GetImageSize(ScriptInterpreter*);
	static void GetImagePixel(ScriptInterpreter*);

	static void LoadFile(ScriptInterpreter*);
	static void PickFile(ScriptInterpreter*);

	static void PlaySound(ScriptInterpreter*);
	static void PauseSound(ScriptInterpreter*);
	static void GetAmplitude(ScriptInterpreter*);
	static void GetSoundDuration(ScriptInterpreter*);
	static void GetSoundProgress(ScriptInterpreter*);

	static void BeginTimestamp(ScriptInterpreter*);
	static void EndTimestamp(ScriptInterpreter*);
	static void GetTimestampData(ScriptInterpreter*);

	static void CreateInstanceBuffer(ScriptInterpreter*);
	static void UpdateInstanceBuffer(ScriptInterpreter*);
};

} // namespace Ingenuity

#define POP_PARAM(NUM,NAME,TYPE) ScriptParam NAME = interpreter->PopParam();\
	if(NAME.type != ScriptParam::TYPE) { \
	interpreter->ThrowError("parameter " #NUM " (" #NAME ") is not of type " #TYPE); \
	return; \
	}

#define POP_NUMPARAM(NUM,NAME) ScriptParam NAME = interpreter->PopParam();\
	if(!NAME.IsNumber() && NAME.type != ScriptParam::BOOL) {\
	interpreter->ThrowError("parameter " #NUM " (" #NAME ") is not a number");\
	return; }

#define POP_PTRPARAM(NUM,NAME,PTRTYPE) ScriptParam NAME = interpreter->PopParam();\
	if(!NAME.CheckPointer(ScriptPtrType::PTRTYPE)) {\
	interpreter->ThrowError("parameter " #NUM " (" #NAME ") is not a pointer of type " #PTRTYPE);\
	return; }