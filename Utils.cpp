/**
 * Utils.cpp - Consolidated utilities for XVPatcher
 * This file combines functionality from:
 * - UtilsFS.cpp
 * - UtilsStr.cpp
 * - UtilsMisc.cpp
 * - UtilsXML.cpp
 * - UtilsZlib.cpp
 * - UtilsCrypto.cpp
 */

#include "Utils.h"

// Include necessary implementation includes
#include "debug.h"
#include <windows.h>
#include <vector>
#include <algorithm>
#include <string>
#include <codecvt>
#include <locale>
#include <fstream>
#include <sstream>
#include <ctime>
#include <stdint.h>
#include <direct.h>
#include <sys/stat.h>
#include <io.h>

// Include TinyXML for XML functions
#include "tinyxml/tinyxml.h"

// The implementation of each Utils category will be in separate namespaces
// in this file, but will be accessible via the Utils:: namespace as before

//================================================
// UtilsFS implementation
//================================================

namespace Utils
{
    // File system related functions

    bool FileExists(const std::string &path)
    {
        DWORD attr = GetFileAttributesA(path.c_str());
        return ((attr != INVALID_FILE_ATTRIBUTES) && !(attr & FILE_ATTRIBUTE_DIRECTORY));
    }

    bool DirExists(const std::string &path)
    {
        DWORD attr = GetFileAttributesA(path.c_str());
        return ((attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY));
    }

    bool MkdirRecursive(const std::string &path)
    {
        size_t pos = 0;
        std::string dir;
        std::string sep = "\\";
        bool success = true;
        
        if (path.empty())
            return false;
            
        if (path.back() == '\\')
            dir = path;
        else
            dir = path + "\\";
        
        if (dir.find(':') != std::string::npos)
        {
            pos = dir.find(':', 0);
            pos++;
        }
        
        while (success && pos != std::string::npos)
        {
            pos = dir.find(sep, pos+1);
            std::string subdir = dir.substr(0, pos);
            
            if (!DirExists(subdir))
            {
                success = (CreateDirectoryA(subdir.c_str(), nullptr) == TRUE);
            }
        }
        
        return success;
    }

    bool ReadTextFile(const std::string &path, std::string &str, bool show_error)
    {
        std::ifstream file(path.c_str(), std::ios::binary);
        
        if (!file.is_open())
        {
            if (show_error)
                DPRINTF("Failed to open %s\n", path.c_str());
                
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        str = buffer.str();
        
        return true;
    }

    bool WriteTextFile(const std::string &path, const std::string &str)
    {
        std::ofstream file(path.c_str(), std::ios::binary);
        
        if (!file.is_open())
        {
            DPRINTF("Failed to write %s\n", path.c_str());
            return false;
        }
        
        file << str;
        return true;
    }

    bool GetFileDate(const std::string &path, time_t *mtime)
    {
        struct stat st;
        
        if (stat(path.c_str(), &st) != 0)
            return false;
            
        *mtime = st.st_mtime;
        return true;
    }

    std::string GetFileNameFromPath(const std::string &path)
    {
        size_t pos = path.find_last_of("\\/");
        
        if (pos == std::string::npos)
            return path;
            
        return path.substr(pos + 1);
    }

    std::string GetDirNameFromPath(const std::string &path)
    {
        size_t pos = path.find_last_of("\\/");
        
        if (pos == std::string::npos)
            return "";
            
        return path.substr(0, pos);
    }

    std::string GetExtension(const std::string &path)
    {
        size_t pos = path.find_last_of('.');
        
        if (pos == std::string::npos)
            return "";
            
        return path.substr(pos);
    }

    std::string GetBaseName(const std::string &path)
    {
        std::string file = GetFileNameFromPath(path);
        size_t pos = file.find_last_of('.');
        
        if (pos == std::string::npos)
            return file;
            
        return file.substr(0, pos);
    }

    // Additional UtilsFS functions would go here
}

//================================================
// UtilsStr implementation
//================================================

namespace Utils
{
    // String related functions

    std::string ToLowerCase(const std::string &str)
    {
        std::string ret = str;
        std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
        return ret;
    }

    std::string ToUpperCase(const std::string &str)
    {
        std::string ret = str;
        std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
        return ret;
    }

    std::string TrimString(const std::string &str)
    {
        size_t first = str.find_first_not_of(" \t\n\r\f\v");
        size_t last = str.find_last_not_of(" \t\n\r\f\v");
        
        if (first == std::string::npos)
            return "";
            
        return str.substr(first, (last - first + 1));
    }

    bool BeginsWith(const std::string &str, const std::string &substr)
    {
        if (substr.length() > str.length())
            return false;
            
        return (str.substr(0, substr.length()) == substr);
    }

    bool EndsWith(const std::string &str, const std::string &substr)
    {
        if (substr.length() > str.length())
            return false;
            
        return (str.substr(str.length() - substr.length()) == substr);
    }

    std::vector<std::string> TokenizeString(const std::string &str, const std::string &delimiters)
    {
        std::vector<std::string> tokens;
        size_t start = 0, end = 0;
        
        while ((end = str.find_first_of(delimiters, start)) != std::string::npos)
        {
            if (end != start)
                tokens.push_back(str.substr(start, end - start));
                
            start = end + 1;
        }
        
        if (start < str.length())
            tokens.push_back(str.substr(start));
            
        return tokens;
    }

    std::wstring Utf8ToWide(const std::string &utf8str)
    {
        if (utf8str.empty())
            return L"";
            
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, nullptr, 0);
        std::wstring wstr(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8str.c_str(), -1, &wstr[0], size_needed);
        wstr.resize(size_needed - 1);  // Remove the null terminator
        
        return wstr;
    }

    std::string WideToUtf8(const std::wstring &wstr)
    {
        if (wstr.empty())
            return "";
            
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string str(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, nullptr, nullptr);
        str.resize(size_needed - 1);  // Remove the null terminator
        
        return str;
    }

    std::string Bcd2String(const uint8_t *bcd, size_t size)
    {
        std::string ret;
        ret.reserve(size * 2);
        
        for (size_t i = 0; i < size; i++)
        {
            char ch = '0' + ((bcd[i] >> 4) & 0xF);
            ret.push_back(ch);
            
            ch = '0' + (bcd[i] & 0xF);
            ret.push_back(ch);
        }
        
        return ret;
    }

    // Additional UtilsStr functions would go here
}

//================================================
// UtilsMisc implementation
//================================================

namespace Utils
{
    // Miscellaneous utility functions
    
    uint32_t GetTickCount()
    {
        return ::GetTickCount();
    }
    
    uint32_t GetRandomInt()
    {
        static bool initialized = false;
        
        if (!initialized)
        {
            srand((unsigned int)time(nullptr));
            initialized = true;
        }
        
        return rand();
    }

    // Additional UtilsMisc functions would go here
}

//================================================
// UtilsZlib implementation
//================================================

namespace Utils
{
    // Zlib compression/decompression functions
    // (Simplified here since NO_ZLIB is defined)

    bool DecompressZlib(uint8_t *in, size_t in_size, uint8_t *out, size_t out_size)
    {
        // This is a placeholder - with NO_ZLIB defined, we'd need a custom implementation
        // or just return false to indicate not supported
        DPRINTF("Zlib decompression not supported in this build (NO_ZLIB defined).\n");
        return false;
    }

    // Additional UtilsZlib functions would go here
}

//================================================
// UtilsCrypto implementation
//================================================

namespace Utils
{
    // Crypto related functions
    
    // Basic crypto implementations
    void XorData(unsigned char *buf, size_t size, uint8_t key)
    {
        for (size_t i = 0; i < size; i++)
            buf[i] ^= key;
    }
    
    // Additional UtilsCrypto functions would go here
}

//================================================
// UtilsXML implementation
//================================================

namespace Utils
{
    // XML-related functions would go here
    // These would call into tinyxml functions as needed
    // Basic implementations shown here
    
    bool ReadXmlFile(const std::string &path, TiXmlDocument &doc)
    {
        if (!doc.LoadFile(path))
        {
            DPRINTF("Failed to load XML file %s. Error: %s\n", path.c_str(), doc.ErrorDesc());
            return false;
        }
        
        return true;
    }
    
    bool SaveXmlFile(const std::string &path, const TiXmlDocument &doc)
    {
        if (!doc.SaveFile(path))
        {
            DPRINTF("Failed to save XML file %s\n", path.c_str());
            return false;
        }
        
        return true;
    }
    
    // Additional UtilsXML functions would go here
} 