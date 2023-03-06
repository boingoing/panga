//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef BITVECTOR_H__
#define BITVECTOR_H__

#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

namespace panga {

class BitVector {
 public:
  explicit BitVector(size_t bit_count = 0);
  BitVector(const BitVector& source);
  ~BitVector() = default;

  /**
   * Deep copy a BitVector.<br/>
   * Resizes this to be the same size as rhs.
   */
  BitVector& operator=(const BitVector& rhs);

  /**
   * Calculate equality between this BitVector and |rhs|.<br/>
   * @param bits_to_compare Limit comparision to only the first bits_to_compare
   * bits.
   */
  bool Equals(const BitVector& rhs, size_t bits_to_compare) const;

  /**
   * Calculate equality between this BitVector and |rhs|.<br/>
   * Note: Compares up to the number of bits in |this| BitVector.
   */
  bool Equals(const BitVector& rhs) const;

  /**
   * Set the number of bits in the BitVector.<br/>
   * Note: Unsets all bits in the BitVector.
   */
  void SetBitCount(size_t bit_count);

  /**
   * Get the count of bits in the BitVector.
   */
  size_t GetBitCount() const;

  /**
   * Clip the length of the BitVector.<br/>
   * Does not reduce the size of the backing storage.<br/>
   * Note: Cannot grow the BitVector (bitCount must be less than current count).
   */
  void Clip(size_t bit_count);

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
  bool Get(size_t index) const;

  /**
   * Flip one bit in the BitVector.
   * @param index The index of the bit to flip.
   */
  void Flip(size_t index);

  /**
   * Construct an integer from bits in the BitVector.
   * @param bit_index Starting bit index in the BitVector at which to fetch the
   * integer.
   * @param bit_width Number of bits to use in constructing the integer.
   */
  template <typename IntegerType = uint64_t>
  IntegerType GetInt(size_t bit_index, size_t bit_width) const {
    assert((bit_index + bit_width) <= this->bit_count_);
    return ReadInt<IntegerType>(this->bytes_.data(), bit_index, bit_width);
  }

  /**
   * Store an integer value in the BitVector.
   * @param value Value of the integer to store.
   * @param bit_index Starting bit index in the BitVector at which to store the
   * integer.
   * @param bit_width Number of bits to store in this BitVector for the integer.
   */
  template <typename IntegerType = uint64_t>
  void SetInt(IntegerType value, size_t bit_index, size_t bit_width) {
    assert((bit_index + bit_width) <= this->bit_count_);
    WriteInt<IntegerType>(this->bytes_.data(), bit_index, bit_width, value);
  }

  /**
   * Copy a range of bits from this vector.
   * @param destination BitVector into which we'll write bits from this.
   * @param destination_start_bit_offset Bit index into destination where we
   * will start writing.
   * @param source_start_bit_offset Bit index into this where we will start
   * copying.
   * @param bits_to_copy Count of bits we should copy.
   */
  void SubVector(BitVector* destination, size_t destination_start_bit_offset,
                 size_t source_start_bit_offset, size_t bits_to_copy) const;

  /**
   * Calculate the Hamming Distance between this and |rhs|.<br/>
   * The Hamming Distance is the number of bits which differ between two
   * BitVectors.
   * @return The number of bits which differ between this and |rhs|.
   */
  size_t HammingDistance(const BitVector& rhs) const;

  /**
   * Write a binary representation of this BitVector into a buffer.<br/>
   * The buffer will be null-terminated.<br/>
   * Each character in the resulting buffer will be a '0' or '1'.<br/>
   * The most-significant bit is written at the beginning of the buffer.</br>
   * Note: buffer_length must be > bitCount.
   * @param buffer Buffer which we will write into.
   * @param buffer_length Length of buffer in bytes.
   * @return If buffer is nullptr, returns the number of bytes needed to store
   * the binary representation of this BitVector. Otherwise, returns the number
   *         of bytes written to buffer.
   */
  size_t ToString(char* buffer, size_t buffer_length) const;

  /**
   * Read a BitVector from a null-terminated string of '0' and '1' characters.
   * @param buffer String which we will read from.
   * @param buffer_length Length of buffer in bytes.
   */
  void FromString(const char* buffer, size_t buffer_length);

  /**
   * Write a hex representation of this BitVector into a buffer.<br/>
   * The buffer will be null-terminated.<br/>
   * Note: buffer_length must be > bitCount / 8 * 2.
   * @see BitVector::HexFormat
   * @param buffer Buffer which we will write into.
   * @param buffer_length Length of buffer in bytes.
   * @return If buffer is nullptr, returns the number of bytes needed to store
   * the hex representation of this BitVector. Otherwise, returns the number of
   * bytes written to buffer.
   */
  size_t ToStringHex(char* buffer, size_t buffer_length) const;

  /**
   * Read a BitVector from a null-terminated string containing a hex
   * representation.
   * @param buffer String which we will read from.
   * @param buffer_length Length of buffer in bytes.
   */
  void FromStringHex(const char* buffer, size_t buffer_length);

 protected:
  /**
   * Get a read-only view of the byte buffer underlying this BitVector.
   */
  const std::vector<std::byte>& GetBytes() const;

  /**
   * Get a writable view of the byte buffer underlying this BitVector.
   */
  std::vector<std::byte>& GetBytesWritable();

  /**
   * Resize the BitVector such that it contains |bit_count| bits.
   * Note: Does not change the value of any existing bits currently in the
   * vector.
   */
  void Resize(size_t bit_count);

  /**
   * Write |bits_to_copy| bits from an integer type |value| as binary data into
   * |bytes| beginning at |start_bit_index|.
   */
  template <typename IntegerType = uint64_t>
  static void WriteInt(std::byte* bytes, size_t start_bit_index,
                       size_t bits_to_copy, IntegerType value) {
    assert(bits_to_copy <= CHAR_BIT * sizeof(IntegerType));
    WriteBytes(reinterpret_cast<std::byte*>(&value), 0, bytes, start_bit_index,
               bits_to_copy);
  }

  /**
   * Read |bits_to_copy| bits from |bytes| starting at |start_bit_index| and
   * interpret them as an integer type value.
   */
  template <typename IntegerType = uint64_t>
  static IntegerType ReadInt(const std::byte* bytes, size_t start_bit_index,
                             size_t bits_to_copy) {
    assert(bits_to_copy <= CHAR_BIT * sizeof(IntegerType));
    IntegerType value = 0;
    WriteBytes(bytes, start_bit_index, reinterpret_cast<std::byte*>(&value), 0,
               bits_to_copy);
    return value;
  }

  /**
   * Copy |bits_to_copy| bits from a byte buffer |source| into
   * |destination|.<br/> |source_start_bit_offset| and
   * |destination_start_bit_offset| are bit offsets into |source| and
   * |destination|.<br/> Supports non-byte-aligned copy, though this is slower.
   */
  static void WriteBytes(const std::byte* source,
                         size_t source_start_bit_offset, std::byte* destination,
                         size_t destination_start_bit_offset,
                         size_t bits_to_copy);

  /**
   * Compare |bits_to_compare| bits between |left| and |right|.<br/>
   * Note: Does not support non-byte-aligned compare, begins comparing at the
   * beginning of each byte array.
   */
  static bool Compare(const std::byte* left, const std::byte* right,
                      size_t bits_to_compare);

 public:
  struct HexFormatWrapper {
    std::ostream& _os;
  };
  struct HexFormat_t {
    friend HexFormatWrapper operator<<(std::ostream& out,
                                       const HexFormat_t& manip);
  };

  /**
   * An output manipulator which modifies subsequent stream operations such
   * that the BitVector will be printed in hex characters instead of binary.
   * <br/>
   * BitVector bv;<br/>
   * cout << BitVector::HexFormat << bv;<br/>
   * Note: Only the BitVector following BitVector::HexFormat will be formatted
   * in hex.<br/>
   * cout << BitVector::HexFormat << bv1 << BitVector::HexFormat << bv2;
   */
  static HexFormat_t HexFormat;

  friend std::ostream& operator<<(const HexFormatWrapper& wrapper,
                                  const BitVector& bv);
  friend std::ostream& operator<<(std::ostream& out, const BitVector& bv);

 protected:
  void WriteToStream(std::ostream& out) const;
  void WriteToStreamHex(std::ostream& out) const;

 private:
  /**
   * Underlying storage for the bits of the BitVector.
   */
  std::vector<std::byte> bytes_;

  /**
   * Count of the bits stored in the BitVector.<br/>
   * Note: This may be less than the number of bits stored in all the bytes
   *       in the bytes array.
   */
  size_t bit_count_ = 0;
};

}  // namespace panga

#endif  // BITVECTOR_H__
