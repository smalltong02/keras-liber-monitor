#pragma once
#include "PeLib.h"

namespace cchips {

    class DynamicBuffer
    {
    public:
        DynamicBuffer() : _data(), _capacity(0) {}
        DynamicBuffer(uint32_t capacity) : _data(), _capacity(capacity) { _data.reserve(capacity); }
        DynamicBuffer(const std::vector<uint8_t>& data) : _data(data), _capacity(data.size()) {}
        DynamicBuffer(const DynamicBuffer& dynamicBuffer) : _data(dynamicBuffer._data), _capacity(dynamicBuffer._capacity) {}
        DynamicBuffer(const DynamicBuffer& dynamicBuffer, uint32_t startPos, uint32_t amount) {
            std::vector<uint8_t> tmpBuffer = dynamicBuffer.getBuffer();
            std::vector<uint8_t> buffer(tmpBuffer.begin() + startPos, tmpBuffer.begin() + startPos + amount);

            _data = buffer;
            _capacity = static_cast<uint32_t>(buffer.size());
        }

        DynamicBuffer& operator =(DynamicBuffer dynamicBuffer) {
            std::swap(_data, dynamicBuffer._data);
            std::swap(_capacity, dynamicBuffer._capacity);
            return *this;
        }

        void setCapacity(uint32_t capacity) {
            _capacity = capacity;
            _data.reserve(_capacity);
        }
        uint32_t getCapacity() const { return _capacity; }

        uint32_t getRealDataSize() const { return static_cast<uint32_t>(_data.size()); }

        void erase(uint32_t startPos, uint32_t amount) {
            if (startPos >= _data.size())
                return;

            amount = startPos + amount > _data.size() ? _data.size() - startPos : amount;
            _data.erase(_data.begin() + startPos, _data.begin() + startPos + amount);
        }

        const uint8_t* getRawBuffer() const { return _data.data(); }
        std::vector<uint8_t> getBuffer() const { return _data; }

        void forEach(const std::function<void(uint8_t&)>& func) {
            for (uint8_t& byte : _data)
                func(byte);
        }
        void forEachReverse(const std::function<void(uint8_t&)>& func) {
            for (std::vector<uint8_t>::reverse_iterator itr = _data.rbegin(); itr != _data.rend(); ++itr)
            {
                uint8_t& byte = *itr;
                func(byte);
            }
        }

        /**
         * Reads the data from the buffer. If the reading position is beyond the size of the real data, the real data are resized so this
         * value can be read filling the new bytes with default (0) value. If the read overlaps the capacity of the buffer, only the bytes
         * that still fall into the capacity are read and the rest is filled with default (0) value.
         *
         * @tparam The type of the data to read. This must be integral type.
         *
         * @param pos Position where to start the reading.
         * @param endianness The endianness in which the data should be read. If not specified, default endianness assigned to DynamicBuffer is used.
         *
         * @return The read value from the buffer.
         */
        template <typename T> T read(uint32_t pos) const
        {
            static_assert(std::is_integral<T>::value, "DynamicBuffer::read can only accept integral types");
            return readImpl<T>(pos);
        }

        std::string readString(uint32_t pos, uint32_t maxLength = 0) const {
            std::string str;
            char ch;

            while (((ch = read<char>(pos++)) != 0) && (!maxLength || str.length() < maxLength))
                str += ch;

            return str;
        }

        /**
         * Writes the data to the buffer. If the writing poisition is beyond the size of the real data, the real data are resized so this
         * value can be written filling the new bytes with default (0) value. If the write overlaps the capacity of the buffer, only the bytes
         * that still fall into the capacity are written and the rest is ignored.
         *
         * @tparam The type of the data to write. This must be integral type.
         *
         * @param data The data to write.
         * @param pos The position where to start writing.
         * @param endianness The endianness in which the data should be written. If not specified, default endianness assigned to DynamicBuffer is used.
         */
        template <typename T> void write(const T& data, uint32_t pos)
        {
            static_assert(std::is_integral<T>::value, "DynamicBuffer::write can only accept integral types");
            writeImpl(data, pos);
        }

        void writeRepeatingByte(uint8_t byte, uint32_t pos, uint32_t repeatAmount) {
            if (pos + repeatAmount > _capacity)
                repeatAmount = _capacity - pos;

            if (pos + repeatAmount > _data.size())
                _data.resize(pos + repeatAmount);

            memset(&_data[pos], byte, repeatAmount);
        }

    private:
        template <typename T> void writeImpl(const T& data, uint32_t pos)
        {
            // If the writing position is completely out of bounds, we just end
            if (pos >= _capacity)
                return;

            // Buffer would overlap the capacity, copy just the chunk that fits
            uint32_t bytesToWrite = sizeof(T);
            if (pos + bytesToWrite > getCapacity())
                bytesToWrite = getCapacity() - pos;

            if (bytesToWrite == 0)
                return;

            // Check whether there is enough space allocated
            if (pos + bytesToWrite > getRealDataSize())
                _data.resize(pos + bytesToWrite);

            for (uint32_t i = 0; i < bytesToWrite; ++i)
            {
                _data[pos + i] = (data >> (i << 3)) & 0xFF;
            }
        }

        template <typename T> T readImpl(uint32_t pos) const
        {
            // We are at the end, we are unable to read anything
            if (pos >= _data.size())
                return T{};

            // We are at the end, we are unable to read anything
            if (pos >= _capacity)
                return T{};

            // If reading overlaps over the size, make sure we don't access uninitialized memory
            uint32_t bytesToRead = sizeof(T);
            if (pos + bytesToRead > getCapacity())
                bytesToRead = getCapacity() - pos;

            if (pos + bytesToRead > getRealDataSize())
                bytesToRead = getRealDataSize() - pos;

            T ret = T{};
            for (uint32_t i = 0; i < bytesToRead; ++i)
            {
                ret |= static_cast<uint64_t>(_data[pos + i]) << (i << 3);
            }

            return ret;
        }

        mutable std::vector<uint8_t> _data;
        uint32_t _capacity;
    };
} // namespace cchips
