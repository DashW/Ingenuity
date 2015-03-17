#pragma once

#include "ScriptInterpreter.h"
#include "ScriptTypes.h"
#include "AssetMgr.h"
#include <RealtimeApp.h>
#include <vector>

namespace Ingenuity {

struct LocalMesh;

class ScriptCallbacks
{
	ScriptCallbacks();

	static Files::Directory * projectDirectory;

	static Files::Directory * GetDirectory(Files::Api * files, const char * name);
	static void LoadAsset(AssetMgr * assets, AssetBatch & batch, const char * directory, const char * file, const char * type, const char * name);
	
	//static ScriptParam VertexBufferToMap(ScriptInterpreter * interpreter, IVertexBuffer * vertexBuffer);
	//static ScriptParam IndexBufferToMap(ScriptInterpreter * interpreter, unsigned * indexBuffer, unsigned numTriangles);
	static ScriptParam VertexBufferToFloats(ScriptInterpreter * interpreter, IVertexBuffer * vertexBuffer);
	static ScriptParam IndexBufferToFloats(ScriptInterpreter * interpreter, unsigned * indexBuffer, unsigned numTriangles);
	static LocalMesh * FloatsToLocalMesh(ScriptInterpreter * interpreter, ScriptParam type, ScriptParam vtx, ScriptParam idx);

public:
	static void SetSubDirectory(Files::Directory * directory) { projectDirectory = directory; }

	static void RegisterWith(ScriptInterpreter * interpreter)
	{
		interpreter->RegisterCallback("CreateWindow", &ScriptCallbacks::CreateWindow,
			"([width,height]) Creates a window [of the given width and height]");
		interpreter->RegisterCallback("GetMainWindow", &ScriptCallbacks::GetMainWindow,
			"() Gets a reference to the application's main window");
		interpreter->RegisterCallback("GetWindowSurface", &ScriptCallbacks::GetWindowSurface,
			"(window) Gets a reference to the GPU draw surface of the given window");
		interpreter->RegisterCallback("SetWindowProps", &ScriptCallbacks::SetWindowProps,
			"(window,width,height,undecorated,resizeable) Sets generic properties for the given window");

		// UTILS
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

		interpreter->RegisterCallback("CreateFloatArray", &ScriptCallbacks::CreateFloatArray,
			"(length) Creates a shader parameter float array of the given length");
		interpreter->RegisterCallback("SetFloatArray", &ScriptCallbacks::SetFloatArray,
			"(floatArray,index,value[,value...]) Sets the given value[s] to the given index of the array");
		interpreter->RegisterCallback("GetFloatArray", &ScriptCallbacks::GetFloatArray,
			"(floatArray,index[,numValues]) Gets one [or more] values from the given index of the array");

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

		interpreter->RegisterOperator(ScriptTypes::GetHandle(TypePhysicsSpring), 
			ScriptInterpreter::IndexSet, ScriptCallbacks::SetPhysicsSpringProperty);

		interpreter->RegisterOperator(ScriptTypes::GetHandle(TypeFloatArray),
			ScriptInterpreter::IndexSet, ScriptCallbacks::SetFloatArray);
		interpreter->RegisterOperator(ScriptTypes::GetHandle(TypeFloatArray),
			ScriptInterpreter::IndexGet, ScriptCallbacks::GetFloatArray);
		interpreter->RegisterOperator(ScriptTypes::GetHandle(TypeFloatArray),
			ScriptInterpreter::Length, ScriptCallbacks::GetFloatArrayLength);
	}

	static void ClearConsole(ScriptInterpreter*);
	static void Help(ScriptInterpreter*);
	static void Require(ScriptInterpreter*);

	static void CreateWindow(ScriptInterpreter*);
	static void GetMainWindow(ScriptInterpreter*);
	static void GetWindowStatus(ScriptInterpreter*);
	static void GetWindowSurface(ScriptInterpreter*);
	static void SetWindowProps(ScriptInterpreter*);

	static void CreateFloatArray(ScriptInterpreter*);
	static void SetFloatArray(ScriptInterpreter*);
	static void GetFloatArray(ScriptInterpreter*);
	static void GetFloatArrayLength(ScriptInterpreter*);

	static void CreateGrid(ScriptInterpreter*);
	static void CreateCube(ScriptInterpreter*);
	static void CreateSphere(ScriptInterpreter*);
	static void CreateCylinder(ScriptInterpreter*);
	static void CreateCapsule(ScriptInterpreter*);

	static void LoadAssets(ScriptInterpreter*);
	static void UnloadAssets(ScriptInterpreter*);
	static void GetLoadProgress(ScriptInterpreter*);
	static void IsLoaded(ScriptInterpreter*);
	static void GetAsset(ScriptInterpreter*);
	static void EnumerateDirectory(ScriptInterpreter*);
	static void GetDirectoryFiles(ScriptInterpreter*);

	static void SetHeightmapScale(ScriptInterpreter*);
	static void GetHeightmapHeight(ScriptInterpreter*);
	static void GetHeightmapModel(ScriptInterpreter*);
	static void GetSVGModel(ScriptInterpreter*);
	static void GetWavefrontMesh(ScriptInterpreter*);

	static void GetMousePosition(ScriptInterpreter*);
	static void GetMouseDelta(ScriptInterpreter*);
	static void GetMouseLeft(ScriptInterpreter*);
	static void GetMouseRight(ScriptInterpreter*);
	static void GetKeyState(ScriptInterpreter*);
	static void GetTypedText(ScriptInterpreter*);

	static void GetImageSize(ScriptInterpreter*);
	static void GetImagePixel(ScriptInterpreter*);

	static void LoadFile(ScriptInterpreter*);
	static void PickFile(ScriptInterpreter*);

	static void PlaySound(ScriptInterpreter*);
	static void PauseSound(ScriptInterpreter*);
	static void GetAmplitude(ScriptInterpreter*);
	static void GetSoundDuration(ScriptInterpreter*);
	static void GetSoundProgress(ScriptInterpreter*);

	static void CreatePhysicsWorld(ScriptInterpreter*);
	static void CreatePhysicsMaterial(ScriptInterpreter*);
	static void CreatePhysicsAnchor(ScriptInterpreter*);
	static void CreatePhysicsCuboid(ScriptInterpreter*);
	static void CreatePhysicsSphere(ScriptInterpreter*);
	static void CreatePhysicsCapsule(ScriptInterpreter*);
	static void CreatePhysicsMesh(ScriptInterpreter*);
	static void CreatePhysicsHeightmap(ScriptInterpreter*);
	static void CreatePhysicsSpring(ScriptInterpreter*);

	static void UpdatePhysicsWorld(ScriptInterpreter*);
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
