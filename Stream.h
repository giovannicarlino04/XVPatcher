#ifndef __STREAM_H__
#define __STREAM_H__

#include "BaseFile.h"
typedef __int64 off64_t; // For Windows

class Stream : public BaseFile
{
public:

    enum class Hash
    {
        NONE,
        SHA1
    };

    enum class Cipher
    {
        NONE,
        AES_PLAIN,
    };

    virtual uint64_t GetSize() const = 0;
    virtual bool Resize(uint64_t size) = 0;
    virtual bool Grow(int64_t size) { return Resize(GetSize()+size); }

    virtual bool Read(void *buf, size_t size) = 0;
    virtual bool Write(const void *buf, size_t size) = 0;

	inline bool Read8(uint8_t *ptr)	{ return Read(ptr, sizeof(uint8_t)); }
    inline bool Read8(int8_t *ptr) { return Read8((uint8_t *)ptr); }

    inline bool Read16(uint16_t *ptr)
    {
        bool ret = Read(ptr, sizeof(uint16_t));
        if (ret)
            *ptr = val16(*ptr);

        return ret;
    }

    inline bool Read16(int16_t *ptr) { return Read16((uint16_t *)ptr); }

    inline bool Read32(uint32_t *ptr)
    {
        bool ret = Read(ptr, sizeof(uint32_t));
        if (ret)
            *ptr = val32(*ptr);

        return ret;
    }

    inline bool Read32(int32_t *ptr) { return Read32((uint32_t *)ptr); }

    inline bool Read64(uint64_t *ptr)
    {
        bool ret = Read(ptr, sizeof(uint64_t));
        if (ret)
            *ptr = val64(*ptr);

        return ret;
    }

    inline bool Read64(int64_t *ptr) { return Read64((uint64_t *)ptr); }

	inline bool ReadGuid(uint8_t *ptr) { return Read(ptr, 16); }

    inline bool Write8(uint8_t value)	{ return Write(&value, sizeof(uint8_t)); }

    inline bool Write16(uint16_t value)
    {
        value = val16(value);
        return Write(&value, sizeof(uint16_t));
    }

    inline bool Write32(uint32_t value)
    {
        value = val32(value);
        return Write(&value, sizeof(uint32_t));
    }

    inline bool Write64(uint64_t value)
    {
        value = val64(value);
        return Write(&value, sizeof(uint64_t));
    }

	inline bool WriteGuid(const uint8_t *ptr)	{ return Write(ptr, 16); }

    virtual bool Seek(off64_t offset, int whence) = 0;
    virtual uint64_t Tell() = 0;

    virtual bool Copy(Stream *other, size_t size);
    virtual bool CopyEx(Stream *other, size_t size, Hash hash_mode=Hash::NONE, uint8_t *hash=nullptr, Cipher decrypt_mode=Cipher::NONE, const uint8_t *decrypt_key=nullptr, int decrypt_key_size=0, Cipher encrypt_mode=Cipher::NONE, const uint8_t *encrypt_key=nullptr, int encrypt_key_size=0);
    virtual bool Align(unsigned int alignment);
};

#endif /* __STREAM_H__ */
