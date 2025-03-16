/**
 * Stream.h - Consolidated stream classes for XVPatcher
 * This file combines functionality from:
 * - Stream.h
 * - MemoryStream.h
 * - FileStream.h
 * - FixedMemoryStream.h
 * - FixedBitStream.h
 * - BitStream.h
 * - CrilaylaFixedBitStream.h
 */

#ifndef STREAM_H
#define STREAM_H

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <memory>

// For SEEK_* constants and compatibility
#include <stdio.h>

// Visual C++ compatibility
#ifdef _MSC_VER
    // Ensure __declspec(dllexport) is defined properly
    #ifdef BUILDING_DLL
        #define STREAM_API __declspec(dllexport)
    #else
        #define STREAM_API __declspec(dllimport)
    #endif
#else
    #define STREAM_API
#endif

//================================================
// Base Stream Class
//================================================

class STREAM_API Stream
{
protected:
    bool big_endian;
    
public:
    Stream();
    virtual ~Stream();
    
    virtual size_t Read(void *buf, size_t size) = 0;
    virtual size_t Write(const void *buf, size_t size) = 0;
    virtual size_t Tell() = 0;
    virtual size_t Seek(size_t offset, int whence) = 0;
    virtual size_t GetSize() = 0;
    virtual size_t GetAvailable() = 0;
    
    bool IsBigEndian() const { return big_endian; }
    void SetEndianness(bool big) { big_endian = big; }
    
    uint8_t Read8();
    uint16_t Read16();
    uint32_t Read32();
    uint64_t Read64();
    
    void Write8(uint8_t val);
    void Write16(uint16_t val);
    void Write32(uint32_t val);
    void Write64(uint64_t val);
    
    float ReadFloat();
    void WriteFloat(float val);
    
    std::string ReadString(size_t size);
    std::string ReadCString();
    void WriteString(const std::string &str, bool null_terminated = false);
};

//================================================
// Memory Stream
//================================================

class STREAM_API MemoryStream : public Stream
{
protected:
    uint8_t *buf;
    size_t size;
    size_t position;
    
public:
    MemoryStream();
    virtual ~MemoryStream();
    
    virtual size_t Read(void *buf, size_t size) override;
    virtual size_t Write(const void *buf, size_t size) override;
    virtual size_t Tell() override;
    virtual size_t Seek(size_t offset, int whence) override;
    virtual size_t GetSize() override;
    virtual size_t GetAvailable() override;
    
    uint8_t *GetMemory(bool unlink = false);
    void Resize(size_t new_size);
};

//================================================
// Fixed Memory Stream
//================================================

class STREAM_API FixedMemoryStream : public Stream
{
protected:
    uint8_t *buf;
    size_t size;
    size_t position;
    bool is_owner;
    
public:
    FixedMemoryStream(void *buf, size_t size, bool take_ownership = false);
    virtual ~FixedMemoryStream();
    
    virtual size_t Read(void *buf, size_t size) override;
    virtual size_t Write(const void *buf, size_t size) override;
    virtual size_t Tell() override;
    virtual size_t Seek(size_t offset, int whence) override;
    virtual size_t GetSize() override;
    virtual size_t GetAvailable() override;
    
    const uint8_t *GetMemory() const { return buf; }
};

//================================================
// File Stream
//================================================

class STREAM_API FileStream : public Stream
{
protected:
    std::fstream file;
    std::string filename;
    size_t size;
    
public:
    FileStream();
    ~FileStream();
    
    bool Open(const std::string &filename, bool read_only = false);
    void Close();
    
    virtual size_t Read(void *buf, size_t size) override;
    virtual size_t Write(const void *buf, size_t size) override;
    virtual size_t Tell() override;
    virtual size_t Seek(size_t offset, int whence) override;
    virtual size_t GetSize() override;
    virtual size_t GetAvailable() override;
    
    const std::string &GetFileName() const { return filename; }
};

//================================================
// Bit Stream
//================================================

class STREAM_API BitStream
{
protected:
    const uint8_t *buf;
    size_t bit_position;
    size_t size_in_bits;
    
public:
    BitStream(const void *buf, size_t size_in_bits);
    virtual ~BitStream();
    
    uint32_t GetBits(int num_bits);
    size_t Tell() const { return bit_position; }
    void Seek(size_t bit_pos);
    size_t GetSize() const { return size_in_bits; }
};

//================================================
// Fixed Bit Stream
//================================================

class STREAM_API FixedBitStream
{
protected:
    const uint8_t *buf;
    size_t bit_position;
    size_t size_in_bits;
    size_t size_in_bytes;
    
public:
    FixedBitStream(const void *buf, size_t size_in_bits);
    virtual ~FixedBitStream();
    
    uint32_t GetBits(int num_bits);
    size_t Tell() const { return bit_position; }
    void Seek(size_t bit_pos);
    size_t GetSize() const { return size_in_bits; }
};

//================================================
// Crilayla Fixed Bit Stream
//================================================

class STREAM_API CrilaylaFixedBitStream : public FixedBitStream
{
private:
    bool result_ready;
    uint32_t result;
    
public:
    CrilaylaFixedBitStream(const void *buf, size_t size_in_bits);
    ~CrilaylaFixedBitStream();
    
    uint32_t ReadBits(int num_bits);
    void ReadNextBits();
};

#endif // STREAM_H
