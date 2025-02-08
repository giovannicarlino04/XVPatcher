#ifndef XVPATCHER_H
#define XVPATCHER_H

#include <windows.h>
#include <windows.h>
#include <d3d9.h>
#include <d3dcommon.h>
#include <unordered_set>
#include <cstdint>
#include <fstream>
#include "IniFile.h"

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
#define DATA_FOLDER     "data"
#define CONTENT_ROOT        "./"
#define SLOTS_FILE          "./data/XVP_SLOTS.xs"
#define XVPATCHER_FOLDER    "./XVPATCHER/"
#define PATCHES_PATH        "./XVPATCHER/EPatches"
#define INI_PATH            "./XVPATCHER/xvpatcher.ini"
#define LOG_FILE            "./XVPATCHER/xv1_log.txt"

#define XVPATCHER_VERSION       "1.10"
#define MINIMUM_GAME_VERSION	L"ver.1.08.00"

#define XV_PATCHER_TAG "XVP"

#define MY_DUMP_BASE	(0x1390000)

#define DPRINTF1_SYMBOL				(0x18AEBD3-MY_DUMP_BASE)
#define DPRINTF2_SYMBOL				(0x18AEBBB-MY_DUMP_BASE)

#define CPK_OPEN_FILE_SYMBOL		(0x192A6B8-MY_DUMP_BASE)
#define CPK_FILE_EXISTS_SYMBOL		(0x176B4E0-MY_DUMP_BASE)

extern std::string myself_path;
extern IniFile ini;

typedef HRESULT (* GetRenderTargetDataType)(IDirect3DDevice9 *pDevice, IDirect3DBaseTexture9 *pSrc, IDirect3DSurface9 *pDest);
typedef IDirect3DBaseTexture9 *(* GetTextureBufferType)(void *);

// Direct3D 9-specific texture saving function
static GetRenderTargetDataType _GetRenderTargetData;
static GetTextureBufferType GetTextureBuffer;

typedef  void (* IGGYSetTraceCallbackType)(void *callback, void *param);
typedef  void (* IGGYSetWarningCallbackType)(void *callback, void *param);

extern void iggy_trace_callback(void *, void *, const char *str, size_t);
extern void iggy_warning_callback(void *, void *, uint32_t, const char *str);

#endif