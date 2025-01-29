#ifndef CPKDEF_H
#define CPKDEF_H

#include <windows.h>
#include <cstdint>
#include "CpkFile.h"

extern uint8_t (* __thiscall cpk_file_exists)(void *, char *);

extern uint64_t data_toc_offset, data_toc_size;
extern uint64_t data2_toc_offset, data2_toc_size;
extern uint64_t datap1_toc_offset, datap1_toc_size;
extern uint64_t datap2_toc_offset, datap2_toc_size;
extern uint64_t datap3_toc_offset, datap3_toc_size;

extern uint8_t *data_toc, *data2_toc, *datap1_toc, *datap2_toc, *datap3_toc;
extern uint8_t *data_hdr, *data2_hdr, *datap1_hdr, *datap2_hdr, *datap3_hdr;
extern void **readfile_import;
extern void *original_readfile;

uint8_t *read_file_from(const char *file, uint64_t offset, unsigned int *psize);
CpkFile *get_cpk_toc(const char *file, uint64_t *toc_offset, uint64_t *toc_size, uint8_t **hdr_buf, uint8_t **toc_buf);
bool get_cpk_tocs(CpkFile **data, CpkFile **data2, CpkFile **datap1, CpkFile **datap2, CpkFile **datap3);
bool local_file_exists(char *path);
bool local_file_exists(FileEntry *entry);
void patch_toc(CpkFile *cpk);
bool IsThisFile(HANDLE hFile, const char *name);
uint64_t GetFilePointer(HANDLE hFile);
BOOL WINAPI ReadFile_patched(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
uint8_t __thiscall cpk_file_exists_patched(void *object, char *file);
void patches();

#endif // CPKDEF_H
