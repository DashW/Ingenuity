#pragma once

#include "ScriptInterpreter.h"
#include "AssetMgr.h"
#include <RealtimeApp.h>
#include <vector>

namespace Ingenuity {

struct LocalMesh;

class ScriptCallbacks
{
	ScriptCallbacks();

	enum PtrType
	{
		TypeGpuComplexModel,
		TypeGpuEffect,
		TypeGpuTexture,
		TypeGpuCubeMap,
		TypeGpuVolumeTexture,
		TypeGpuCamera,
		TypeGpuFont,
		TypeGpuLight,
		TypeGpuDrawSurface,
		TypeGpuScene,
		TypeGpuShader,
		TypeGpuInstanceBuffer,
		
		TypeFloatArray,
		TypeHeightParser,
		TypeImageBuffer,
		TypeAudioItem,
		TypeIsoSurface,
		TypeSVGParser,
		TypePhysicsWorld,
		TypePhysicsObject,
		TypePhysicsMaterial,
		TypePhysicsRagdoll,
		TypePhysicsSpring,
		TypeLeapHelper,
		
		TypeCount
	};
	static unsigned typeHandles[TypeCount];
	static std::wstring projectDirPath;

	static inline bool CheckPtrType(ScriptParam & param, PtrType type) { return param.CheckPointer(typeHandles[type]); }
	static Files::Directory * GetDirectory(Files::Api * files, const char * name);
	static void LoadAsset(AssetMgr * assets, AssetBatch & batch, const char * directory, const char * file, const char * type, const char * name);
	//static ScriptParam VertexBufferToMap(ScriptInterpreter * interpreter, IVertexBuffer * vertexBuffer);
	//static ScriptParam IndexBufferToMap(ScriptInterpreter * interpreter, unsigned * indexBuffer, unsigned numTriangles);
	static ScriptParam VertexBufferToFloats(ScriptInterpreter * interpreter, IVertexBuffer * vertexBuffer);
	static ScriptParam IndexBufferToFloats(ScriptInterpreter * interpreter, unsigned * indexBuffer, unsigned numTriangles);
	static LocalMesh * FloatsToLocalMesh(ScriptInterpreter * interpreter, ScriptParam type, ScriptParam vtx, ScriptParam idx);

public:
	static void SetSubDirectory(const wchar_t * subPath) { projectDirPath = subPath ? subPath : L""; }

	static void RegisterWith(ScriptInterpreter * interpreter)
	{
		// Types
		typeHandles[TypeGpuComplexModel] = interpreter->RegisterPointerType();
		typeHandles[TypeGpuEffect] = interpreter->RegisterPointerType();
		typeHandles[TypeGpuTexture] = interpreter->RegisterPointerType();
		typeHandles[TypeGpuCubeMap] = interpreter->RegisterPointerType();
		typeHandles[TypeGpuVolumeTexture] = interpreter->RegisterPointerType();
		typeHandles[TypeGpuCamera] = interpreter->RegisterPointerType();
		typeHandles[TypeGpuFont] = interpreter->RegisterPointerType();
		typeHandles[TypeGpuLight] = interpreter->RegisterPointerType();
		typeHandles[TypeGpuDrawSurface] = interpreter->RegisterPointerType();
		typeHandles[TypeGpuScene] = interpreter->RegisterPointerType();
		typeHandles[TypeGpuShader] = interpreter->RegisterPointerType();
		typeHandles[TypeGpuInstanceBuffer] = interpreter->RegisterPointerType();
		
		typeHandles[TypeFloatArray] = interpreter->RegisterPointerType();
		typeHandles[TypeHeightParser] = interpreter->RegisterPointerType();
		typeHandles[TypeImageBuffer] = interpreter->RegisterPointerType();
		typeHandles[TypeAudioItem] = interpreter->RegisterPointerType();
		typeHandles[TypeIsoSurface] = interpreter->RegisterPointerType();
		typeHandles[TypeSVGParser] = interpreter->RegisterPointerType();

		typeHandles[TypePhysicsWorld] = interpreter->RegisterPointerType();
		typeHandles[TypePhysicsObject] = interpreter->RegisterPointerType();
		typeHandles[TypePhysicsMaterial] = interpreter->RegisterPointerType();
		typeHandles[TypePhysicsRagdoll] = interpreter->RegisterPointerType();
		typeHandles[TypePhysicsSpring] = interpreter->RegisterPointerType();

		typeHandles[TypeLeapHelper] = interpreter->RegisterPointerType();

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
		interpreter->RegisterCallback("GetCameraRay", &ScriptCallbacks::GetCameraRay,
			"(camera,x,y[,surface]) Returns the vector4 ray from the camera for the given point [on the surface]");

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
			"([tex]) Returns vertex and index buffers [with texture coordinates] for a 2x2x2 cube at (0,0,0)");
		interpreter->RegisterCallback("CreateSphere", &ScriptCallbacks::CreateSphere,
			"(texture,tangent) Returns vertex and index buffers for a sphere");
		interpreter->RegisterCallback("CreateCylinder", &ScriptCallbacks::CreateCylinder,
			"(length) Returns vertex and index buffers for a cylinder");
		interpreter->RegisterCallback("CreateCapsule", &ScriptCallbacks::CreateCapsule,
			"(length) Returns vertex and index buffers for a capsule (round-ended cylinder)");

		interpreter->RegisterCallback("SetModelPosition", &ScriptCallbacks::SetModelPosition,
			"(model,x,y,z) Sets the world position of a model");
		interpreter->RegisterCallback("SetModelRotation", &ScriptCallbacks::SetModelRotation,
			"(model,x,y,z) Sets the world rotation of a model");
		interpreter->RegisterCallback("SetModelScale", &ScriptCallbacks::SetModelScale,
			"(model,scale[,scaleY,scaleZ]) Sets the local size of a model, optionally in each axis.");
		interpreter->RegisterCallback("SetMeshPosition", &ScriptCallbacks::SetMeshPosition,
			"(model,meshnum,x,y,z) Sets the position of a mesh relative to its parent model");
		interpreter->RegisterCallback("SetMeshRotation", &ScriptCallbacks::SetMeshRotation,
			"(model,meshnum,x,y,z) Sets the rotation of a mesh relative to its parent model");
		interpreter->RegisterCallback("SetMeshMatrix", &ScriptCallbacks::SetMeshMatrix,
			"(model,meshnum,matrix) Sets the transform matrix to the given mesh of the model");
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
		interpreter->RegisterCallback("GetWavefrontMesh", &ScriptCallbacks::GetWavefrontMesh,
			"(name,index) Gets the indexed mesh of the loaded wavefront model with the given asset name");

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

		// PHYSICS
		interpreter->RegisterCallback("CreatePhysicsWorld", &ScriptCallbacks::CreatePhysicsWorld,
			"() Creates an empty physics world");
		interpreter->RegisterCallback("CreatePhysicsMaterial", &ScriptCallbacks::CreatePhysicsMaterial,
			"(elasticity,sfriction,kfriction,softness) Creates a physics material");
		interpreter->RegisterCallback("CreatePhysicsAnchor", &ScriptCallbacks::CreatePhysicsAnchor,
			"() Creates a static, zero-sized physics anchor for joint attachment");
		interpreter->RegisterCallback("CreatePhysicsCuboid", &ScriptCallbacks::CreatePhysicsCuboid,
			"(w,h,d,kinematic) Creates a physics cuboid with the given dimenstions and control method");
		interpreter->RegisterCallback("CreatePhysicsSphere", &ScriptCallbacks::CreatePhysicsSphere,
			"(r,kinematic) Creates a physics sphere with the given radius and control method");
		interpreter->RegisterCallback("CreatePhysicsCapsule", &ScriptCallbacks::CreatePhysicsCapsule,
			"(r,l,kinematic) Creates a physics capsule with the given radius/length and control method");
		interpreter->RegisterCallback("CreatePhysicsMesh", &ScriptCallbacks::CreatePhysicsMesh,
			"(type,vtx,idx) Creates a physics mesh from the given vertex type, vertex and index buffers");
		interpreter->RegisterCallback("CreatePhysicsHeightmap", &ScriptCallbacks::CreatePhysicsHeightmap,
			"(heightmap) Creates a physics heightmap from the given heightmap parser");
		interpreter->RegisterCallback("CreatePhysicsSpring", &ScriptCallbacks::CreatePhysicsSpring,
			"(obj1,obj2,attach1,attach2) Creates a physics spring between two objects");
		interpreter->RegisterCallback("AddToPhysicsWorld", &ScriptCallbacks::AddToPhysicsWorld,
			"(world,object[,static]) Adds the given physics object to the given physics world");
		interpreter->RegisterCallback("RemoveFromPhysicsWorld", &ScriptCallbacks::RemoveFromPhysicsWorld,
			"(world,object) Removes the given physics object from the given physics world");
		interpreter->RegisterCallback("UpdatePhysicsWorld", &ScriptCallbacks::UpdatePhysicsWorld,
			"(world,delta) Updates the given physics world by the given time delta");
		interpreter->RegisterCallback("SetPhysicsPosition", &ScriptCallbacks::SetPhysicsPosition,
			"(object,x,y,z[,local]) Sets the [local] position of the physics object");
		interpreter->RegisterCallback("SetPhysicsRotation", &ScriptCallbacks::SetPhysicsRotation,
			"(object,x,y,z[,local]) Sets the [local] rotation of the physics object");
		interpreter->RegisterCallback("SetPhysicsScale", &ScriptCallbacks::SetPhysicsScale,
			"(object,x[,y,z]) Sets the scale of the physics object");
		interpreter->RegisterCallback("SetPhysicsMass", &ScriptCallbacks::SetPhysicsMass,
			"(object,mass) Sets the mass of the given physics object");
		interpreter->RegisterCallback("SetPhysicsMaterial", &ScriptCallbacks::SetPhysicsMaterial,
			"(object,material) Sets the given physics material to the given physics object");
;		interpreter->RegisterCallback("GetPhysicsPosition", &ScriptCallbacks::GetPhysicsPosition,
			"(object) Gets the x,y,z position of the physics object");
		//interpreter->RegisterCallback("GetPhysicsRotation", &ScriptCallbacks::GetPhysicsRotation,
		//	"(object) Gets the x,y,z rotation of the physics object");
		interpreter->RegisterCallback("GetPhysicsMatrix", &ScriptCallbacks::GetPhysicsMatrix,
			"(object) Returns the transformation matrix of the given physics object");
		interpreter->RegisterCallback("SetPhysicsMatrix", &ScriptCallbacks::SetPhysicsMatrix,
			"(object,matrix) Sets the transformation matrix of the physics object if possible");
		interpreter->RegisterCallback("SetPhysicsSpringProperty", &ScriptCallbacks::SetPhysicsSpringProperty,
			"(spring,name,value) Applies a property change to a spring, e.g. 'stiffness', 'damping', 'length'");

		interpreter->RegisterCallback("CreatePhysicsRagdoll", &ScriptCallbacks::CreatePhysicsRagdoll,
			"(world) Creates a physical ragdoll in the given physics world");
		interpreter->RegisterCallback("AddPhysicsRagdollBone", &ScriptCallbacks::AddPhysicsRagdollBone,
			"(ragdoll,object,index) Adds a physics object to the ragdoll as a bone");
		interpreter->RegisterCallback("GetPhysicsRagdollBone", &ScriptCallbacks::GetPhysicsRagdollBone,
			"(ragdoll,index) Gets a specific bone physics object from the physics ragdoll");
		interpreter->RegisterCallback("FinalizePhysicsRagdoll", &ScriptCallbacks::FinalizePhysicsRagdoll,
			"(ragdoll) Performs final measurements and transformations of a physics ragdoll");
		interpreter->RegisterCallback("PickPhysicsObject", &ScriptCallbacks::PickPhysicsObject,
			"(world,origin,ray) Returns the first object, position, normal and distance intersected by the ray vector4s");
		interpreter->RegisterCallback("GetPhysicsDebugModel", &ScriptCallbacks::GetPhysicsDebugModel,
			"(object) Returns a debug model for the given physics object");

#ifdef USE_LEAPMOTION_HELPER

		interpreter->RegisterCallback("CreateLeapHelper", &ScriptCallbacks::CreateLeapHelper,
			"() Returns a new instance of a Leap Motion Helper");
		interpreter->RegisterCallback("GetLeapFrameTime", &ScriptCallbacks::GetLeapFrameTime,
			"(helper) Returns the frame time, in milliseconds, for the current Leap frame");
		interpreter->RegisterCallback("GetLeapNumBones", &ScriptCallbacks::GetLeapNumBones,
			"(helper) Returns the number of bones the Leap Motion is capable of tracking");
		interpreter->RegisterCallback("GetLeapBoneDetails", &ScriptCallbacks::GetLeapBoneDetails,
			"(helper,index) Returns the visibility, length and radius of the given Leap Motion bone");
		interpreter->RegisterCallback("GetLeapBonePosition", &ScriptCallbacks::GetLeapBonePosition,
			"(helper,index) Returns the position of the given Leap Motion bone");
		interpreter->RegisterCallback("SetLeapPosition", &ScriptCallbacks::SetLeapPosition,
			"(helper,x,y,z) Offsets the positions of all bones in the Leap world");
		interpreter->RegisterCallback("SetLeapScale", &ScriptCallbacks::SetLeapScale,
			"(helper,scale) Sets the scale of the Leap world, including bone details and matrices");
		interpreter->RegisterCallback("GetLeapBoneMatrix", &ScriptCallbacks::GetLeapBoneMatrix,
			"(helper,index) Returns the transformation matrix of the indexed Leap Motion bone");

#endif

		interpreter->RegisterOperator(typeHandles[TypePhysicsSpring], 
			ScriptInterpreter::IndexSet, ScriptCallbacks::SetPhysicsSpringProperty);
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
	static void GetCameraRay(ScriptInterpreter*);

	static void CreateGrid(ScriptInterpreter*);
	static void CreateCube(ScriptInterpreter*);
	static void CreateSphere(ScriptInterpreter*);
	static void CreateCylinder(ScriptInterpreter*);
	static void CreateCapsule(ScriptInterpreter*);

	static void SetModelPosition(ScriptInterpreter*);
	static void SetModelRotation(ScriptInterpreter*);
	static void SetModelScale(ScriptInterpreter*);
	static void DrawModel(ScriptInterpreter*);

	static void GetFont(ScriptInterpreter*);
	static void DrawText(ScriptInterpreter*);
	static void SetFontColor(ScriptInterpreter*);

	static void CreateLight(ScriptInterpreter*);         // LIGHTING
	static void SetLightColor(ScriptInterpreter*);       // LIGHTING
	static void SetLightPosition(ScriptInterpreter*);    // LIGHTING
	static void SetLightDirection(ScriptInterpreter*);   // LIGHTING
	static void SetLightAttenuation(ScriptInterpreter*); // LIGHTING

	static void SetMeshPosition(ScriptInterpreter*);
	static void SetMeshRotation(ScriptInterpreter*);
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
	static void GetWavefrontMesh(ScriptInterpreter*);

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

	static void CreatePhysicsWorld(ScriptInterpreter*);
	static void UpdatePhysicsWorld(ScriptInterpreter*);
	static void CreatePhysicsMaterial(ScriptInterpreter*);
	static void CreatePhysicsAnchor(ScriptInterpreter*);
	static void CreatePhysicsCuboid(ScriptInterpreter*);
	static void CreatePhysicsSphere(ScriptInterpreter*);
	static void CreatePhysicsCapsule(ScriptInterpreter*);
	static void CreatePhysicsMesh(ScriptInterpreter*);
	static void CreatePhysicsHeightmap(ScriptInterpreter*);
	static void CreatePhysicsSpring(ScriptInterpreter*);
	static void AddToPhysicsWorld(ScriptInterpreter*);
	static void RemoveFromPhysicsWorld(ScriptInterpreter*);
	static void SetPhysicsPosition(ScriptInterpreter*);
	static void SetPhysicsRotation(ScriptInterpreter*);
	static void SetPhysicsScale(ScriptInterpreter*);
	static void SetPhysicsMass(ScriptInterpreter*);
	static void SetPhysicsMaterial(ScriptInterpreter*);
	static void GetPhysicsPosition(ScriptInterpreter*);
	//static void GetPhysicsRotation(ScriptInterpreter*);
	static void GetPhysicsMatrix(ScriptInterpreter*);
	static void SetPhysicsMatrix(ScriptInterpreter*);
	static void SetPhysicsSpringProperty(ScriptInterpreter*);

	static void CreatePhysicsRagdoll(ScriptInterpreter*);
	static void AddPhysicsRagdollBone(ScriptInterpreter*);
	static void GetPhysicsRagdollBone(ScriptInterpreter*);
	static void FinalizePhysicsRagdoll(ScriptInterpreter*);
	static void PickPhysicsObject(ScriptInterpreter*);
	static void GetPhysicsDebugModel(ScriptInterpreter*);

#ifdef USE_LEAPMOTION_HELPER

	static void CreateLeapHelper(ScriptInterpreter*);
	static void GetLeapFrameTime(ScriptInterpreter*);
	static void GetLeapNumBones(ScriptInterpreter*);
	static void GetLeapBoneDetails(ScriptInterpreter*);
	static void GetLeapBonePosition(ScriptInterpreter*);
	static void SetLeapPosition(ScriptInterpreter*);
	static void SetLeapScale(ScriptInterpreter*);
	static void GetLeapBoneMatrix(ScriptInterpreter*);

#endif
};

} // namespace Ingenuity

// Generic Parameter
#define POP_PARAM(NUM,NAME,TYPE) ScriptParam NAME = interpreter->PopParam();\
	if(NAME.type != ScriptParam::TYPE) { \
	interpreter->ThrowError("parameter " #NUM " (" #NAME ") is not of type " #TYPE); \
	return; \
	}

// Numeric Parameter
#define POP_NUMPARAM(NUM,NAME) ScriptParam NAME = interpreter->PopParam();\
	if(!NAME.IsNumber() && NAME.type != ScriptParam::BOOL) {\
	interpreter->ThrowError("parameter " #NUM " (" #NAME ") is not a number");\
	return; }

// Pointer Parameter
#define POP_PTRPARAM(NUM,NAME,PTRTYPE) ScriptParam NAME = interpreter->PopParam();\
	if(!NAME.CheckPointer(typeHandles[PTRTYPE])) {\
	interpreter->ThrowError("parameter " #NUM " (" #NAME ") is not a pointer of type " #PTRTYPE);\
	return; }

// Special Pointer Parameter
#define POP_SPTRPARAM(NUM,NAME,PTRTYPE) ScriptParam NAME = interpreter->PopParam();\
	if(!NAME.CheckPointer(interpreter->GetSpecialPtrType(ScriptInterpreter::PTRTYPE))) {\
	interpreter->ThrowError("parameter " #NUM " (" #NAME ") is not a pointer of type " #PTRTYPE);\
	return; }
