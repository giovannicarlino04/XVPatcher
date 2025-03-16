/**
 * Utils.h - Consolidated utilities header for XVPatcher
 * This file combines functionality from:
 * - UtilsFS.h
 * - UtilsStr.h
 * - UtilsMisc.h
 * - UtilsXML.h
 * - UtilsZlib.h
 * - UtilsCrypto.h
 */

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <stdint.h>
#include <ctime>

// Visual C++ compatibility macros
#ifdef _MSC_VER
    #define FUNCNAME    __FUNCSIG__
    #ifndef snprintf
        #define snprintf _snprintf
    #endif
    #ifndef strncasecmp
        #define strncasecmp _strnicmp
    #endif
    #ifndef strcasecmp
        #define strcasecmp _stricmp
    #endif
    // For file offsets
    #define fseeko _fseeki64
    #define ftello _ftelli64
    #define off64_t int64_t
#else
    #define FUNCNAME    __PRETTY_FUNCTION__
#endif

// Forward declare TinyXML classes
class TiXmlDocument;
class TiXmlElement;

namespace Utils
{
    //================================================
    // File System Functions (from UtilsFS)
    //================================================
    
    bool FileExists(const std::string &path);
    bool DirExists(const std::string &path);
    bool MkdirRecursive(const std::string &path);
    bool ReadTextFile(const std::string &path, std::string &str, bool show_error = true);
    bool WriteTextFile(const std::string &path, const std::string &str);
    bool GetFileDate(const std::string &path, time_t *mtime);
    std::string GetFileNameFromPath(const std::string &path);
    std::string GetDirNameFromPath(const std::string &path);
    std::string GetExtension(const std::string &path);
    std::string GetBaseName(const std::string &path);
    
    //================================================
    // String Functions (from UtilsStr)
    //================================================
    
    std::string ToLowerCase(const std::string &str);
    std::string ToUpperCase(const std::string &str);
    std::string TrimString(const std::string &str);
    bool BeginsWith(const std::string &str, const std::string &substr);
    bool EndsWith(const std::string &str, const std::string &substr);
    std::vector<std::string> TokenizeString(const std::string &str, const std::string &delimiters);
    std::wstring Utf8ToWide(const std::string &utf8str);
    std::string WideToUtf8(const std::wstring &wstr);
    std::string Bcd2String(const uint8_t *bcd, size_t size);
    
    //================================================
    // Miscellaneous Functions (from UtilsMisc)
    //================================================
    
    uint32_t GetTickCount();
    uint32_t GetRandomInt();
    
    //================================================
    // Zlib Functions (from UtilsZlib)
    //================================================
    
    bool DecompressZlib(uint8_t *in, size_t in_size, uint8_t *out, size_t out_size);
    
    //================================================
    // Crypto Functions (from UtilsCrypto)
    //================================================
    
    void XorData(unsigned char *buf, size_t size, uint8_t key);
    
    //================================================
    // XML Functions (from UtilsXML)
    //================================================
    
    bool ReadXmlFile(const std::string &path, TiXmlDocument &doc);
    bool SaveXmlFile(const std::string &path, const TiXmlDocument &doc);
}

#endif // UTILS_H
