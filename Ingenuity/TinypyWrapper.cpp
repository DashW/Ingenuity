//#include "TinypyWrapper.h"
//
//#include <GpuApi.h>
//#include <InputApi.h>
//#include <WavefrontLoader.h>
//#include "Tinypy\tp.c"
//#include <string>
//#include <set>
//#include <vector>
//
//#define CHAR_TO_WCHAR_T(IN,OUT) \
//	std::string tmp1(IN);\
//	std::wstring tmp2(tmp1.begin(), tmp1.end());\
//	OUT = tmp2.c_str()
//
//std::map<tp_vm*,TinypyWrapper&> TinypyWrapper::instanceMap;
//
//struct IngenuityModule
//{
//#define CHECK_PTR(FNC,PTR) if(validPointers.find(PTR) == validPointers.end())\
//	tp_raise(tp_None, "%s: pointer invalid", FNC)
//
//	static tp_obj CreateFont(tp_vm *tp);
//	static tp_obj DrawText(tp_vm *tp);
//	static tp_obj CreateTextureFromFile(tp_vm *tp);
//	
//	static tp_obj CreateSprite(tp_vm *tp);
//	static tp_obj SetSpritePosition(tp_vm *tp);
//	static tp_obj SetSpriteColor(tp_vm *tp);
//	static tp_obj SetSpriteClipRect(tp_vm *tp);
//	static tp_obj SetSpriteRotateStretch(tp_vm *tp);
//	static tp_obj SetSpriteFlags(tp_vm *tp);
//	static tp_obj DrawSprite(tp_vm *tp);
//
//	//static tp_obj CreateSphere(tp_vm *tp);
//	//static tp_obj CreateGrid(tp_vm *tp);
//	//static tp_obj CreateCube(tp_vm *tp);
//	//static tp_obj CreateLight(tp_vm *tp);
//	//static tp_obj CreateMesh(tp_vm *tp);
//
//	static tp_obj LoadObj(tp_vm *tp);
//	//static tp_obj SetModelPosition(tp_vm *tp);
//	//static tp_obj SetModelRotation(tp_vm *tp);
//	//static tp_obj SetModelScaling(tp_vm *tp);
//	//static tp_obj SetModelFlags(tp_vm *tp);
//
//	//static tp_obj AddToScene(tp_vm *tp);
//	static tp_obj DrawModel(tp_vm *tp);
//
//	static tp_obj CreateCamera(tp_vm *tp);
//	static tp_obj SetCameraPosition(tp_vm *tp);
//	static tp_obj SetCameraTarget(tp_vm *tp);
//	static tp_obj SetCameraPerspective(tp_vm *tp);
//
//	static tp_obj GetKeyHeld(tp_vm *tp);
//	static tp_obj GetKeyUp(tp_vm *tp);
//	static tp_obj GetKeyDown(tp_vm *tp);
//
//	static std::set<void*> validPointers;
//
//	static GpuApi *gpu;
//	static AssetMgr *assets;
//	static KeyState *keyState;
//};
////struct IngenuityModule
//
//std::set<void*> IngenuityModule::validPointers;
//
//GpuApi *IngenuityModule::gpu = 0;
//AssetMgr *IngenuityModule::assets = 0;
//KeyState *IngenuityModule::keyState = 0;
//
//TinypyWrapper::TinypyWrapper()
//{
//	tp = tp_init(0,0);
//
//	instanceMap.insert(std::pair<tp_vm*,TinypyWrapper&>(tp, *this));
//}
//
//TinypyWrapper::~TinypyWrapper()
//{
//	instanceMap.erase(tp);
//
//	tp_deinit(tp);
//}
//
//void TinypyWrapper::Import(const char *filename, const char *modname)
//{
//	error = false;
//	if(tp_has(tp,tp->modules,tp_string(modname)).number.val > 0)
//	{
//		tp_obj g = tp_get(tp,tp->modules,tp_string(modname));
//
//		//tp_obj funcPath = tpw_find_funcs(tp,g);
//
//		//if(funcPath.type == TP_NONE)
//		//{
//			tp_import(tp, filename, "__temp__", 0);
//
//			tp_obj t = tp_get(tp,tp->modules,tp_string("__temp__"));
//
//			if(tp_has(tp,t,tp_string("__code__")).number.val < 1)
//			{
//				logger.Log("Syntax error importing module %s\n", modname);
//				error = true;
//				tp_del(tp,tp->modules,tp_string("__temp__"));
//			}
//			else
//			{
//				_tp_dict* tempDict = t.dict.val;
//				for(int i = 0; i < tempDict->alloc; i++)
//				{
//					tp_item &item = tempDict->items[i];
//
//					if(item.val.type == TP_NONE) continue;
//					if(item.val.type == TP_NUMBER) continue;
//					if(item.val.type == TP_STRING && tp_cmp(tp,item.key,tp_string("__file__")) != 0) continue; 
//					if(item.val.type == TP_DICT && (tp_has(tp,item.val,tp_string("__file__")).number.val > 0)) continue;
//					if(item.val.type == TP_DICT && (tp_has(tp,item.val,tp_string("__new__")).number.val < 1)) continue;
//
//					tp_obj key = tempDict->items[i].key;
//					tp_obj val = tempDict->items[i].val;
//			
//					if(val.type == TP_FNC)
//					{
//						val.fnc.info->globals = g;
//					}
//					tp_set(tp,g,key,val);
//				}
//			}
//		//}
//		//else
//		//{
//		//	logger.Log("Cannot safely refresh: state contains a function\n");
//		//	logger.Log("%s\n",funcPath.string.val);
//		//}
//	}
//	else
//	{
//		tp_import(tp, filename, modname, 0);
//
//		tp_obj module = tp_get(tp,tp->modules,tp_string(modname));
//
//		if(tp_has(tp,module,tp_string("__code__")).number.val < 1)
//		{
//			tpw_log(tp,"Syntax error importing module %s\n", modname);
//			tpw_err(tp);
//			tp_del(tp,tp->modules,tp_string(modname));
//		}
//	}
//}
//
//void TinypyWrapper::Call(const char *modname, const char *fncname, int argc, ...)
//{
//    tp_obj r = tp_params(tp);
//    va_list a; va_start(a, argc);
//    for (int i = 0; i < argc; i++) {
//		TinypyParam tparam = (TinypyParam) va_arg(a, TinypyParam);
//		tp_obj param;
//		switch (tparam.paramType)
//		{
//		case TinypyParam::INT:
//			param = tp_number(*((int*)(tparam.item)));
//			break;
//		case TinypyParam::FLOAT:
//			param = tp_number(*((float*)(tparam.item)));
//			break;
//		case TinypyParam::STRING:
//			// TODO: THIS NEEDS TO CONVERT WCHARS!
//			param = tp_string((const char*)tparam.item);
//			break;
//		case TinypyParam::POINTER:
//		default:
//			param = tp_number((int)tparam.item);
//			break;
//		}
//        _tp_list_append(tp,r.list.val,param);
//    }
//    va_end(a);
//	tp_call(tp, modname, fncname, r);
//}
//
//TinypyParam TinypyWrapper::Eval(char* expr)
//{
//	tp_obj globals = tp_get(tp, tp->modules, tp_string("__main__"));
//	tp_obj out = tp_eval(tp, expr, globals);
//	switch(out.type)
//	{
//	case TP_NUMBER:
//		return TinypyParam(TinypyParam::FLOAT, &out.number.val);
//	case TP_STRING:
//		return TinypyParam(TinypyParam::STRING, (void*) out.string.val);
//	default:
//		return TinypyParam(TinypyParam::NONE, 0);
//	}
//}
//
//void TinypyWrapper::Error()
//{
//	error = true;
//}
//
//bool TinypyWrapper::IsInError()
//{
//	return error;
//}
//
//void TinypyWrapper::RegisterModule()
//{
//	tp_obj module = tp_dict(tp);
//
//	tp_set(tp, module, tp_string("CreateFont"), 
//		tp_fnc(tp, IngenuityModule::CreateFont));
//	tp_set(tp, module,
//		tp_string("DrawText"),
//		tp_fnc(tp, IngenuityModule::DrawText));
//	tp_set(tp, module,
//		tp_string("CreateTextureFromFile"),
//		tp_fnc(tp, IngenuityModule::CreateTextureFromFile));
//	
//	tp_set(tp, module,
//		tp_string("CreateSprite"),
//		tp_fnc(tp, IngenuityModule::CreateSprite));
//	tp_set(tp, module,
//		tp_string("SetSpritePosition"),
//		tp_fnc(tp, IngenuityModule::SetSpritePosition));
//	tp_set(tp, module,
//		tp_string("SetSpriteColor"),
//		tp_fnc(tp, IngenuityModule::SetSpriteColor));
//	tp_set(tp, module,
//		tp_string("SetSpriteClipRect"),
//		tp_fnc(tp, IngenuityModule::SetSpriteClipRect));
//	tp_set(tp, module,
//		tp_string("SetSpriteRotateStretch"),
//		tp_fnc(tp, IngenuityModule::SetSpriteRotateStretch));
//	tp_set(tp, module,
//		tp_string("SetSpriteFlags"),
//		tp_fnc(tp, IngenuityModule::SetSpriteFlags));
//	tp_set(tp, module,
//		tp_string("DrawSprite"),
//		tp_fnc(tp, IngenuityModule::DrawSprite));
//
//	tp_set(tp, module,
//		tp_string("LoadObj"),
//		tp_fnc(tp, IngenuityModule::LoadObj));
//	tp_set(tp, module,
//		tp_string("DrawModel"),
//		tp_fnc(tp, IngenuityModule::DrawModel));
//	tp_set(tp, module,
//		tp_string("CreateCamera"),
//		tp_fnc(tp, IngenuityModule::CreateCamera));
//	tp_set(tp, module,
//		tp_string("SetCameraPosition"),
//		tp_fnc(tp, IngenuityModule::SetCameraPosition));
//	tp_set(tp, module,
//		tp_string("SetCameraTarget"),
//		tp_fnc(tp, IngenuityModule::SetCameraTarget));
//	tp_set(tp, module,
//		tp_string("SetCameraPerspective"),
//		tp_fnc(tp, IngenuityModule::SetCameraPerspective));
//
//	tp_set(tp, module,
//		tp_string("GetKeyHeld"),
//		tp_fnc(tp, IngenuityModule::GetKeyHeld));
//	tp_set(tp, module,
//		tp_string("GetKeyUp"),
//		tp_fnc(tp, IngenuityModule::GetKeyUp));
//	tp_set(tp, module,
//		tp_string("GetKeyDown"),
//		tp_fnc(tp, IngenuityModule::GetKeyDown));
//
//    tp_set(tp, module, tp_string("__name__"), tp_string("ingenuity"));
//    tp_set(tp, module, tp_string("__file__"), tp_string(__FILE__));
//
//	tp_set(tp, tp->modules, tp_string("ingenuity"), module);
//
//	tp_get(tp, tp->modules, tp_string("ingenuity"));
//}
//
//void TinypyWrapper::SetState(GpuApi* gpu, AssetMgr *assets, KeyState* keyboard)
//{
//	IngenuityModule::gpu = gpu;
//	IngenuityModule::assets = assets;
//	IngenuityModule::keyState = keyboard;
//}
//
//ScriptLogger& TinypyWrapper::GetLogger()
//{
//	return logger;
//}
//
//TinypyWrapper* TinypyWrapper::GetInstance(tp_vm* ref)
//{
//	if(instanceMap.find(ref) != instanceMap.end())
//	{
//		return &instanceMap.at(ref);
//	}
//	return 0;
//}
//
//tp_obj IngenuityModule::CreateFont(tp_vm *tp)
//{
//	double height = TP_NUM();
//	const char* facename = TP_STR();
//
//	const wchar_t* out;
//	CHAR_TO_WCHAR_T(facename, out);
//
//	GpuFont* font = gpu->CreateGpuFont((int)height, out);
//	
//	validPointers.insert(font);
//	return tp_data(tp,0,font);
//}
//
//tp_obj IngenuityModule::DrawText(tp_vm *tp)
//{
//	void* fontPtr = TP_OBJ().data.val;
//	CHECK_PTR("DrawText",fontPtr);
//	const char* text = TP_STR();
//	double x = TP_NUM();
//	double y = TP_NUM();
//	bool center = TP_NUM() > 0.0;
//
//	GpuFont* font = (GpuFont*) fontPtr;
//
//	const wchar_t* out;
//	CHAR_TO_WCHAR_T(text,out);
//
//	gpu->DrawGpuText(font, out, (float)x, (float)y, center);
//
//	return tp_None;
//}
//
//tp_obj	IngenuityModule::CreateTextureFromFile(tp_vm *tp)
//{
//	const char* path = TP_STR();
//
//	const wchar_t* out;
//	CHAR_TO_WCHAR_T(path,out);
//	
//	GpuTexture* tex = gpu->CreateGpuTextureFromFile(out);
//	
//	validPointers.insert(tex);
//	return tp_data(tp,0,tex);
//}
//
//tp_obj IngenuityModule::CreateSprite(tp_vm *tp)
//{
//	void* texPtr = TP_OBJ().data.val;
//	CHECK_PTR("CreateSrpite",texPtr);
//	GpuTexture* tex = (GpuTexture*) texPtr;
//	float centerX = (float) TP_NUM();
//	float centerY = (float) TP_NUM();
//	float size = (float) TP_NUM();
//
//	GpuSprite* sprite = new GpuSprite(tex, centerX, centerY, size);
//	sprite->pixelSpace = true;
//	
//	validPointers.insert(sprite);
//	return tp_data(tp,0,sprite);
//}
//
//tp_obj IngenuityModule::SetSpritePosition(tp_vm *tp)
//{
//	void* spritePtr = TP_OBJ().data.val;
//	CHECK_PTR("SetSpritePosition",spritePtr);
//	GpuSprite* sprite = (GpuSprite*) spritePtr;
//	sprite->position.x = (float) TP_NUM();
//	sprite->position.y = (float) TP_NUM();
//	if(tp->params.list.val->len)
//	{
//		sprite->position.z = (float) TP_NUM();
//	}
//	return tp_None;
//}
//
//tp_obj IngenuityModule::SetSpriteColor(tp_vm *tp)
//{
//	void* spritePtr = TP_OBJ().data.val;
//	CHECK_PTR("SetSpriteColor",spritePtr);
//	GpuSprite* sprite = (GpuSprite*) spritePtr;
//	sprite->color.r = (float) TP_NUM();
//	sprite->color.g = (float) TP_NUM();
//	sprite->color.b = (float) TP_NUM();
//	if(tp->params.list.val->len)
//	{
//		sprite->color.a = (float) TP_NUM();
//	}
//	return tp_None;
//}
//
//tp_obj IngenuityModule::SetSpriteClipRect(tp_vm *tp)
//{
//	void* spritePtr = TP_OBJ().data.val;
//	CHECK_PTR("SetSpriteClipRect",spritePtr);
//	GpuSprite* sprite = (GpuSprite*) spritePtr;
//	sprite->clipRect = GpuRect((float)TP_NUM(),
//		(float)TP_NUM(),(float)TP_NUM(),(float)TP_NUM());
//	return tp_None;
//}
//
//tp_obj IngenuityModule::SetSpriteRotateStretch(tp_vm *tp)
//{
//	void* spritePtr = TP_OBJ().data.val;
//	CHECK_PTR("SetSpriteRotateStretch",spritePtr);
//	GpuSprite* sprite = (GpuSprite*) spritePtr;
//	sprite->rotation = (float) TP_NUM();
//	sprite->scale.x = (float) TP_NUM();
//	sprite->scale.y = (float) TP_NUM();
//	return tp_None;
//}
//
//tp_obj IngenuityModule::SetSpriteFlags(tp_vm *tp)
//{
//	void* spritePtr = TP_OBJ().data.val;
//	CHECK_PTR("SetSpriteFlags",spritePtr);
//	GpuSprite* sprite = (GpuSprite*) spritePtr;
//	sprite->pixelSpace = (TP_NUM() > 0);
//	sprite->brightAsAlpha = (TP_NUM() > 0);
//	return tp_None;
//}
//
//tp_obj IngenuityModule::DrawSprite(tp_vm *tp)
//{
//	void* spritePtr = TP_OBJ().data.val;
//	CHECK_PTR("DrawSprite",spritePtr);
//	GpuSprite* sprite = (GpuSprite*) spritePtr;
//
//	gpu->DrawGpuSprite((GpuSprite*)spritePtr);
//
//	return tp_None;
//}
//
//tp_obj IngenuityModule::LoadObj(tp_vm *tp)
//{
//	const char* path = TP_STR();
//	const wchar_t* out;
//	CHAR_TO_WCHAR_T(path,out);
//
//	WavefrontLoader loader(gpu, assets, out);
//	GpuComplexModel* model = loader.LoadModel();
//	
//	if(model)
//	{
//		validPointers.insert(model);
//		return tp_data(tp, 0, model);
//	}
//	else
//	{
//		return tp_None;
//	}
//}
//
//tp_obj IngenuityModule::DrawModel(tp_vm *tp)
//{
//	void* mdlRef = TP_OBJ().data.val;
//	void* camRef = TP_OBJ().data.val;
//	CHECK_PTR("DrawModel",mdlRef);
//	CHECK_PTR("DrawModel",camRef);
//	GpuComplexModel* model = (GpuComplexModel*) mdlRef;
//	GpuCamera* camera = (GpuCamera*) camRef;
//
//	model->BeDrawn(gpu,camera,0,0);
//
//	return tp_None;
//}
//
//tp_obj IngenuityModule::CreateCamera(tp_vm *tp)
//{
//	GpuCamera *camera = new GpuCamera();
//	validPointers.insert(camera);
//	return tp_data(tp, 0, camera);
//}
//
//tp_obj IngenuityModule::SetCameraPosition(tp_vm *tp)
//{
//	void* ref = TP_OBJ().data.val;
//	CHECK_PTR("SetCameraPosition",ref);
//	GpuCamera* camera = (GpuCamera*) ref;
//	camera->position.x = (float) TP_NUM();
//	camera->position.y = (float) TP_NUM();
//	camera->position.z = (float) TP_NUM();
//	return tp_None;
//}
//
//tp_obj IngenuityModule::SetCameraTarget(tp_vm *tp)
//{
//	void* ref = TP_OBJ().data.val;
//	CHECK_PTR("SetCameraTarget",ref);
//	GpuCamera* camera = (GpuCamera*) ref;
//	camera->target.x = (float) TP_NUM();
//	camera->target.y = (float) TP_NUM();
//	camera->target.z = (float) TP_NUM();
//	return tp_None;
//}
//
//tp_obj IngenuityModule::SetCameraPerspective(tp_vm *tp)
//{
//	void* ref = TP_OBJ().data.val;
//	CHECK_PTR("SetCameraPerspective",ref);
//	GpuCamera* camera = (GpuCamera*) ref;
//	camera->fovOrHeight = (float) TP_NUM();
//	camera->nearClip = (float) TP_NUM();
//	camera->farClip = (float) TP_NUM();
//	return tp_None;
//}
//
//tp_obj IngenuityModule::GetKeyHeld(tp_vm *tp)
//{
//	int number = (int) TP_NUM();
//	if(number < 0 || number > 255) return tp_number(0);
//	if(keyState->keys[number])
//	{
//		return tp_number(1);
//	}
//	else
//	{
//		return tp_number(0);
//	}
//}
//
//tp_obj IngenuityModule::GetKeyUp(tp_vm *tp)
//{
//	int number = (int) TP_NUM();
//	if(number < 0 || number > 255) return tp_number(0);
//	if(keyState->upKeys[number])
//	{
//		return tp_number(1);
//	}
//	else
//	{
//		return tp_number(0);
//	}
//}
//
//tp_obj IngenuityModule::GetKeyDown(tp_vm *tp)
//{
//	int number = (int) TP_NUM();
//	if(number < 0 || number > 255) return tp_number(0);
//	if(keyState->downKeys[number])
//	{
//		return tp_number(1);
//	}
//	else
//	{
//		return tp_number(0);
//	}
//}
//
////tp_obj tpw_stringify(tp_vm* tp, tp_obj obj, bool data)
////{
////	_tp_dict *inDict;
////	_tp_list *inList;
////	int i;
////	char* newString;
////	std::string s;
////	tp_obj child;
////
////	switch(obj.type)
////	{
////	case TP_NONE:
////		return tp_string("None");
////	case TP_NUMBER:
////		return tp_str(tp, obj);
////	case TP_STRING:
////		s = "\"";
////		s.append(obj.string.val);
////		s.append("\"");
////		newString = new char[s.size() + 1];
////		memcpy(newString, s.c_str(), s.size() + 1);
////		return tp_string(newString);
////	case TP_DICT:
////		inDict = obj.dict.val;
////		s = "{\n";
////		for(i = 0; i < inDict->len; i++)
////		{
////			child = tpw_stringify(tp, inDict->items[i].key);
////			if(child.type != TP_NONE)
////			{
////				s.append(child.string.val);
////				s.append(": ");
////			}
////			child = tpw_stringify(tp, inDict->items[i].val);
////			if(child.type != TP_NONE)
////			{
////				s.append(child.string.val);
////				s.append(" \n");
////			}
////		}
////		s.append("}\n\n");
////		newString = new char[s.size() + 1];
////		memcpy(newString, s.c_str(), s.size() + 1);
////		return tp_string(newString);
////	case TP_LIST:
////
////	}
////}
//
//// NOT WORKING
////tp_obj tpw_deep_copy(tp_vm* tp, tp_obj obj)
////{
////	_tp_dict *inDict, *outDict;
////	_tp_list *inList, *outList;
////	int i;
////
////    switch(obj.type)
////	{
////	case TP_NONE:
////		return tp_None;
////	case TP_NUMBER:
////		return tp_number(obj.number.val);
////	case TP_STRING:
////		{
////        tp_obj r = tp_string_t(tp,obj.string.len);
////        char *ptr = r.string.info->s;
////        memcpy(ptr,obj.string.val,obj.string.len);
////		return r;
////		}
////	case TP_DICT:
////		{
////			tp_obj r = {TP_DICT};
////			inDict = obj.dict.val;
////			outDict = r.dict.val = _tp_dict_new();
////			for(i = 0; i < inDict->alloc; i++)
////			{
////				if(inDict->items[i].used)
////				{
////					tp_obj key = tpw_deep_copy(tp,inDict->items[i].key);
////					tp_obj val = tpw_deep_copy(tp,inDict->items[i].val);
////					if(val.type == TP_FNC || val.type == TP_LIST)
////					{
////						tpw_err(tp);
////					}
////					_tp_dict_set(tp,outDict,key,val);
////				}
////			}
////			return r;
////		}
////	case TP_LIST:
////		{
////			tp_obj r = {TP_LIST};
////			inList = obj.list.val;
////			outList = r.list.val = _tp_list_new();
////			for(i = 0; i < inList->len; i++)
////			{
////				_tp_list_append(tp,outList,tpw_deep_copy(tp,inList->items[i]));
////			}
////			return r;
////		}
////	case TP_FNC:
////		tpw_log(tp,"Deep Copy: attempted to copy function pointer %d \n", obj.fnc.val);
////		tpw_err(tp);
////		return tp_None;
////	case TP_DATA:
////		return tp_data(tp,0,obj.data.val);
////	}
////	tpw_log(tp,"Deep Copy: unrecognised type %d", obj.type);
////	tpw_err(tp);
////	return tp_None;
////}
//
//// WORKING
////tp_obj tpw_find_funcs(tp_vm* tp, tp_obj obj, std::set<void*> &seen)
////{
////	
////	switch(obj.type)
////	{
////	case TP_NONE:
////	case TP_NUMBER:
////	case TP_STRING:
////	case TP_DATA:
////		return tp_None;
////	case TP_DICT:
////		{
////			_tp_dict* inDict = obj.dict.val;
////
////			if(seen.find(inDict) != seen.end())
////				return tp_None;
////			seen.emplace(inDict);
////
////			for(int i = 0; i < inDict->alloc; i++)
////			{
////				if(inDict->items[i].used)
////				{
////					tp_obj key = inDict->items[i].key;
////					tp_obj val = inDict->items[i].val;
////					tp_obj keyResult = tpw_find_funcs(tp,key);
////					if(keyResult.type != TP_NONE)
////					{
////						return tp_printf(tp,"%s{%s}",
////							tp_str(tp,key).string.val,
////							keyResult.string.val);
////					}
////					tp_obj valResult = tpw_find_funcs(tp,obj);
////					if(valResult.type != TP_NONE)
////					{
////						return tp_printf(tp,"%s.%s",
////							tp_str(tp,key).string.val,
////							valResult.string.val);
////					}
////
////				}
////			}
////			return tp_None;
////		}
////	case TP_LIST:
////		{
////			_tp_list* inList = obj.list.val;
////
////			if(seen.find(inList) != seen.end())
////				return tp_None;
////			seen.emplace(inList);
////
////			for(int i = 0; i < inList->len; i++)
////			{
////				tp_obj item = inList->items[i];
////				tp_obj result = tpw_find_funcs(tp,item);
////				if(result.type != TP_NONE)
////				{
////					return tp_printf(tp,"item[%d].%s",i,result.string.val);
////				}
////			}
////			return tp_None;
////		}
////	case TP_FNC:
////		return tp_string("");
////	}
////	return tp_printf(tp,"Actually it was an unrecognised object type: %d", obj.type);
////}
//
//// NOT WORKING
////void tpw_deep_copy(tp_vm* tp, tp_obj in, tp_obj& out)
////{
////    switch(in.type)
////	{
////	case TP_NONE:
////		out = tp_None;
////		return;
////	case TP_NUMBER:
////		out = tp_number(in.number.val);
////		return;
////	case TP_STRING:
////		{
////        out = tp_string_t(tp,in.string.len);
////        char *ptr = out.string.info->s;
////        memcpy(ptr,in.string.val,in.string.len);
////		}
////		return;
////	case TP_DICT:
////		{
////			if(out.type != TP_DICT) { out = tp_dict(tp); }
////			_tp_dict* inDict = in.dict.val;
////			_tp_dict* outDict = out.dict.val;
////			for(int i = 0; i < inDict->alloc; i++)
////			{
////				if(inDict->items[i].used && inDict->items[i].val.type != TP_FNC)
////				{
////					tp_obj key = tpw_deep_copy(tp,inDict->items[i].key);
////					tp_obj val = tpw_deep_copy(tp,inDict->items[i].val);
////					_tp_dict_set(tp,outDict,key,val);
////				}
////			}
////		}
////		return;
////	case TP_LIST:
////		{
////			if(out.type != TP_LIST) { out = tp_list(tp); }
////			_tp_list* inList = in.list.val;
////			_tp_list* outList = out.list.val;
////			for(int i = 0; i < inList->len; i++)
////			{
////				if(inList->items[i].type != TP_FNC)
////				{
////					_tp_list_set(tp,outList,i,tpw_deep_copy(tp,inList->items[i]),0);
////				}
////			}
////		}
////		return;
////	case TP_FNC:
////		tpw_log(tp,"Deep Copy: attempted to copy function pointer %d \n", in.fnc.val);
////		tpw_err(tp);
////		return;
////	case TP_DATA:
////		out = tp_data(tp,0,in.data.val);
////		return;
////	}
////}
//
//void tpw_log(TP, const char* fmt, ...)
//{
//	TinypyWrapper* wrapper = TinypyWrapper::GetInstance(tp);
//	if(wrapper)
//	{
//		ScriptLogger& logger = wrapper->GetLogger();
//		va_list args;
//		va_start(args,fmt);
//		logger.Log(fmt,args);
//		va_end(args);
//	}
//}
//
//void tpw_err(TP)
//{
//	TinypyWrapper* wrapper = TinypyWrapper::GetInstance(tp);
//	if(wrapper){ wrapper->Error(); }
//}
//
//bool tpw_in_err(TP)
//{
//	TinypyWrapper* wrapper = TinypyWrapper::GetInstance(tp);
//	if(wrapper){ return wrapper->IsInError(); }
//	return false;
//}
