#define _WIN32_WINNT	0x600

#include <windows.h>
#include <winbase.h>
#include <io.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <cstdio>
#include <algorithm>
#include <memory>
#include <thread>

#include "patch.h"
#include "patches.hpp"
#include "debug.h"
#include "symbols.h"
#include "CpkFile.h"
#include "Mutex.h"
#include "Utils.h"
#include "PatchUtils.h"
#include "IggyFile.h"
#include "CpkDef.hpp"
#include "xvpatcher.h"
#include <MinHook.h>

static HMODULE patched_dll;
static Mutex mutex;
HMODULE myself;
std::string myself_path;
wchar_t *version;



std::string x2s_raw;

uintptr_t GetModuleBaseAddress(const wchar_t* modName) {
    HMODULE hModule = GetModuleHandleW(modName);
    if (hModule != NULL) {
        return (uintptr_t)hModule; // Return the base address of the module
    } else {
        return 0; // Module not found
    }
}


static void ReadSlotsFile()
{
	static time_t mtime = 0;
	const std::string path = SLOTS_FILE;
	time_t current_mtime;
	
	if (Utils::GetFileDate(path, &current_mtime) && current_mtime != mtime)
	{
		Utils::ReadTextFile(path, x2s_raw, false);
		mtime = current_mtime;
	}
}

const std::string &GetSlotsData()
{
	ReadSlotsFile();
	return x2s_raw;
}

extern "C" PUBLIC int ExternalAS3CallbackPatched(void *custom_arg, void *iggy_obj, const char **pfunc_name)
{
    if (!pfunc_name || !*pfunc_name)
    {
        DPRINTF("Error: pfunc_name is NULL or empty\n");
        return 0;
    }
    
    const char *func_name = *pfunc_name;

    if (!strstr(func_name, XV_PATCHER_TAG))
    {
    	return ExternalAS3Callback(custom_arg, iggy_obj, pfunc_name);
    }
    
    DPRINTF("Calling %s\n", func_name);

    if (!IggyPlayerCallbackResultPath || !IggyValueSetStringUTF8RS)
    {
        if (!IggyPlayerCallbackResultPath)
        {
            HMODULE iggy = GetModuleHandle("iggy_w32.dll");
            if (!iggy)
            {
                DPRINTF("Error: Failed to get module handle for iggy_w32.dll\n");
                return 0;
            }

            IggyPlayerCallbackResultPath = (IggyPlayerCallbackResultPathType)GetProcAddress(iggy, "_IggyPlayerCallbackResultPath@4");
            if (!IggyPlayerCallbackResultPath)
            {
                DPRINTF("Error: Failed to get IggyPlayerCallbackResultPath address\n");
                return 0;
            }
        }

        if (!IggyValueSetStringUTF8RS)
        {
            HMODULE iggy = GetModuleHandle("iggy_w32.dll");
            if (!iggy)
            {
                DPRINTF("Error: Failed to get module handle for iggy_w32.dll (again)\n");
                return 0;
            }

            IggyValueSetStringUTF8RS = (IggyValueSetStringUTF8RSType)GetProcAddress(iggy, "_IggyValueSetStringUTF8RS@20");
            if (!IggyValueSetStringUTF8RS)
            {
                DPRINTF("Error: Failed to get IggyValueSetStringUTF8RS address\n");
                return 0;
            }
        }
    }

    if (strcmp(func_name, "XVPGetSlots") == 0)
    {

        void *ret = IggyPlayerCallbackResultPath(iggy_obj);
		try{        
			const std::string &slots = GetSlotsData();
			if (slots.empty())
			{
				DPRINTF("Error: GetSlotsData returned empty string\n");
				return 0;
			}

			DPRINTF("%s", slots.c_str());
			IggyValueSetStringUTF8RS(ret, nullptr, nullptr, slots.c_str(), slots.length());
		    return ExternalAS3Callback(custom_arg, iggy_obj, pfunc_name);

		}
		catch (const std::exception& ex){
			DPRINTF(ex.what());
		}
    }

    return ExternalAS3Callback(custom_arg, iggy_obj, pfunc_name);
}


void HookExternalAS3Callback()
{
    // Ottieni l'indirizzo base del modulo
    HMODULE mba = GetModuleHandle("DBXV.exe");
    if (!mba)
    {
        DPRINTF("Failed to get module handle\n");
        return;
    }
    HANDLE hProcess = GetCurrentProcess();

    // L'indirizzo della funzione patchata
    void* patchedFunction = (void*)&ExternalAS3CallbackPatched;

    // Ottieni l'indirizzo della funzione da patchare
    void* patchAddress = (void*)((BYTE*)mba + 0x34A03B);
	ExternalAS3Callback = (ExternalAS3CallbackType)((void*)((BYTE*)mba + 0x34B1D0)); //0x34B1D0

    // Cambia la protezione della memoria per permettere la scrittura
    DWORD oldProtect;
    if (!VirtualProtectEx(hProcess, patchAddress, 4, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        DPRINTF("Failed to change memory protection\n");
        return;
    }


    // Scrivi la nuova istruzione (PUSH l'indirizzo della funzione patchata)
    BYTE patch[5];
    patch[0] = 0x68; // Codice opcode per PUSH
    *(DWORD*)&patch[1] = (DWORD)ExternalAS3CallbackPatched; // Indirizzo della funzione patchata

    // Scrivi in memoria
    if (!WriteProcessMemory(hProcess, patchAddress, patch, sizeof(patch), NULL))
    {
        DPRINTF("Failed to write memory\n");
        return;
    }

    // Ripristina la protezione della memoria
    if (!VirtualProtectEx(hProcess, patchAddress, 5, oldProtect, &oldProtect))
    {
        DPRINTF("Failed to restore memory protection\n");
        return;
    }

}
void iggy_trace_callback(void *, void *, const char *str, size_t)
{
	if (str && strcmp(str, "\n") == 0)
		return;
	
	DPRINTF("IGGY: %s\n", str);

}
void iggy_warning_callback(void *, void *, uint32_t, const char *str)
{
	if (str)
	{
		if (strcmp(str, "\n") == 0 || strstr(str, "Uncaught exception TypeError non-callable object was called with $update in method updateButtons") != nullptr || strstr(str, "Cannot maintain framerate specified in SWF file") != nullptr)
			return;
	}
	
	DPRINTF("%s\n", str);
}

static void IggySetWarningCallbackPatched(void *, void *param)
{
	HMODULE iggy = GetModuleHandle("iggy_w32.dll");
	if (!iggy)
		return;
	
	IGGYSetWarningCallbackType func = (IGGYSetWarningCallbackType)GetProcAddress(iggy, "_IggySetWarningCallback@8");
	
	if (func)
		func((void *)iggy_warning_callback, param);

}

// The following functions are  more like a template setup for other similar functions
// To understand how iggy works


// This function is always set to true in Xenoverse, setting it to false just
// removes smoothing from the bitmaps
static void IggyForceBitmapSmoothingPatched(bool bitmapSmoothingBool)
{
    HMODULE iggy = GetModuleHandle("iggy_w32.dll");
    if (!iggy)
        return;

    IggyForceBitmapSmoothingType func = (IggyForceBitmapSmoothingType)GetProcAddress(iggy, "_IggyForceBitmapSmoothing@4");

    if (func) {
        func(true);
    } else {
        UPRINTF("Failed to find _IggyForceBitmapSmoothing@4 function.\n");
    }
}

static void IggyGenericSetTextureFromResourcePatched(int param_1, unsigned short param_2, int param_3)
{
    HMODULE iggy = GetModuleHandle("iggy_w32.dll");
    if (!iggy)
        return;

    IggyGenericSetTextureFromResourceType func = (IggyGenericSetTextureFromResourceType)GetProcAddress(iggy, "_IggyGenericSetTextureFromResource@12");

    if (func) {
        func(param_1, param_2, param_3);
    } else {
        UPRINTF("Failed to find IggyGenericSetTextureFromResource@12 function.\n");
    }
}
static void IggyUseExplorerPatched (int param_1, int param_2)
{
    HMODULE iggy = GetModuleHandle("iggy_w32.dll");
    if (!iggy)
		return;
	

	IggyUseExplorerType func = (IggyUseExplorerType)GetProcAddress(iggy, " _IggyUseExplorer@8");
	if (!func) {
		UPRINTF("Failed to find IggyUseExplorer@8 in iggy_w32.dll.\n");
		return;
	}
	
    if (func) {
        func(param_1, param_2);
    }
}
//////////////////////////////////////

static void IggySetTraceCallbackUTF8Patched(void *, void *param)
{
	HMODULE iggy = GetModuleHandle("iggy_w32.dll");
	if (!iggy)
		return;
	
	IGGYSetTraceCallbackType func = (IGGYSetTraceCallbackType)GetProcAddress(iggy, "_IggySetTraceCallbackUTF8@8");
	
	if (func)
		func((void *)iggy_trace_callback, param);
}
extern "C"
{
	PUBLIC DWORD XInputGetState()
	{
		DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
		return ERROR_DEVICE_NOT_CONNECTED;
	}
	
	PUBLIC DWORD XInputSetState()
	{
		DPRINTF("%s ****************I have been called but I shouldn't!!!\n", FUNCNAME);
		return ERROR_DEVICE_NOT_CONNECTED;
	}
	
	PUBLIC DWORD XInputGetBatteryInformation(DWORD,  BYTE, void *)
	{
		DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
		return ERROR_DEVICE_NOT_CONNECTED;
	}
	
	PUBLIC void XInputEnable(BOOL)
	{
		DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
	}
	
	PUBLIC DWORD XInputGetCapabilities(DWORD, DWORD, void *)
	{
		DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
		return ERROR_DEVICE_NOT_CONNECTED;
	}
	
	PUBLIC DWORD XInputGetDSoundAudioDeviceGuids(DWORD, void *, void *)
	{
		DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
		return ERROR_DEVICE_NOT_CONNECTED;
	}
	
	PUBLIC DWORD XInputGetKeystroke(DWORD, DWORD, void *)
	{
		DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
		return ERROR_DEVICE_NOT_CONNECTED;
	}
	
	PUBLIC DWORD XInputGetStateEx(DWORD, void *)
	{
		DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
		return ERROR_DEVICE_NOT_CONNECTED;
	}
	
	PUBLIC DWORD XInputWaitForGuideButton(DWORD, DWORD, void *)
	{
		DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
		return ERROR_DEVICE_NOT_CONNECTED;
	}
	
	PUBLIC DWORD XInputCancelGuideButtonWait()
	{
		DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
		return ERROR_DEVICE_NOT_CONNECTED;
	}
	
	PUBLIC DWORD XInputPowerOffController()
	{
		DPRINTF("%s: ****************I have been called but I shouldn't!!!\n", FUNCNAME);
		return ERROR_DEVICE_NOT_CONNECTED;
	}
}

static BOOL InGameProcess(VOID)
{
	char szPath[MAX_PATH];
	
	GetModuleFileName(NULL, szPath, MAX_PATH);
	_strlwr(szPath);
	
	// A very poor aproach, I know
	return (strstr(szPath, PROCESS_NAME) != NULL);
}

static bool load_dll(bool critical)
{
	static const std::vector<const char *> exports =
	{
		"XInputGetState",
		"XInputSetState",
		"XInputGetBatteryInformation",
		"XInputEnable",
		"XInputGetCapabilities",
		"XInputGetDSoundAudioDeviceGuids",
		"XInputGetKeystroke",
		(char *)100,
		(char *)101,
		(char *)102,
		(char *)103
	};	
	
	static char mod_path[2048];
	static char original_path[256];	
	static bool loaded = false;
	
	MutexLocker lock(&mutex);
	
	if (loaded)
		return true;
	
	myself = GetModuleHandle("xinput1_3.dll");

	GetModuleFileNameA(myself, mod_path, sizeof(mod_path));
	
	myself_path = Utils::NormalizePath(mod_path);
	myself_path = myself_path.substr(0, myself_path.rfind('/')+1);	
	DPRINTF("Myself path = %s\n", myself_path.c_str());
	DPRINTF("Slots path = %s\n", SLOTS_FILE);
	
	if (Utils::FileExists(myself_path+"xinput_other.dll"))
	{
		strncpy(original_path, myself_path.c_str(), sizeof(original_path));
		strncat(original_path, "xinput_other.dll", sizeof(original_path));
	}
	else
	{	
		if (GetSystemDirectory(original_path, sizeof(original_path)) == 0)
			return false;
		strncat(original_path, "\\xinput1_3.dll", sizeof(original_path));
	}
	
	patched_dll = LoadLibrary(original_path);		
	if (!patched_dll)
	{
		if (critical)
			UPRINTF("Cannot load original DLL (%s).\nLoadLibrary failed with error %u\n", original_path, (unsigned int)GetLastError());
				
		return false;
	}
	
	DPRINTF("original DLL path: %s   base=%p\n", original_path, patched_dll);
		
	for (auto &export_name : exports)
	{
		uint64_t ordinal = (uint64_t)export_name;
		
		uint8_t *orig_func = (uint8_t *)GetProcAddress(patched_dll, export_name);
		
		if (!orig_func)
		{
			if (ordinal < 0x1000)			
			{
				DPRINTF("Warning: ordinal function %I64d doesn't exist in this system.\n", ordinal);
				continue;		
			}
			else
			{
				if (Utils::IsWine())
				{
					DPRINTF("Failed to get original function: %s --- ignoring error because running under wine.\n", export_name);
					continue;
				}
				else
				{
					UPRINTF("Failed to get original function: %s\n", export_name);			
					return false;
				}
			}
		}
		uint8_t *my_func = (uint8_t *)GetProcAddress(myself, export_name);		
		
		if (!my_func)
		{
			if (ordinal < 0x1000)			
				UPRINTF("Failed to get my function: %I64d\n", ordinal);
			else
				UPRINTF("Failed to get my function: %s\n", export_name);
			
			return false;
		}
		
		if (ordinal < 0x1000)
			DPRINTF("%I64d: address of microsoft: %p, address of mine: %p\n", ordinal, orig_func, my_func);
		else
			DPRINTF("%s: address of microsoft: %p, address of mine: %p\n", export_name, orig_func, my_func);
		
		DPRINTF("Content of microsoft func: %02X%02X%02X%02X%02X\n", orig_func[0], orig_func[1], orig_func[2], orig_func[3], orig_func[4]);
		DPRINTF("Content of my func: %02X%02X%02X%02X%02X\n", my_func[0], my_func[1], my_func[2], my_func[3], my_func[4]);
		
		PatchUtils::Hook(my_func, nullptr, orig_func);
		DPRINTF("Content of my func after patch: %02X%02X%02X%02X%02X\n", my_func[0], my_func[1], my_func[2], my_func[3], my_func[4]);
	}
	loaded = true;
	return true;
}

static void unload_dll()
{
	if (patched_dll)
	{
		FreeLibrary((HMODULE)patched_dll);
		patched_dll = nullptr;
	}
}
// Function to get the last error message
std::string GetLastErrorAsString() {
    DWORD errorMessageID = GetLastError();
    if (errorMessageID == 0) {
        return std::string(); // No error message
    }

    LPVOID messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, 0, (LPSTR)&messageBuffer, 0, NULL);

    std::string message(static_cast<char*>(messageBuffer), size);

    LocalFree(messageBuffer);

    return message;
}

bool CheckVersion(){
	LPCWSTR my_dll_name = L"xinput1_3.dll";
	DPRINTF("XVPATCHER VERSION " XVPATCHER_VERSION ". Exe base = %p. My Dll base = %p. My dll name: %ls\n", GetModuleHandle(NULL), GetModuleHandleW(my_dll_name), my_dll_name);	
	
	/*float version = Utils::GetExeVersion(myself_path+EXE_PATH);

	DPRINTF("Running on game version %.3f\n", version);
	
	if (version != 0.0 && version < (MINIMUM_GAME_VERSION - 0.00001))
	{
		UPRINTF("This game version (%.3f) is not compatible with this version of the patcher.\nMin version required is: %.3f\n", version, MINIMUM_GAME_VERSION);
		exit(-1);
	}

	This doesn't work, they forgot to update the exe version HAHAHAHA

	*/
	
	if(!version){
		UPRINTF("Exe version not found, how tf did it get lost in the first place idk\n");
		return false;
	}
	
	DPRINTF("Running on game version %ls\n", version);

	if(wcscmp(version,	MINIMUM_GAME_VERSION) != 0){
		UPRINTF("Unable to patch game version %ls, please update your game\n", version);
		return false;
	}
	return true;
}


VOID WINAPI GetStartupInfoW_Patched(LPSTARTUPINFOW lpStartupInfo)
{
    static bool started = false;

    if (!started)
    {   
        started = true; 

        if (!load_dll(false))
            exit(-1);

        bool iggy_debug = true;
        if (iggy_debug)
        {
            if (!PatchUtils::HookImport("iggy_w32.dll", "_IggySetTraceCallbackUTF8@8", (void *)IggySetTraceCallbackUTF8Patched))
            {
                UPRINTF("Failed to hook import of _IggySetTraceCallbackUTF8@8.\n");                        
            }
            if (!PatchUtils::HookImport("iggy_w32.dll", "_IggySetWarningCallback@8", (void *)IggySetWarningCallbackPatched))
            {
                UPRINTF("Failed to hook import of _IggySetWarningCallback@8.\n");                        
            }   

        }
    }   
    GetStartupInfoW(lpStartupInfo);
}


DWORD WINAPI StartThread(LPVOID)
{
	load_dll(false);
	return 0;
}


extern "C" BOOL EXPORT DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
       
    HANDLE consoleHandle = nullptr;
	AllocConsole();

	consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (consoleHandle == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {

        if (InGameProcess())
        {
            HANDLE hProcess = GetCurrentProcess();
            uintptr_t moduleBaseAddress = GetModuleBaseAddress(L"DBXV.exe");
            uintptr_t iggy = GetModuleBaseAddress(L"iggy_w32.dll");

            if (!load_dll(false))
                return FALSE;

            version = RetrieveVersionString(hProcess, moduleBaseAddress);
			if(version){
				CheckVersion();
				free(version);
			}
			else{
				return TRUE;
			}
			IggyCrashPatch(hProcess, iggy);
            CMSPatch(hProcess, moduleBaseAddress);
            VersionStringPatch(hProcess, moduleBaseAddress);
            BacBcmPatch(hProcess, moduleBaseAddress);
            InfiniteTimerPatch(hProcess, moduleBaseAddress);
			HookExternalAS3Callback();
			
			CpkFile *data, *data2, *datap1, *datap2, *datap3;
			if (get_cpk_tocs(&data, &data2, &datap1, &datap2, &datap3))
			{
				patch_toc(data);
				patch_toc(data2);
				patch_toc(datap1);
				patch_toc(datap2);
				patch_toc(datap3);

				data->RevertEncryption(false);
				data2->RevertEncryption(false);
				datap1->RevertEncryption(false);
				datap2->RevertEncryption(false);
				datap3->RevertEncryption(false);

				patches();

				delete data;
				delete data2;
				delete datap1;
				delete datap2;
				delete datap3;
			}


			/////////////////////////////////////////////
            if (!PatchUtils::HookImport("KERNEL32.dll", "GetStartupInfoW", (void *)GetStartupInfoW_Patched))
            {
                UPRINTF("GetStartupInfoW hook failed.\n");
                return TRUE;
            }   
    

        }
        break;
    }

    case DLL_PROCESS_DETACH:
    {
        if (!lpvReserved)
        {
            FreeConsole();
            unload_dll();
        }
    }
    break;
    }

    return TRUE;
}