#ifndef PATCHES_H
#define PATCHES_H

#include <windows.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Typedefs per le funzioni originali
typedef uint64_t (*LoadTableType)(void *, void *, uint32_t);
typedef int (*CpkGetFileInfoType)(uint8_t *, const char *, uint8_t *, uint32_t *);
typedef int (*CpkMovieFileType)(void *, void *, const char *);
typedef int (*CpkLoggerType)(int32_t, const char*);

// Variabili globali per le funzioni originali
extern LoadTableType orig_LoadTable;
extern CpkGetFileInfoType orig_CpkGetFileInfo;
extern CpkMovieFileType orig_CpkMovieFile;
extern CpkLoggerType orig_cpkLogger;

// Variabili di configurazione
extern bool log_all_files;
extern bool log_loose_files;

// Funzioni di setup e patch
void LoadToc_Setup(LoadTableType orig);
uint64_t LoadToc(void *unk, void *buf, uint32_t size);
void CpkGetFileInfo_Setup(CpkGetFileInfoType orig);
int CpkGetFileInfo_Patched(uint8_t *cpk_object, const char *path, uint8_t *ret_entry, uint32_t *result);
void SetupLogMovieFiles(CpkMovieFileType orig);
int HookMovieLoad(void *pthis, void *unk, const char *path);
void* CpkLoggerPatched(int32_t arg1, const char* str);

#ifdef __cplusplus
}
#endif

#endif // PATCHES_H
