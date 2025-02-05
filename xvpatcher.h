#ifndef XVPATCHER_H
#define XVPATCHER_H

#include <windows.h>
#include <windows.h>
#include <d3d9.h>
#include <d3dcommon.h>
#include <unordered_set>
#include <cstdint>
#include <fstream>

#define EXPORT WINAPI __declspec(dllexport)
#define PUBLIC EXPORT

#define NUM_EXPORTED_FUNCTIONS	18

#define EXE_PATH	    "DBXV.exe"
#define PROCESS_NAME 	"dbxv.exe"
#define DATA_CPK		"data.cpk"
#define DATA2_CPK		"data2.cpk"
#define DATAP1_CPK		"datap1.cpk"
#define DATAP2_CPK		"datap2.cpk"
#define DATAP3_CPK		"datap3.cpk"

#define SLOTS_FILE          "./data/XVP_SLOTS.xs"
#define XVPATCHER_FOLDER    "./XVPATCHER/"
#define LOG_FILE            XVPATCHER_FOLDER "/XVP.log"

#define XVPATCHER_VERSION       "1.06"
#define MINIMUM_GAME_VERSION	L"ver.1.08.00"

#define XV_PATCHER_TAG "XVP"

typedef HRESULT (* GetRenderTargetDataType)(IDirect3DDevice9 *pDevice, IDirect3DBaseTexture9 *pSrc, IDirect3DSurface9 *pDest);
typedef IDirect3DBaseTexture9 *(* GetTextureBufferType)(void *);

// Direct3D 9-specific texture saving function
static GetRenderTargetDataType _GetRenderTargetData;
static GetTextureBufferType GetTextureBuffer;


typedef  void (* IGGYSetTraceCallbackType)(void *callback, void *param);
typedef  void (* IGGYSetWarningCallbackType)(void *callback, void *param);
typedef  void (* IggyForceBitmapSmoothingType)(bool bitmapSmoothingBool);
typedef  void (* IggyGenericSetTextureFromResourceType)(int param_1, unsigned short param_2, int param_3);
typedef  void (* IggyUseExplorerType)(int param_1,int param_2);
typedef PUBLIC int (*  ExternalAS3CallbackType)(void *custom_arg, void *iggy_obj, const char **pfunc_name);
typedef  void * (* IggyPlayerCallbackResultPathType)(void *unk0);
typedef  void (*  IggyValueSetStringUTF8RSType)(void *arg1, void *unk2, void *unk3, const char *str, size_t length);
typedef  void (*  IggyValueSetS32RSType)(void *arg1, uint32_t unk2, uint32_t unk3, uint32_t value);

static IggyPlayerCallbackResultPathType IggyPlayerCallbackResultPath;
static IggyValueSetStringUTF8RSType IggyValueSetStringUTF8RS;
static IggyValueSetS32RSType IggyValueSetS32RS;
static ExternalAS3CallbackType ExternalAS3Callback;


extern void iggy_trace_callback(void *param, void *unk, const char *str, size_t len);
#endif