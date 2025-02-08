#define _WIN32_WINNT	0x600

#include "debug.h"
#include "Mutex.h"
#include "Utils.h"
#include "PatchUtils.h"
#include "IggyFile.h"
#include "xvpatcher.h"
#include "EPatchFile.h"
#include "IniFile.h"

#include <wininet.h>
#include <windows.h>
#include "CpkFile.h"
#include "cpkdef.h"

static int num_patches_fail;
static bool count_patches_failures;
static HMODULE patched_dll;
static Mutex mutex;
HMODULE myself;
wchar_t *version;
std::string x2s_raw;
std::string myself_path;
DWORD initial_tick;
IniFile ini;

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

static void IggySetTraceCallbackUTF8Patched(void *, void *param)
{

	HMODULE iggy = GetModuleHandle("iggy_w32.dll");
	if (!iggy){
		DPRINTF("No iggy?");
		return;
	}

	IGGYSetTraceCallbackType func = (IGGYSetTraceCallbackType)GetProcAddress(iggy, "_IggySetTraceCallbackUTF8@8");

	if (func)
		func((void *)iggy_trace_callback, param);

}
extern "C" {

typedef  void (* IggyForceBitmapSmoothingType)(bool bitmapSmoothingBool);
typedef  void (* IggyGenericSetTextureFromResourceType)(int param_1, unsigned short param_2, int param_3);
typedef  void (* IggyUseExplorerType)(int param_1,int param_2);
typedef  PUBLIC int (*  ExternalAS3CallbackType)(void *custom_arg, void *iggy_obj, const char **pfunc_name);
typedef  void * (* IggyPlayerCallbackResultPathType)(void *unk0);
typedef  void (*  IggyValueSetStringUTF8RSType)(void *arg1, void *unk2, void *unk3, const char *str, size_t length);
typedef  void (*  IggyValueSetS32RSType)(void *arg1, uint32_t unk2, uint32_t unk3, uint32_t value);

static IggyPlayerCallbackResultPathType IggyPlayerCallbackResultPath;
static IggyValueSetStringUTF8RSType IggyValueSetStringUTF8RS;
static IggyValueSetS32RSType IggyValueSetS32RS;
static ExternalAS3CallbackType ExternalAS3Callback;



PUBLIC int ExternalAS3CallbackPatched(void *custom_arg, void *iggy_obj, const char **pfunc_name)
{    

	if (pfunc_name && *pfunc_name)
	{
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

} // extern "C"
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

std::vector<std::string> ignore_modules;

static LONG CALLBACK ExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
    void *fault_addr;
    void *fault_addr_rel = nullptr;
    HMODULE fault_mod;
    std::string fault_mod_name;
    char path[MAX_PATH];
    
    fault_addr = (void *)ExceptionInfo->ExceptionRecord->ExceptionAddress;    
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)fault_addr, &fault_mod);    
    
    if (fault_mod)
    {
        fault_addr_rel = (void *)((uintptr_t)fault_addr - (uintptr_t)fault_mod);
        GetModuleFileNameA(fault_mod, path, MAX_PATH);
        fault_mod_name = path;
        
        std::string fn = Utils::GetFileNameString(fault_mod_name);
        
        for (const std::string &mod : ignore_modules)
        {
            if (Utils::ToLowerCase(mod) == Utils::ToLowerCase(fn))
                return EXCEPTION_CONTINUE_SEARCH;
        }
    }
    
    DPRINTF("*************************** EXCEPTION CAUGHT ***************************\n");
    
    if (fault_mod)
    {
        DPRINTF("Exception code=0x%x at address %p (%p) Faulting module = %s\n", (unsigned int)ExceptionInfo->ExceptionRecord->ExceptionCode, fault_addr, fault_addr_rel, fault_mod_name.c_str());
    }
    else
    {
        DPRINTF("Exception code=0x%x at address %p\n", (unsigned int)ExceptionInfo->ExceptionRecord->ExceptionCode, fault_addr);
    }

    DPRINTF("EIP = %p, ESP = %p, EAX = %p, EBX = %p\n", (void *)ExceptionInfo->ContextRecord->Eip, (void *)ExceptionInfo->ContextRecord->Esp, 
                                                    (void *)ExceptionInfo->ContextRecord->Eax, (void *)ExceptionInfo->ContextRecord->Ebx);
                                                    
    DPRINTF("ECX = %p, EDX = %p, ESI = %p, EDI = %p\n",  (void *)ExceptionInfo->ContextRecord->Ecx, (void *)ExceptionInfo->ContextRecord->Edx, 
                                                    (void *)ExceptionInfo->ContextRecord->Esi, (void *)ExceptionInfo->ContextRecord->Edi);
                                                    
    DPRINTF("************************************************************************\n");
    
    return EXCEPTION_CONTINUE_SEARCH;
}

VOID WINAPI GetStartupInfoW_Patched(LPSTARTUPINFOW lpStartupInfo)
{
	static bool started = false;
	if (!started)
	{	
		int default_log_level;
		
		if (!load_dll(true))
			exit(-1);
		
		ini.LoadFromFile(myself_path + INI_PATH);
		ini.GetIntegerValue("General", "default_log_level", &default_log_level, 1);
		ini.AddIntegerConstant("auto", 0);
		set_debug_level(default_log_level);				
	
		bool iggy_trace, iggy_warning, exception_handler, loose_files;
		
		started = true;
		ini.GetBooleanValue("Patches", "iggy_trace", &iggy_trace, false);
		ini.GetBooleanValue("Patches", "iggy_warning", &iggy_warning, false);
		ini.GetBooleanValue("Debug", "exception_handler", &exception_handler, false);
		ini.GetBooleanValue("Patches", "loose_files", &loose_files, false);

		if(loose_files){

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
	
		}
		if (!PatchUtils::HookImport("KERNEL32.dll", "GetStartupInfoW", (void *)GetStartupInfoW_Patched))
		{
			DPRINTF("GetStartupInfoW hook failed.\n");
		}

	
		if (iggy_trace)
		{
			if (!PatchUtils::HookImport("iggy_w32.dll", "_IggySetTraceCallbackUTF8@8", (void *)IggySetTraceCallbackUTF8Patched))
			{
				DPRINTF("Failed to hook import of _IggySetTraceCallbackUTF8@8.\n");						
			}
		}

		if (exception_handler)
		{
			ini.GetMultipleStringsValues("Debug", "exception_ignore_modules", ignore_modules);			
			AddVectoredExceptionHandler(1, ExceptionHandler);
		}

	}
	
	GetStartupInfoW(lpStartupInfo);
}


DWORD WINAPI StartThread(LPVOID)
{
	load_dll(false);
	return 0;
}

static bool load_patch_file(const std::string &path, bool is_directory, void *custom_param)
{
	if (!Utils::EndsWith(path, ".xml", false))
		return true;
	
	int num_fails = 0;


	EPatchFile epf(myself_path+"xinput1_3.dll");
	
	int enabled;
	bool enabled_b;
	std::string setting;
	
	if (!epf.CompileFromFile(path))
	{
		UPRINTF("Failed to compile file \"%s\"\n", path.c_str());
		exit(-1);
	}
	
	typedef bool (* ConditionalFunction)();
	
	//DPRINTF("File %s compiled.\n", path.c_str());		
	if ((enabled = epf.GetEnabled(setting)) < 0)
	{	
		if (Utils::EndsWith(setting, "()"))
		{
			const std::string funcion_name = setting.substr(0, setting.rfind("()"));
			ConditionalFunction condition = (ConditionalFunction) GetProcAddress(myself, funcion_name.c_str());
			
			if (condition && condition())
			{
				enabled = true;
			}
			else
			{
				enabled = false;
			}
		}
		else
		{
			ini.GetBooleanValue("Patches", setting, &enabled_b, false);
			enabled = enabled_b;
		}
	}
	
	if (!enabled)
		return true;
	
	for (EPatch &patch : epf)
	{
		if ((enabled = patch.GetEnabled(setting)) < 0)
		{
			if (Utils::EndsWith(setting, "()"))
			{
				const std::string funcion_name = setting.substr(0, setting.rfind("()"));
				ConditionalFunction condition = (ConditionalFunction) GetProcAddress(myself, funcion_name.c_str());
				
				if (condition && condition())
				{
					enabled = true;
				}
				else
				{
					enabled = false;
				}
			}				
			else
			{
				ini.GetBooleanValue("Patches", setting, &enabled_b, false);
				enabled = enabled_b;
			}
		}
		
		if (!enabled)
			continue;
		
		if (!patch.Apply())
		{
			if (count_patches_failures)
			{
				DPRINTF("Failed to apply patch \"%s:%s\"\n", epf.GetName().c_str(), patch.GetName().c_str());
				num_fails++;
			}
			else
			{
				UPRINTF("Failed to apply patch \"%s:%s\"\n", epf.GetName().c_str(), patch.GetName().c_str());
				exit(-1);
			}
		}
	}

	if (count_patches_failures && num_fails > 0)
	{
		num_patches_fail += num_fails;
		DPRINTF("%s: %d/%Id patches failed.\n", epf.GetName().c_str(), num_fails, epf.GetNumPatches());
	}
	
	return true;
}

static void load_patches()
{
	ini.GetBooleanValue("Debug", "count_patches_failures", &count_patches_failures, false);	
	num_patches_fail = 0;
	
	Utils::VisitDirectory(myself_path+PATCHES_PATH, true, false, false, load_patch_file);
	
	if (count_patches_failures && num_patches_fail > 0)
	{
		UPRINTF("The following number of patches failed:%d\n", num_patches_fail);
		exit(-1);
	}
}
bool XVPStart(){
	LPCWSTR my_dll_name = L"xinput1_3.dll";
	DPRINTF("XVPATCHER VERSION " XVPATCHER_VERSION ". Exe base = %p. My Dll base = %p. My dll name: %ls\n", GetModuleHandle(NULL), GetModuleHandleW(my_dll_name), my_dll_name);	
	load_patches();
	return true;
}
static bool in_game_process()
{
	char szPath[MAX_PATH];
	
	GetModuleFileName(NULL, szPath, MAX_PATH);
	_strlwr(szPath);
	
	// A very poor aproach, I know
	return (strstr(szPath, PROCESS_NAME) != NULL);
}

extern "C" BOOL WINAPI EXPORT DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		

			initial_tick = GetTickCount();
			set_debug_level(2); 	

			if (in_game_process())
			{					
				HookExternalAS3Callback();
				CreateMutexA(nullptr, FALSE, "XV2PATCHER_INSTANCE");
				if (GetLastError() == ERROR_ALREADY_EXISTS)
				{
					UPRINTF("An instance of xvpatcher already exists.\n"
							"You probably have mixed xinput1_3 and dinput8!\n");
					exit(-1);
				}
					
				XVPStart();   

				load_dll(false);

				if (!PatchUtils::HookImport("KERNEL32.dll", "GetStartupInfoW", (void *)GetStartupInfoW_Patched))
				{
					UPRINTF("GetStartupInfoW hook failed.\n");
					return TRUE;
				}
			}			
			
		break;
		
		case DLL_PROCESS_DETACH:		
			
			if (!lpvReserved)
				unload_dll();
			
		break;
	}
	
	return TRUE;
}