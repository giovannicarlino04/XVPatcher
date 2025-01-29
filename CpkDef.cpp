#include "CpkDef.h"
#include <cstdio>
#include <cstring>
#include "debug.h"
#include "patch.h"
#include "symbols.h"
#include "xvpatcher.h"

uint8_t (* __thiscall cpk_file_exists)(void *, char *);

uint64_t data_toc_offset, data_toc_size;
uint64_t data2_toc_offset, data2_toc_size;
uint64_t datap1_toc_offset, datap1_toc_size;
uint64_t datap2_toc_offset, datap2_toc_size;
uint64_t datap3_toc_offset, datap3_toc_size;

uint8_t *data_toc, *data2_toc, *datap1_toc, *datap2_toc, *datap3_toc;
uint8_t *data_hdr, *data2_hdr, *datap1_hdr, *datap2_hdr, *datap3_hdr;
void **readfile_import;
void *original_readfile;

uint8_t *read_file_from(const char *file, uint64_t offset, unsigned int *psize)
{
    HANDLE hFile;
    LONG high;
    uint8_t *buf;

    hFile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return NULL;

    high = (offset >> 32);
    if (SetFilePointer(hFile, offset & 0xFFFFFFFF, &high, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
    {
        CloseHandle(hFile);
        return NULL;
    }

    buf = new uint8_t[*psize];
    if (!buf)
    {
        CloseHandle(hFile);
        return NULL;
    }

    if (!ReadFile(hFile, buf, *psize, (LPDWORD)psize, NULL))
    {
        delete[] buf;
        CloseHandle(hFile);
        return NULL;
    }

    CloseHandle(hFile);
    return buf;
}
static bool get_cpk_tocs(CpkFile **data, CpkFile **data2, CpkFile **datap1, CpkFile **datap2, CpkFile **datap3)
{
	*data = get_cpk_toc(DATA_CPK, &data_toc_offset, &data_toc_size, &data_hdr, &data_toc);
	if (!(*data))
		return false;	
	
	*data2 = get_cpk_toc(DATA2_CPK, &data2_toc_offset, &data2_toc_size, &data2_hdr, &data2_toc);
	if (!(*data2))
	{
		delete[] data_toc;
		delete *data;
		return false;
	}
	*datap1 = get_cpk_toc(DATAP1_CPK, &datap1_toc_offset, &datap1_toc_size, &datap1_hdr, &datap1_toc);
	if (!(*datap1))
	{
		delete[] data2_toc;
		delete *data2;
		return false;
	}
	*datap2 = get_cpk_toc(DATAP2_CPK, &datap2_toc_offset, &datap2_toc_size, &datap2_hdr, &datap2_toc);
	if (!(*datap2))
	{
		delete[] datap1_toc;
		delete *datap1;
		return false;
	}
	*datap3 = get_cpk_toc(DATAP3_CPK, &datap3_toc_offset, &datap3_toc_size, &datap3_hdr, &datap3_toc);
	if (!(*datap3))
	{
		delete[] datap2_toc;
		delete *datap2;
		return false;
	}

	DPRINTF("data.cpk.toc = %I64x, size = %I64x\n", data_toc_offset, data_toc_size);
	DPRINTF("data2.cpk.toc = %I64x, size = %I64x\n", data2_toc_offset, data2_toc_size);
	DPRINTF("datap1.cpk.toc = %I64x, size = %I64x\n", datap1_toc_offset, datap1_toc_size);
	DPRINTF("datap2.cpk.toc = %I64x, size = %I64x\n", datap2_toc_offset, datap2_toc_size);
	DPRINTF("datap3.cpk.toc = %I64x, size = %I64x\n", datap3_toc_offset, datap3_toc_size);

	
	return true;
}


CpkFile *get_cpk_toc(const char *file, uint64_t *toc_offset, uint64_t *toc_size, uint8_t **hdr_buf, uint8_t **toc_buf)
{
    unsigned rsize;
    bool success = false;

    *hdr_buf = NULL;
    *toc_buf = NULL;

    rsize = 0x800;
    *hdr_buf = read_file_from(file, 0, &rsize);
    if (!(*hdr_buf))
        return NULL;

    CpkFile *cpk = new CpkFile();

    if (!cpk->ParseHeaderData(*hdr_buf))
        goto clean;

    *toc_offset = cpk->GetTocOffset();
    *toc_size = cpk->GetTocSize();

    if (*toc_offset == (uint64_t)-1 || *toc_size == 0)
        goto clean;

    rsize = *toc_size;
    *toc_buf = read_file_from(file, *toc_offset, &rsize);

    if (!(*toc_buf))
    {
        delete[] *toc_buf;
        goto clean;
    }

    if (!cpk->ParseTocData(*toc_buf))
        goto clean;

    success = true;

clean:
    if (!success)
    {
        delete[] *hdr_buf;
        delete[] *toc_buf;
        delete cpk;
        cpk = NULL;
    }

    return cpk;
}

static bool local_file_exists( char *path)
{
	HANDLE hFind;
	WIN32_FIND_DATA wfd;
	
	hFind = FindFirstFile(path, &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
		return false;
	
	FindClose(hFind);	
	return true;
}

static bool local_file_exists(FileEntry *entry)
{
	char *path;
		
	path = new char[strlen(entry->dir_name) + strlen(entry->file_name) + 2];
	if (!path)
		return false;
	
	sprintf(path, "%s/%s", entry->dir_name, entry->file_name); // Should be safe... should...
	
	bool ret = local_file_exists(path);
	delete[] path;
	
	return ret;
}
bool IsThisFile(HANDLE hFile, const char *name)
{
    static char path[MAX_PATH + 1];
    memset(path, 0, sizeof(path));

    if (GetFinalPathNameByHandle(hFile, path, sizeof(path) - 1, FILE_NAME_NORMALIZED) != 0)
    {
        _strlwr(path);
        return (strstr(path, name) != NULL);
    }

    return false;
}

uint64_t GetFilePointer(HANDLE hFile)
{
    LONG high = 0;
    DWORD ret = SetFilePointer(hFile, 0, &high, FILE_CURRENT);

    if (ret == INVALID_SET_FILE_POINTER)
        return (uint64_t)-1;

    return (((uint64_t)high << 32) | (uint64_t)ret);
}

BOOL WINAPI ReadFile_patched(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}

uint8_t __thiscall cpk_file_exists_patched(void *object, char *file)
{
    uint8_t ret = cpk_file_exists(object, file);

    if (ret == 0)
    {
        return local_file_exists(file);
    }

    return ret;
}
static void patch_toc(CpkFile *cpk)
{
	int count = 0;
	size_t num_files = cpk->GetNumFiles();
	
	for (size_t i = 0; i < num_files; i++)
	{
		FileEntry *file = cpk->GetFileAt(i);
		
		if (local_file_exists(file))
		{
			cpk->UnlinkFileFromDirectory(i);
			count++;
		}
	}
	
	DPRINTF("%d files deleted in RAM.\n", count);
}
void patches()
{
    readfile_import = (void **)GetModuleImport(GetModuleHandle(NULL), "KERNEL32.dll", "ReadFile");
    if (!readfile_import)
    {
        return;
    }

    original_readfile = *readfile_import;
    WriteMemory32((void *)readfile_import, (uint32_t)ReadFile_patched);

    HookFunction(CPK_FILE_EXISTS_SYMBOL, (void **)&cpk_file_exists, (void *)cpk_file_exists_patched);
}
