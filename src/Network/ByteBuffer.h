#pragma once

//#include <boost/endian/conversion.hpp>
#include <cstdint>
#include <vector>
#include <stdio.h>
#include <cstring>
#include <cmath>

class ByteBufferReadException : public std::exception
{
    virtual const char* what() const throw()
    {
        return "Tried to read more data than available in ByteBuffer";
    }
};

class ByteBufferConvertException : public std::exception
{
    virtual const char* what() const throw()
    {
        return "Floating point error";
    }
};

/**
 * @brief Stores raw data
 *
 * Allows of adding specific data types by bytebufferObject << data;
 *
 * Also allows reading via byteBufferObject >> storingVariable;
 *
 * A ByteBuffer needs to be written in the same sequence as it's read.
 *
 * @remark Currently doesn't care for endianess
 */
class ByteBuffer
{
    /// Vector of bytes
    std::vector<uint8_t> mBuffer;

    /// Current position for writing (current size)
    size_t currentPosition = 0;

    /// Current position when reading
    size_t currentReadPosition = 0;

    /**
     * Appends a templated value
     * @param value object to be appended/written
     */
    template <typename T>
    void append(T value)
    {
        static_assert(std::is_fundamental<T>::value, "ByteBuffer: passed value is not a fundamental type");
        //boost::endian::native_to_big(value);
        append((uint8_t*)&value, sizeof(value));
    }

    /**
     * Appends data
     * @param source templated source object
     * @param count array-size of T
     */
    template<class T>
    void append(const T* source, size_t count)
    {
        return append((const uint8_t*)source, count * sizeof(T));
    }

    /**
     * Directly appends bytes
     * @param source (array) of bytes
     * @param count of number of bytes in the array
     */
    void append(const uint8_t* source, size_t count)
    {
        if (mBuffer.size() < currentPosition + count)
            mBuffer.resize(currentPosition + count);

        std::memcpy(&mBuffer[currentPosition], source, count);
        currentPosition += count;
    }

    template <typename T>
    T read()
    {
        T r = read<T>(currentReadPosition);
        currentReadPosition += sizeof(T);
        return r;
    }

    template <typename T>
    T read(size_t position) const
    {
        if (position + sizeof(T) > mBuffer.size())
            throw ByteBufferReadException();

        T value = *((T const*)&mBuffer[position]);

        //boost::endian::big_to_native_inplace(value);
        return value;
    }

public:
    ByteBuffer()
    {
        mBuffer.reserve(2048);
    }

    /**
     * Moves vector
     * @param bytes vector to move to this object
     */
    ByteBuffer(std::vector<uint8_t> bytes) : mBuffer(std::move(bytes))
    {

    }

    /**
     * Moved a vector, but only the items specified with the iterates
     * @param begin iterator of the first element to move
     * @param end iterator of the last element to move
     */
    ByteBuffer(std::vector<uint8_t>::iterator begin, std::vector<uint8_t>::iterator end)
    {
        mBuffer.insert(mBuffer.end(), make_move_iterator(begin), make_move_iterator(end));
    }

    /**
     * @return size of buffer
     */
    size_t getSize()
    {
        return mBuffer.size();
    }

    /**
     * @return raw bytes of the buffer
     */
    uint8_t* getRawData()
    {
        return mBuffer.data();
    }

    /*****************************************/
    /**** Write Functions                 ****/
    /*****************************************/

    ByteBuffer &operator<<(uint8_t value)
    {
        append<uint8_t>(value);
        return *this;
    }

    ByteBuffer &operator<<(uint16_t value)
    {
        append<uint16_t>(value);
        return *this;
    }

    ByteBuffer &operator<<(uint32_t value)
    {
        append<uint32_t>(value);
        return *this;
    }

    ByteBuffer &operator<<(uint64_t value)
    {
        append<uint64_t>(value);
        return *this;
    }

    // signed as in 2e complement
    ByteBuffer &operator<<(int8_t value)
    {
        append<int8_t>(value);
        return *this;
    }

    ByteBuffer &operator<<(int16_t value)
    {
        append<int16_t>(value);
        return *this;
    }

    ByteBuffer &operator<<(int32_t value)
    {
        append<int32_t>(value);
        return *this;
    }

    ByteBuffer &operator<<(int64_t value)
    {
        append<int64_t>(value);
        return *this;
    }


    static_assert(sizeof(float) == sizeof(uint32_t), "ByteBuffer: float doesn't match uint32_t");
    /**
     *
     * @param value
     * @return
     */
    ByteBuffer &operator<<(float value)
    {
        uint32_t number;
        std::memcpy(&number, &value, sizeof(value));

        append<uint32_t>(number);
        return *this;
    }

    static_assert(sizeof(double) == sizeof(uint64_t), "ByteBuffer: double doesn't match uint64_t");

    ByteBuffer &operator<<(double value)
    {
        uint64_t number;
        std::memcpy(&number, &value, sizeof(value));

        append<uint64_t>(number);
        return *this;
    }


    /*****************************************/
    /**** Read  Functions                 ****/
    /*****************************************/
    ByteBuffer &operator>>(bool &value)
    {
        value = read<char>() > 0 ? true : false;
        return *this;
    }

    ByteBuffer &operator>>(uint8_t &value)
    {
        value = read<uint8_t>();
        return *this;
    }

    ByteBuffer &operator>>(uint16_t &value)
    {
        value = read<uint16_t>();
        return *this;
    }

    ByteBuffer &operator>>(uint32_t &value)
    {
        value = read<uint32_t>();
        return *this;
    }

    ByteBuffer &operator>>(uint64_t &value)
    {
        value = read<uint64_t>();
        return *this;
    }

    ByteBuffer &operator>>(int8_t &value)
    {
        value = read<int8_t>();
        return *this;
    }

    ByteBuffer &operator>>(int16_t &value)
    {
        value = read<int16_t>();
        return *this;
    }

    ByteBuffer &operator>>(int32_t &value)
    {
        value = read<int32_t>();
        return *this;
    }

    ByteBuffer &operator>>(int64_t &value)
    {
        value = read<int64_t>();
        return *this;
    }

    ByteBuffer &operator>>(float &value)
    {

        uint32_t number = read<uint32_t>();
        std::memcpy(&value, &number, sizeof(number));

        if (!std::isfinite(value))
            throw ByteBufferConvertException();

        return *this;
    }

    ByteBuffer &operator>>(double &value)
    {
        uint64_t number = read<uint64_t>();
        std::memcpy(&value, &number, sizeof(number));

        if (!std::isfinite(value))
            throw ByteBufferConvertException();

        return *this;
    }

};
