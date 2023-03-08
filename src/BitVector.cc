//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#include "BitVector.h"

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <limits>

namespace {

constexpr size_t BitsPerByte = CHAR_BIT;

// We would like to use std::numeric_limits<std::byte>::max() but that isn't
// defined for std::byte.
constexpr std::byte MaxByteValue = std::byte{UCHAR_MAX};

/**
 * Count the number of bits set in a byte.
 */
uint32_t CountSetBits(std::byte val) {
  static constexpr int bits_set_table[256] = {
      0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4,
      2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4,
      2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6,
      4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5,
      3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6,
      4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
      4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};
  return bits_set_table[std::to_integer<uint8_t>(val)];
}

}  // namespace

namespace panga {

BitVector::HexFormat_t BitVector::HexFormat;

BitVector::BitVector(size_t bit_count) { SetBitCount(bit_count); }

BitVector::BitVector(const BitVector& source) { *this = source; }

BitVector& BitVector::operator=(const BitVector& rhs) {
  if (this != &rhs) {
    Resize(rhs.bit_count_);
    std::copy_n(rhs.bytes_.cbegin(), rhs.bytes_.size(), bytes_.begin());
  }
  return *this;
}

const std::vector<std::byte>& BitVector::GetBytes() const { return bytes_; }

std::vector<std::byte>& BitVector::GetBytesWritable() { return bytes_; }

// static
void BitVector::WriteBytes(const std::byte* source,
                           size_t source_start_bit_offset,
                           std::byte* destination,
                           size_t destination_start_bit_offset,
                           size_t bits_to_copy) {
  const size_t destination_first_byte_index =
      destination_start_bit_offset / BitsPerByte;
  const size_t destination_last_byte_index =
      (destination_start_bit_offset + bits_to_copy) / BitsPerByte;
  const std::byte last_byte = destination[destination_last_byte_index];

  // Copy all the bytes except for the last one.
  // If we're reading from and writing to byte-aligned memory, we can take a
  // fast path using memcpy.
  if (source_start_bit_offset % BitsPerByte == 0 &&
      destination_start_bit_offset % BitsPerByte == 0) {
    std::memcpy(destination + destination_first_byte_index,
                source + (source_start_bit_offset / BitsPerByte),
                sizeof(std::byte) * (destination_last_byte_index -
                                     destination_first_byte_index + 1));
  } else {
    // Keep track of the first byte in destination so we can put back the bits
    // in that byte which are not supposed to be overwritten.
    const std::byte first_byte = destination[destination_first_byte_index];
    // Zero-out all the bytes which we're going to overwrite in destination.
    // TODO(boingoing): Can we avoid this?
    std::memset(destination + destination_first_byte_index, 0,
                sizeof(std::byte) * (destination_last_byte_index -
                                     destination_first_byte_index + 1));

    // Now copy bits from source until we've written at least bits_to_copy.
    size_t bits_written = 0;
    while (bits_written < bits_to_copy) {
      const size_t source_byte_index =
          (source_start_bit_offset + bits_written) / BitsPerByte;
      const size_t source_bit_offset =
          (source_start_bit_offset + bits_written) % BitsPerByte;
      const size_t destination_byte_index =
          (destination_start_bit_offset + bits_written) / BitsPerByte;
      const size_t destination_bit_offset =
          (destination_start_bit_offset + bits_written) % BitsPerByte;

      // Shift the source byte such that it begins with the relevant bits and
      // shift those so they slot into the destination byte at the correct bit
      // offset.
      destination[destination_byte_index] |=
          source[source_byte_index] >> source_bit_offset
                                           << destination_bit_offset;

      // Due to the bit-shifting above, we can only ever copy the smaller of
      // source_bit_offset and destination_bit_offset number of bits while
      // writing a single byte.
      bits_written += std::min(BitsPerByte - source_bit_offset,
                               BitsPerByte - destination_bit_offset);
    }

    // Mask off the bits in the destination first byte which we do not want to
    // overwrite and put them back into the destination first byte.
    const size_t destination_first_byte_bit_index =
        destination_start_bit_offset % BitsPerByte;
    destination[destination_first_byte_index] |=
        first_byte &
        MaxByteValue >> (BitsPerByte - destination_first_byte_bit_index);
  }

  // Now mask off the last byte so we don't lose existing bits there.
  const size_t destination_last_byte_bit_index =
      (destination_start_bit_offset + bits_to_copy) % BitsPerByte;
  const std::byte mask = MaxByteValue << destination_last_byte_bit_index;
  destination[destination_last_byte_index] =
      destination[destination_last_byte_index] & ~mask | last_byte & mask;
}

// static
bool BitVector::Compare(const std::byte* left, const std::byte* right,
                        size_t bits_to_compare) {
  const size_t bytes_to_compare = bits_to_compare / BitsPerByte;
  const size_t bits_remaining = bits_to_compare % BitsPerByte;

  if (std::memcmp(left, right, bytes_to_compare) != 0) {
    return false;
  }

  if (bits_remaining == 0) {
    return true;
  }

  const std::byte mask = MaxByteValue >> (BitsPerByte - bits_remaining);
  const std::byte result =
      (left[bytes_to_compare] & mask) ^ (right[bytes_to_compare] & mask);
  return std::byte{0} == result;
}

void BitVector::Clear() {
  this->bytes_.assign(this->bytes_.size(), std::byte{0x0});
}

void BitVector::Resize(size_t bit_count) {
  // Number of full bytes needed.
  size_t new_bytes_count = bit_count / BitsPerByte;

  // Partial byte at the end.
  if (bit_count % BitsPerByte != 0) {
    new_bytes_count++;
  }

  // Allocate new memory only if we need to grow the vector.
  if (this->bytes_.size() < new_bytes_count) {
    this->bytes_.resize(new_bytes_count);
  }

  // Possibly truncate if the new size is smaller.
  this->bit_count_ = bit_count;
}

void BitVector::SetBitCount(size_t bit_count) {
  this->Resize(bit_count);
  this->Clear();
}

void BitVector::Clip(size_t bit_count) {
  // We can only clip to a smaller number of bits than are allocated.
  assert(bit_count <= this->bit_count_);

  // No need to touch the underlying bytes, just truncate our tracking field.
  this->bit_count_ = bit_count;
}

size_t BitVector::GetBitCount() const { return this->bit_count_; }

void BitVector::Flip(size_t index) {
  assert(index < this->bit_count_);

  const size_t byte_offset = index / BitsPerByte;
  const size_t bit_offset = index % BitsPerByte;
  const std::byte mask = std::byte{1} << bit_offset;
  this->bytes_[byte_offset] ^= mask;
}

void BitVector::Set(size_t index) {
  assert(index < this->bit_count_);

  const size_t byte_offset = index / BitsPerByte;
  const size_t bit_offset = index % BitsPerByte;
  const std::byte mask = std::byte{1} << bit_offset;
  this->bytes_[byte_offset] |= mask;
}

void BitVector::Unset(size_t index) {
  assert(index < this->bit_count_);

  const size_t byte_offset = index / BitsPerByte;
  const size_t bit_offset = index % BitsPerByte;
  const std::byte mask = std::byte{1} << bit_offset;
  this->bytes_[byte_offset] &= ~mask;
}

bool BitVector::Get(size_t index) const {
  assert(index < this->bit_count_);

  const size_t byte_offset = index / BitsPerByte;
  const size_t bit_offset = index % BitsPerByte;
  const std::byte mask = std::byte{1} << bit_offset;
  const std::byte value = this->bytes_[byte_offset] & mask;
  return std::to_integer<uint8_t>(value) != 0;
}

void BitVector::SubVector(BitVector* destination,
                          size_t destination_start_bit_offset,
                          size_t source_start_bit_offset,
                          size_t bits_to_copy) const {
  // Resize destination if it's not big enough
  destination->Resize(destination_start_bit_offset + bits_to_copy);

  WriteBytes(this->bytes_.data(), source_start_bit_offset,
             destination->bytes_.data(), destination_start_bit_offset,
             bits_to_copy);
}

size_t BitVector::HammingDistance(const BitVector& rhs) const {
  size_t distance = 0;

  // If the two BitVectors differ in the number of bits they contain, it's not
  // clear what we should return. For now, treat this as an error condition and
  // return infinity-ish.
  if (this->bit_count_ != rhs.bit_count_) {
    return std::numeric_limits<size_t>::max();
  }

  const size_t last_byte = this->bit_count_ / BitsPerByte;

  // All but the last byte.
  for (size_t i = 0; i < last_byte; i++) {
    distance += CountSetBits(this->bytes_[i] ^ rhs.bytes_[i]);
  }

  // Mask the last byte so it only includes bits which are in the vector.
  const size_t relevant_bits = this->bit_count_ % BitsPerByte;

  if (relevant_bits > 0) {
    const std::byte mask = MaxByteValue >> (BitsPerByte - relevant_bits);
    distance += CountSetBits((this->bytes_[last_byte] & mask) ^
                             (rhs.bytes_[last_byte] & mask));
  }

  return distance;
}

bool BitVector::Equals(const BitVector& rhs, size_t bits_to_compare) const {
  assert(rhs.bit_count_ >= bits_to_compare);
  assert(this->bit_count_ >= bits_to_compare);

  return Compare(this->bytes_.data(), rhs.bytes_.data(), bits_to_compare);
}

bool BitVector::Equals(const BitVector& rhs) const {
  return this->Equals(rhs, this->bit_count_);
}

size_t BitVector::ToString(char* buffer, size_t buffer_length) const {
  if (buffer != nullptr) {
    assert(buffer_length > this->bit_count_);

    for (size_t i = 0; i < this->bit_count_; i++) {
      buffer[i] = this->Get(this->bit_count_ - i - 1) ? '1' : '0';
    }
    buffer[this->bit_count_] = '\0';
  }

  return this->bit_count_ + 1U;
}

void BitVector::FromString(const char* buffer, size_t buffer_length) {
  assert(buffer != nullptr);

  this->SetBitCount(buffer_length);

  for (size_t i = 0; i < buffer_length; i++) {
    assert(buffer[i] == '0' || buffer[i] == '1');

    if (buffer[i] == '1') {
      this->Set(buffer_length - i - 1U);
    }
  }
}

size_t BitVector::ToStringHex(char* buffer, size_t buffer_length) const {
  const size_t last_byte = this->bit_count_ / BitsPerByte;
  const size_t bytes_needed = (last_byte + 1U) * 2U;

  if (buffer != nullptr) {
    assert(buffer_length > bytes_needed);

    // Mask the last byte.
    const size_t relevant_bits = this->bit_count_ % BitsPerByte;
    const std::byte mask = MaxByteValue >> (BitsPerByte - relevant_bits);
    int bytes_written =
        snprintf(buffer, 3U, "%02hhx",
                 std::to_integer<uint8_t>(this->bytes_[last_byte] & mask));
    for (size_t i = 0; i < last_byte; i++) {
      bytes_written +=
          snprintf(buffer + 2U * (i + 1U), 3U, "%02hhx",
                   std::to_integer<uint8_t>(this->bytes_[last_byte - i - 1U]));
    }

    assert(bytes_written == bytes_needed);
    assert(buffer[bytes_needed] == '\0');
  }

  return bytes_needed + 1U;
}

void BitVector::FromStringHex(const char* buffer, size_t buffer_length) {
  assert(buffer != nullptr);
  assert(buffer_length >= 2U);

  this->SetBitCount(buffer_length / 2U * BitsPerByte);

  constexpr int radix = 16;
  char temp_buffer[3];
  size_t i = 0;
  for (std::byte& b : this->bytes_) {
    strncpy(temp_buffer, buffer + buffer_length - 2U * ++i, 2U);
    temp_buffer[2] = '\0';
    b = static_cast<std::byte>(strtoul(temp_buffer, nullptr, radix));
  }
}

void BitVector::WriteToStream(std::ostream& out) const {
  for (size_t i = 0; i < this->bit_count_; i++) {
    out << (this->Get(this->bit_count_ - i - 1U) ? '1' : '0');
  }
}

void BitVector::WriteToStreamHex(std::ostream& out) const {
  if (this->bytes_.empty()) {
    return;
  }

  // Mask the last byte.
  const size_t last_byte = this->bit_count_ / BitsPerByte;
  const size_t relevant_bits = this->bit_count_ % BitsPerByte;
  const std::byte mask = MaxByteValue >> (BitsPerByte - relevant_bits);

  out << std::hex << std::setfill('0') << std::setw(2)
      << static_cast<int>(this->bytes_[last_byte] & mask);
  for (size_t i = 0; i < last_byte; i++) {
    out << std::hex << std::setfill('0') << std::setw(2)
        << static_cast<int>(this->bytes_[last_byte - i - 1U]) << std::dec;
  }
}

std::ostream& operator<<(std::ostream& out, const BitVector& bv) {
  bv.WriteToStream(out);
  return out;
}

BitVector::HexFormatWrapper operator<<(
    std::ostream& out, const BitVector::HexFormat_t& /*manip*/) {
  return {out};
}

std::ostream& operator<<(const BitVector::HexFormatWrapper& wrapper,
                         const BitVector& bv) {
  std::ostream& out = wrapper.os;
  bv.WriteToStreamHex(out);
  return out;
}

}  // namespace panga
