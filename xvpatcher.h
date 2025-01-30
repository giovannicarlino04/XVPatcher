#ifndef XVPATCHER_H
#define XVPATCHER_H

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

#define XVPATCHER_VERSION "1.06"
#define MINIMUM_GAME_VERSION	L"ver.1.08.00"

#define INI_FILE "./XVPatcher/XVPatcher.ini"


typedef void (* IGGYSetTraceCallbackType)(void *callback, void *param);
typedef void (* IGGYSetWarningCallbackType)(void *callback, void *param);
typedef void (* IggyForceBitmapSmoothingType)(bool bitmapSmoothingBool);
typedef void (* IggyGenericSetTextureFromResourceType)(int param_1, unsigned short param_2, int param_3);
typedef void (* IggyUseExplorerType)(int param_1,int param_2);








extern void iggy_trace_callback(void *param, void *unk, const char *str, size_t len);
#endif