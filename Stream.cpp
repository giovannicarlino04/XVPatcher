/**
 * Stream.cpp - Consolidated stream implementations for XVPatcher
 * This file combines implementations from:
 * - Stream.cpp
 * - MemoryStream.cpp
 * - FileStream.cpp
 * - FixedMemoryStream.cpp
 * - FixedBitStream.cpp
 * - BitStream.cpp
 * - CrilaylaFixedBitStream.cpp
 */

#include "Stream.h"

#ifndef NO_CRYPTO
#include "crypto/sha1.h"
#endif

#include "debug.h"
#include <algorithm>
#include <cstring>

#define BUFFER_SIZE	(16*1024*1024)

//================================================
// Base Stream Implementation
//================================================

Stream::Stream() : big_endian(false)
{
}

Stream::~Stream()
{
}

uint8_t Stream::Read8()
{
    uint8_t ret = 0;
    Read(&ret, sizeof(ret));
    return ret;
}

uint16_t Stream::Read16()
{
    uint16_t ret = 0;
    Read(&ret, sizeof(ret));
    
    if (big_endian)
    {
        ret = ((ret & 0xFF) << 8) | ((ret >> 8) & 0xFF);
    }
    
    return ret;
}

uint32_t Stream::Read32()
{
    uint32_t ret = 0;
    Read(&ret, sizeof(ret));
    
    if (big_endian)
    {
        ret = ((ret & 0xFF) << 24) | 
              (((ret >> 8) & 0xFF) << 16) | 
              (((ret >> 16) & 0xFF) << 8) | 
              ((ret >> 24) & 0xFF);
    }
    
    return ret;
}

uint64_t Stream::Read64()
{
    uint64_t ret = 0;
    Read(&ret, sizeof(ret));
    
    if (big_endian)
    {
        ret = ((uint64_t)(ret & 0xFF) << 56) |
              ((uint64_t)((ret >> 8) & 0xFF) << 48) |
              ((uint64_t)((ret >> 16) & 0xFF) << 40) |
              ((uint64_t)((ret >> 24) & 0xFF) << 32) |
              ((uint64_t)((ret >> 32) & 0xFF) << 24) |
              ((uint64_t)((ret >> 40) & 0xFF) << 16) |
              ((uint64_t)((ret >> 48) & 0xFF) << 8) |
              ((uint64_t)((ret >> 56) & 0xFF));
    }
    
    return ret;
}

void Stream::Write8(uint8_t val)
{
    Write(&val, sizeof(val));
}

void Stream::Write16(uint16_t val)
{
    if (big_endian)
    {
        val = ((val & 0xFF) << 8) | ((val >> 8) & 0xFF);
    }
    
    Write(&val, sizeof(val));
}

void Stream::Write32(uint32_t val)
{
    if (big_endian)
    {
        val = ((val & 0xFF) << 24) | 
              (((val >> 8) & 0xFF) << 16) | 
              (((val >> 16) & 0xFF) << 8) | 
              ((val >> 24) & 0xFF);
    }
    
    Write(&val, sizeof(val));
}

void Stream::Write64(uint64_t val)
{
    if (big_endian)
    {
        val = ((uint64_t)(val & 0xFF) << 56) |
              ((uint64_t)((val >> 8) & 0xFF) << 48) |
              ((uint64_t)((val >> 16) & 0xFF) << 40) |
              ((uint64_t)((val >> 24) & 0xFF) << 32) |
              ((uint64_t)((val >> 32) & 0xFF) << 24) |
              ((uint64_t)((val >> 40) & 0xFF) << 16) |
              ((uint64_t)((val >> 48) & 0xFF) << 8) |
              ((uint64_t)((val >> 56) & 0xFF));
    }
    
    Write(&val, sizeof(val));
}

float Stream::ReadFloat()
{
    float ret;
    uint32_t data = Read32();
    memcpy(&ret, &data, sizeof(ret));
    return ret;
}

void Stream::WriteFloat(float val)
{
    uint32_t data;
    memcpy(&data, &val, sizeof(data));
    Write32(data);
}

std::string Stream::ReadString(size_t size)
{
    std::string ret;
    
    if (size == 0)
        return ret;
        
    ret.resize(size);
    Read(&ret[0], size);
    
    // Trim any null terminators if present
    size_t pos = ret.find('\0');
    if (pos != std::string::npos)
    {
        ret.resize(pos);
    }
    
    return ret;
}

std::string Stream::ReadCString()
{
    std::string ret;
    char c;
    
    while ((c = Read8()) != 0)
    {
        ret.push_back(c);
    }
    
    return ret;
}

void Stream::WriteString(const std::string &str, bool null_terminated)
{
    if (str.empty())
        return;
        
    Write(str.c_str(), str.length());
    
    if (null_terminated)
        Write8(0);
}

//================================================
// Memory Stream Implementation
//================================================

MemoryStream::MemoryStream() : buf(nullptr), size(0), position(0)
{
}

MemoryStream::~MemoryStream()
{
    if (buf)
        delete[] buf;
}

size_t MemoryStream::Read(void *buffer, size_t read_size)
{
    if (!buf || position >= size)
        return 0;
        
    size_t available = size - position;
    size_t to_read = std::min(read_size, available);
    
    memcpy(buffer, buf + position, to_read);
    position += to_read;
    
    return to_read;
}

size_t MemoryStream::Write(const void *buffer, size_t write_size)
{
    if (position + write_size > size)
    {
        // Expand the buffer
        Resize(position + write_size);
    }
    
    memcpy(buf + position, buffer, write_size);
    position += write_size;
    
    return write_size;
}

size_t MemoryStream::Tell()
{
    return position;
}

size_t MemoryStream::Seek(size_t offset, int whence)
{
    switch (whence)
    {
        case SEEK_SET:
            position = offset;
            break;
            
        case SEEK_CUR:
            position += offset;
            break;
            
        case SEEK_END:
            position = size + offset; // Note: offset is typically negative here
            break;
    }
    
    // Bound checking
    if (position > size)
        position = size;
        
    return position;
}

size_t MemoryStream::GetSize()
{
    return size;
}

size_t MemoryStream::GetAvailable()
{
    return (size > position) ? (size - position) : 0;
}

uint8_t *MemoryStream::GetMemory(bool unlink)
{
    uint8_t *ret = buf;
    
    if (unlink)
        buf = nullptr;
        
    return ret;
}

void MemoryStream::Resize(size_t new_size)
{
    if (new_size == 0)
    {
        if (buf)
        {
            delete[] buf;
            buf = nullptr;
        }
        
        size = 0;
        position = 0;
        return;
    }
    
    uint8_t *new_buf = new uint8_t[new_size];
    
    if (buf)
    {
        size_t to_copy = std::min(size, new_size);
        memcpy(new_buf, buf, to_copy);
        delete[] buf;
    }
    
    buf = new_buf;
    size = new_size;
    
    if (position > size)
        position = size;
}

//================================================
// Fixed Memory Stream Implementation
//================================================

FixedMemoryStream::FixedMemoryStream(void *buffer, size_t buffer_size, bool take_ownership)
    : buf((uint8_t *)buffer), size(buffer_size), position(0), is_owner(take_ownership)
{
}

FixedMemoryStream::~FixedMemoryStream()
{
    if (is_owner && buf)
    {
        delete[] buf;
        buf = nullptr;
    }
}

size_t FixedMemoryStream::Read(void *buffer, size_t read_size)
{
    if (!buf || position >= size)
        return 0;
        
    size_t available = size - position;
    size_t to_read = std::min(read_size, available);
    
    memcpy(buffer, buf + position, to_read);
    position += to_read;
    
    return to_read;
}

size_t FixedMemoryStream::Write(const void *buffer, size_t write_size)
{
    if (!buf || position >= size)
        return 0;
        
    size_t available = size - position;
    size_t to_write = std::min(write_size, available);
    
    memcpy(buf + position, buffer, to_write);
    position += to_write;
    
    return to_write;
}

size_t FixedMemoryStream::Tell()
{
    return position;
}

size_t FixedMemoryStream::Seek(size_t offset, int whence)
{
    switch (whence)
    {
        case SEEK_SET:
            position = offset;
            break;
            
        case SEEK_CUR:
            position += offset;
            break;
            
        case SEEK_END:
            position = size + offset; // Note: offset is typically negative here
            break;
    }
    
    // Bound checking
    if (position > size)
        position = size;
        
    return position;
}

size_t FixedMemoryStream::GetSize()
{
    return size;
}

size_t FixedMemoryStream::GetAvailable()
{
    return (size > position) ? (size - position) : 0;
}

//================================================
// File Stream Implementation
//================================================

FileStream::FileStream() : size(0)
{
}

FileStream::~FileStream()
{
    Close();
}

bool FileStream::Open(const std::string &file_path, bool read_only)
{
    std::ios_base::openmode mode = std::ios::binary;
    
    if (read_only)
        mode |= std::ios::in;
    else
        mode |= std::ios::in | std::ios::out;
    
    file.open(file_path.c_str(), mode);
    
    if (!file.is_open())
        return false;
    
    filename = file_path;
    
    // Get file size
    file.seekg(0, std::ios::end);
    size = (size_t)file.tellg();
    file.seekg(0, std::ios::beg);
    
    return true;
}

void FileStream::Close()
{
    if (file.is_open())
        file.close();
    
    filename.clear();
    size = 0;
}

size_t FileStream::Read(void *buffer, size_t read_size)
{
    if (!file.is_open())
        return 0;
    
    file.read((char *)buffer, read_size);
    return (size_t)file.gcount();
}

size_t FileStream::Write(const void *buffer, size_t write_size)
{
    if (!file.is_open())
        return 0;
    
    file.write((const char *)buffer, write_size);
    
    if (file.tellp() > size)
        size = (size_t)file.tellp();
    
    return write_size;
}

size_t FileStream::Tell()
{
    if (!file.is_open())
        return 0;
    
    return (size_t)file.tellg();
}

size_t FileStream::Seek(size_t offset, int whence)
{
    if (!file.is_open())
        return 0;
    
    std::ios_base::seekdir dir;
    
    switch (whence)
    {
        case SEEK_SET:
            dir = std::ios::beg;
            break;
            
        case SEEK_CUR:
            dir = std::ios::cur;
            break;
            
        case SEEK_END:
            dir = std::ios::end;
            break;
            
        default:
            return Tell();
    }
    
    file.seekg(offset, dir);
    file.seekp(offset, dir);
    
    return (size_t)file.tellg();
}

size_t FileStream::GetSize()
{
    return size;
}

size_t FileStream::GetAvailable()
{
    if (!file.is_open())
        return 0;
    
    size_t pos = (size_t)file.tellg();
    return (size > pos) ? (size - pos) : 0;
}

//================================================
// Bit Stream Implementation
//================================================

BitStream::BitStream(const void *buffer, size_t bit_size)
    : buf((const uint8_t *)buffer), bit_position(0), size_in_bits(bit_size)
{
}

BitStream::~BitStream()
{
}

uint32_t BitStream::GetBits(int num_bits)
{
    if (num_bits <= 0 || num_bits > 32 || bit_position + num_bits > size_in_bits)
        return 0;
    
    uint32_t result = 0;
    size_t byte_pos = bit_position >> 3;
    size_t bit_in_byte = bit_position & 7;
    
    for (int i = 0; i < num_bits; i++)
    {
        if (buf[byte_pos] & (1 << bit_in_byte))
            result |= (1 << i);
            
        bit_in_byte++;
        if (bit_in_byte >= 8)
        {
            byte_pos++;
            bit_in_byte = 0;
        }
    }
    
    bit_position += num_bits;
    return result;
}

void BitStream::Seek(size_t bit_pos)
{
    bit_position = std::min(bit_pos, size_in_bits);
}

//================================================
// Fixed Bit Stream Implementation
//================================================

FixedBitStream::FixedBitStream(const void *buffer, size_t bit_size)
    : buf((const uint8_t *)buffer), bit_position(0), size_in_bits(bit_size),
      size_in_bytes((bit_size + 7) >> 3)
{
}

FixedBitStream::~FixedBitStream()
{
}

uint32_t FixedBitStream::GetBits(int num_bits)
{
    if (num_bits <= 0 || num_bits > 32 || bit_position + num_bits > size_in_bits)
        return 0;
    
    uint32_t result = 0;
    size_t byte_pos = bit_position >> 3;
    size_t bit_in_byte = bit_position & 7;
    
    for (int i = 0; i < num_bits; i++)
    {
        if (buf[byte_pos] & (1 << bit_in_byte))
            result |= (1 << i);
            
        bit_in_byte++;
        if (bit_in_byte >= 8)
        {
            byte_pos++;
            bit_in_byte = 0;
        }
    }
    
    bit_position += num_bits;
    return result;
}

void FixedBitStream::Seek(size_t bit_pos)
{
    bit_position = std::min(bit_pos, size_in_bits);
}

//================================================
// Crilayla Fixed Bit Stream Implementation
//================================================

CrilaylaFixedBitStream::CrilaylaFixedBitStream(const void *buffer, size_t bit_size)
    : FixedBitStream(buffer, bit_size), result_ready(false), result(0)
{
}

CrilaylaFixedBitStream::~CrilaylaFixedBitStream()
{
}

uint32_t CrilaylaFixedBitStream::ReadBits(int num_bits)
{
    if (!result_ready)
        ReadNextBits();
    
    uint32_t ret = result & ((1 << num_bits) - 1);
    
    result >>= num_bits;
    result_ready = (result != 0);
    
    return ret;
}

void CrilaylaFixedBitStream::ReadNextBits()
{
    const size_t byte_pos = bit_position >> 3;
    const size_t bit_in_byte = bit_position & 7;
    
    if (byte_pos >= size_in_bytes)
    {
        result = 0;
        result_ready = true;
        return;
    }
    
    // Read 32 bits or whatever is available
    result = 0;
    size_t bits_read = 0;
    size_t current_byte = byte_pos;
    size_t current_bit = bit_in_byte;
    
    while (bits_read < 32 && current_byte < size_in_bytes)
    {
        if (buf[current_byte] & (1 << current_bit))
            result |= (1 << bits_read);
            
        bits_read++;
        current_bit++;
        
        if (current_bit >= 8)
        {
            current_byte++;
            current_bit = 0;
        }
    }
    
    bit_position += bits_read;
    result_ready = true;
}

bool Stream::Copy(Stream *other, size_t size)
{
    uint8_t *copy_buf;
    size_t remaining;

    if (size <BUFFER_SIZE)
    {
        copy_buf = new uint8_t[size];
    }
    else
    {
        copy_buf = new uint8_t[BUFFER_SIZE];
    }

    remaining = size;

    while (remaining > 0)
    {
        size_t r = (remaining > BUFFER_SIZE) ? BUFFER_SIZE: remaining;

        if (!other->Read(copy_buf, r))
        {
            delete[] copy_buf;
            return false;
        }

        if (!Write(copy_buf, r))
        {
            delete[] copy_buf;
            return false;
        }

        remaining -= r;
    }

    delete[] copy_buf;
    return true;
}

#ifdef NO_CRYPTO

bool Stream::CopyEx(Stream *, size_t, Hash, uint8_t *, Cipher, const uint8_t *, int, Cipher, const uint8_t *, int)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
    return false;
}

#else

bool Stream::CopyEx(Stream *other, size_t size, Hash hash_mode, uint8_t *hash, Cipher decrypt_mode, const uint8_t *decrypt_key, int decrypt_key_size, Cipher encrypt_mode, const uint8_t *encrypt_key, int encrypt_key_size)
{
    static const size_t block_size = 16;
    uint8_t *copy_buf;
    size_t remaining;
    SHA1_CTX ctx;
    bool decrypt, encrypt, do_hash;

    //assert((BUFFER_SIZE % block_size) == 0);

    do_hash = (hash_mode == Hash::SHA1);
    decrypt = (decrypt_mode == Cipher::AES_PLAIN);
    encrypt = (encrypt_mode == Cipher::AES_PLAIN);

    if (size < BUFFER_SIZE)
    {
        size_t buf_size = size;

        if (decrypt || encrypt)
        {
            if ((buf_size % block_size) != 0)
            {
                buf_size += (block_size - (buf_size % block_size));
            }
        }

        copy_buf = new uint8_t[buf_size];
    }
    else
    {
        copy_buf = new uint8_t[BUFFER_SIZE];
    }   

    if (do_hash)
        __SHA1_Init(&ctx);

    remaining = size;

    while (remaining > 0)
    {
        size_t r = (remaining > BUFFER_SIZE) ? BUFFER_SIZE: remaining;
        size_t read_size = r;
        size_t write_size = r;
        bool eof = false;

        if (decrypt)
        {
            if ((read_size % block_size) != 0)
            {
                read_size += (block_size - (r % block_size));
                eof = true;
            }
        }

        if (!other->Read(copy_buf, read_size))
        {
            delete[] copy_buf;
            return false;
        }

        if (do_hash)
            __SHA1_Update(&ctx, copy_buf, (uint32_t)r);

        if (decrypt)
        {
        }

        if (encrypt)
        {
            if ((write_size % block_size) != 0)
            {
                size_t new_write_size = write_size + (block_size - (r % block_size));

                write_size = new_write_size;
            }

        }

        if (!Write(copy_buf, write_size))
        {
            delete[] copy_buf;
            return false;
        }

        if (eof)
            break;

        remaining -= r;
    }

    if (do_hash)
        __SHA1_Final(&ctx, hash);

    delete[] copy_buf;
    return true;
}

#endif // NO_CRYPTO

bool Stream::Align(unsigned int alignment)
{
    unsigned int write_size = 0;
    uint64_t file_pos = Tell();

    if ((file_pos % alignment) != 0)
        write_size = (alignment - (file_pos % alignment));

    for (unsigned int i = 0; i < write_size; i++)
    {
        if (!Write8(0))
            return false;
    }

    return true;
}

