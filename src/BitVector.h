#pragma once

#include <cstddef>
#include <cstdint>

namespace panga {

class BitVector {
protected:
    /**
     * Underlying storage for the bits of the BitVector.
     */
    std::byte* _bytes;

    /**
     * Count of the bytes in the underlying byte storage of the BitVector.
     */
    size_t _bytesCount;

    /**
     * Count of the bits stored in the BitVector.
     * Note: This may be less than the number of bits stored in all the bytes
     *       in the bytes array.
     */
    size_t _bitCount;

public:
    BitVector(size_t bitCount = 0);
    BitVector(const BitVector& source);
    ~BitVector();

    /**
     * Deep copy a BitVector.
     * Resizes this.
     */
    void operator=(const BitVector& rhs);

    /**
     * Calculate equality between two BitVectors.
     * @param bitsToCompare Limit comparision to only the first bitsToCompare bits.
     */
    bool Equals(const BitVector& rhs, size_t bitsToCompare);

    /**
     * Calculate equality between two BitVectors.
     */
    bool Equals(const BitVector& rhs);

    /**
     * Set the number of bits in the BitVector.
     * Note: Unsets all bits in the BitVector.
     */
    void SetBitCount(size_t bitCount);

    /**
     * Get the count of bits in the BitVector.
     */
    size_t GetBitCount() const;

    /**
     * Clip the length of the BitVector.
     * Does not reduce the size of the backing storage.
     * Note: Cannot grow the BitVector (bitCount must be less than current count).
     */
    void Clip(size_t bitCount);

    /**
     * Unset all bits in the BitVector.
     */
    void Clear();

    /**
     * Set a single bit the BitVector.
     * @param index The index of the bit to set.
     */
    void Set(size_t index);

    /**
     * Unset a single bit in the BitVector.
     * @param index The index of the bit to unset.
     */
    void Unset(size_t index);

    /**
     * Get a single bit from the BitVector.
     * @param index The index of the bit to get.
     * @return true if bit is set.
     */
    bool Get(size_t index);

    /**
     * Flip one bit in the BitVector.
     * @param index The index of the bit to flip.
     */
    void Flip(size_t index);

    /**
     * Construct an integer from bits in the BitVector.
     * @param bitIndex Starting bit index for the integer to fetch.
     * @param bitWidth Number of bits to use in constructing the integer.
     */
    template <typename IntegerType = uint64_t>
    IntegerType GetInt(size_t bitIndex, size_t bitWidth) {
        assert((bitIndex + bitWidth) <= this->_bitCount);
        return ReadInt<IntegerType>(this->_bytes, bitIndex, bitWidth);
    }

    /**
     * Store an integer value in the BitVector.
     * @param value Value of the integer to store.
     * @param bitIndex Starting bit index into this BitVector where we should
     * store the integer.
     * @param bitWidth Number of bits to store in this BitVector for the integer.
     */
    template <typename IntegerType = uint64_t>
    void SetInt(IntegerType value, size_t bitIndex, size_t bitWidth) {
        assert((bitIndex + bitWidth) <= this->_bitCount);
        WriteInt<IntegerType>(this->_bytes, bitIndex, bitWidth, value);
    }

    /**
     * Copy a range of bits from this vector.
     * @param destination BitVector into which we'll write bits from this.
     * @param destinationStartIndex Bit index into destination where we will start writing.
     * @param sourceStartIndex Bit index into this where we will start copying.
     * @param bitWidth Count of bits we should copy.
     */
    void SubVector(BitVector* destination, size_t destinationStartIndex, size_t sourceStartIndex, size_t bitWidth);

    /**
     * Count the number of bits set in a byte.
     */
    uint32_t CountSetBits(std::byte val);

    /**
     * Calculate the Hamming Distance between two BitVectors.
     */
    size_t HammingDistance(const BitVector& rhs);

    /**
     * Write a binary representation of this BitVector into a buffer.
     * The buffer will be null-terminated.
     * Each character in the resulting buffer will be a '0' or '1'.
     * Note: bufferLength must be > bitCount.
     * @param buffer Buffer which we will write into.
     * @param bufferLength Length of buffer in bytes.
     * @return If buffer is nullptr, returns the number of bytes needed to store the
     *         binary representation of this BitVector. Otherwise, returns the number
     *         of bytes written to buffer.
     */
    size_t ToString(char* buffer, size_t bufferLength);

    /**
     * Read a BitVector from a null-terminated string of '0' and '1' characters.
     * @param buffer String which we will read from.
     * @param bufferLength Length of buffer in bytes.
     */
    void FromString(const char* buffer, size_t bufferLength);

    /**
     * Write a hex representation of this BitVector into a buffer.
     * The buffer will be null-terminated.
     * Note: bufferLength must be > bitCount / 8 * 2.
     * @param buffer Buffer which we will write into.
     * @param bufferLength Length of buffer in bytes.
     * @return If buffer is nullptr, returns the number of bytes needed to store the
     *         hex representation of this BitVector. Otherwise, returns the number
     *         of bytes written to buffer.
     */
    size_t ToHexString(char* buffer, size_t bufferLength);

    /**
     * Read a BitVector from a null-terminated string containing a hex
     * representation.
     * @param buffer String which we will read from.
     * @param bufferLength Length of buffer in bytes.
     */
    void FromHexString(const char* buffer, size_t bufferLength);

protected:
    void Resize(size_t bitCount);

    static bool Compare(std::byte* left, std::byte* right, size_t bitsToCompare);

    static void WriteBytes(std::byte* source, size_t sourceBitStartIndex, std::byte* destination, size_t destinationBitStartIndex, size_t bitWidth);

    template <typename IntegerType = uint64_t>
    static void WriteInt(std::byte* bytes, size_t bitIndex, size_t bitWidth, IntegerType value) {
        assert(bitWidth <= 8 * sizeof(IntegerType));
        WriteBytes(reinterpret_cast<std::byte*>(&value), 0, bytes, bitIndex, bitWidth);
    }

    template <typename IntegerType = uint64_t>
    static IntegerType ReadInt(std::byte* bytes, size_t bitIndex, size_t bitWidth) {
        assert(bitWidth <= 8 * sizeof(IntegerType));
        IntegerType value = 0;
        WriteBytes(bytes, bitIndex, reinterpret_cast<std::byte*>(&value), 0, bitWidth);
        return value;
    }
};

} // namespace panga
