//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include <limits>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iomanip>

#include "BitVector.h"

using namespace panga;

BitVector::HexFormat_t BitVector::HexFormat;

void BitVector::WriteBytes(std::byte* source, size_t sourceBitStartIndex, std::byte* destination, size_t destinationBitStartIndex, size_t bitWidth) {
    size_t destinationFirstByteIndex = destinationBitStartIndex / 8;
    size_t destinationLastByteIndex = (destinationBitStartIndex + bitWidth) / 8;
    std::byte lastByte = destination[destinationLastByteIndex];

    // If we're reading from and writing to byte-aligned memory, we can take a fast path using memcpy.
    if (sourceBitStartIndex % 8 == 0 && destinationBitStartIndex % 8 == 0) {
        memcpy(destination + destinationFirstByteIndex, source + (sourceBitStartIndex / 8), sizeof(std::byte) * (destinationLastByteIndex - destinationFirstByteIndex + 1));
    } else {
        std::byte firstByte = destination[destinationFirstByteIndex];
        memset(destination + destinationFirstByteIndex, 0, sizeof(std::byte) * (destinationLastByteIndex - destinationFirstByteIndex + 1));
        size_t bitsWritten = 0;

        while (bitsWritten < bitWidth) {
            size_t sourceByteIndex = (sourceBitStartIndex + bitsWritten) / 8;
            size_t destinationByteIndex = (destinationBitStartIndex + bitsWritten) / 8;
            size_t sourceBitOffset = (sourceBitStartIndex + bitsWritten) % 8;
            size_t destinationBitOffset = (destinationBitStartIndex + bitsWritten) % 8;

            destination[destinationByteIndex] |= source[sourceByteIndex] >> sourceBitOffset << destinationBitOffset;

            bitsWritten += std::min(8 - sourceBitOffset, 8 - destinationBitOffset);
        }

        size_t destinationFirstByteBitIndex = destinationBitStartIndex % 8;
        destination[destinationFirstByteIndex] |= firstByte & std::byte{ 0xFF } >> (8 - destinationFirstByteBitIndex);
    }

    size_t destinationLastByteBitIndex = (destinationBitStartIndex + bitWidth) % 8;
    std::byte mask = std::byte{ 0xFF } << destinationLastByteBitIndex;
    destination[destinationLastByteIndex] = destination[destinationLastByteIndex] & ~mask | lastByte & mask;
}

bool BitVector::Compare(std::byte* left, std::byte* right, size_t bitsToCompare) {
    size_t bytesToCompare = bitsToCompare / 8;
    size_t bitsRemaining = bitsToCompare % 8;

    if (memcmp(left, right, bytesToCompare) != 0) {
        return false;
    }

    std::byte mask = std::byte{ 0xFF } >> (8 - bitsRemaining);
    std::byte result = (left[bytesToCompare] & mask) ^ (right[bytesToCompare] & mask);
    return std::to_integer<uint8_t>(result) == 0;
}

BitVector::BitVector(size_t bitCount) :
    _bytes(nullptr),
    _bytesCount(0),
    _bitCount(0) {
    this->SetBitCount(bitCount);
}

BitVector::BitVector(const BitVector& source) :
    _bytes(nullptr),
    _bytesCount(0),
    _bitCount(0) {
    *this = source;
}

BitVector::~BitVector() {
    free(this->_bytes);
}

void BitVector::operator=(const BitVector& rhs) {
    this->Resize(rhs._bitCount);
    memcpy(this->_bytes, rhs._bytes, sizeof(std::byte) * this->_bytesCount);
}

void BitVector::Clear() {
    memset(this->_bytes, 0, sizeof(std::byte) * this->_bytesCount);
}

void BitVector::Resize(size_t bitCount) {
    size_t newBytesCount = 1 + (bitCount / 8);

    // Allocate new memory only if we need to grow the vector.
    if (this->_bytesCount < newBytesCount) {
        this->_bytes = (std::byte*)realloc(this->_bytes, sizeof(std::byte) * newBytesCount);
        this->_bytesCount = newBytesCount;
    }

    this->_bitCount = bitCount;
}

void BitVector::SetBitCount(size_t bitCount) {
    this->Resize(bitCount);
    this->Clear();
}

void BitVector::Clip(size_t bitCount) {
    assert(bitCount <= this->_bitCount);

    this->_bitCount = bitCount;
}

size_t BitVector::GetBitCount() const {
    return this->_bitCount;
}

void BitVector::Flip(size_t index) {
    assert(index < this->_bitCount);

    size_t byteOffset = index / 8;
    size_t bitOffset = index % 8;
    std::byte mask = std::byte{ 1 } << bitOffset;
    this->_bytes[byteOffset] ^= mask;
}

void BitVector::Set(size_t index) {
    assert(index < this->_bitCount);

    size_t byteOffset = index / 8;
    size_t bitOffset = index % 8;
    std::byte mask = std::byte{ 1 } << bitOffset;
    this->_bytes[byteOffset] |= mask;
}

void BitVector::Unset(size_t index) {
    assert(index < this->_bitCount);

    size_t byteOffset = index / 8;
    size_t bitOffset = index % 8;
    std::byte mask = std::byte{ 1 } << bitOffset;
    this->_bytes[byteOffset] &= ~mask;
}

bool BitVector::Get(size_t index) const {
    assert(index < this->_bitCount);

    size_t byteOffset = index / 8;
    size_t bitOffset = index % 8;
    std::byte mask = std::byte{ 1 } << bitOffset;
    std::byte value = this->_bytes[byteOffset] & mask;
    return std::to_integer<uint8_t>(value) != 0;
}

void BitVector::SubVector(BitVector* destination, size_t destinationStartIndex, size_t sourceStartIndex, size_t bitWidth) const {
    // Resize destination if it's not big enough
    destination->Resize(destinationStartIndex + bitWidth);

    WriteBytes(this->_bytes, sourceStartIndex, destination->_bytes, destinationStartIndex, bitWidth);
}

uint32_t BitVector::CountSetBits(std::byte val) {
    uint32_t v = static_cast<uint32_t>(val);
    // c accumulates the total bits set in val
    uint32_t c = 0;
    for (c = 0; v; c++) {
        // Clear the least significant bit set
        v &= v - 1;
    }
    return c;
}

size_t BitVector::HammingDistance(const BitVector* rhs) const {
    size_t distance = 0;

    if (this->_bitCount != rhs->_bitCount) {
        return std::numeric_limits<size_t>::max();
    }

    size_t lastByte = this->_bitCount / 8;

    // All but the last byte
    for (size_t i = 0; i < lastByte; i++) {
        distance += CountSetBits(this->_bytes[i] ^ rhs->_bytes[i]);
    }

    // Mask the last byte
    size_t relevantBits = this->_bitCount % 8;
    std::byte mask = std::byte{ 0xFF } >> (8 - relevantBits);

    distance += CountSetBits((this->_bytes[lastByte] & mask)
                             ^ (rhs->_bytes[lastByte] & mask));

    return distance;
}

bool BitVector::Equals(const BitVector* rhs, size_t bitsToCompare) const {
    assert(rhs->_bitCount >= bitsToCompare);
    assert(this->_bitCount >= bitsToCompare);

    return Compare(this->_bytes, rhs->_bytes, bitsToCompare);
}

bool BitVector::Equals(const BitVector* rhs) const {
    return this->Equals(rhs, this->_bitCount);
}

size_t BitVector::ToString(char* buffer, size_t bufferLength) const {
    if (buffer == nullptr) {
        return this->_bitCount + 1;
    }

    assert(bufferLength > this->_bitCount);

    for (size_t i = 0; i < this->_bitCount; i++) {
        buffer[i] = this->Get(this->_bitCount - i - 1) ? '1' : '0';
    }
    buffer[this->_bitCount] = '\0';

    return this->_bitCount + 1;
}

void BitVector::FromString(const char* buffer, size_t bufferLength) {
    this->Resize(bufferLength);
    this->Clear();

    for (size_t i = 0; i < bufferLength; i++) {
        assert(buffer[i] == '0' || buffer[i] == '1');

        if (buffer[i] == '1') {
            this->Set(bufferLength - i - 1);
        }
    }
}

size_t BitVector::ToStringHex(char* buffer, size_t bufferLength) const {
    size_t lastByte = this->_bitCount / 8;
    size_t bytesNeeded = (lastByte + 1) * 2;

    if (buffer == nullptr) {
        return bytesNeeded + 1;
    }

    assert(bufferLength > bytesNeeded);

    // Mask the last byte
    size_t relevantBits = this->_bitCount % 8;
    std::byte mask = std::byte{ 0xFF } >> (8 - relevantBits);

    sprintf(buffer, "%02hhx", this->_bytes[lastByte] & mask);

    for (size_t i = 0; i < lastByte; i++) {
        sprintf(buffer + 2 * (i + 1), "%02hhx", this->_bytes[lastByte - i - 1]);
    }

    buffer[bytesNeeded] = '\0';

    return bytesNeeded + 1;
}

void BitVector::FromStringHex(const char* buffer, size_t bufferLength) {
    assert(buffer != nullptr);
    assert(bufferLength >= 2);

    this->SetBitCount(bufferLength / 2 * 8);

    for (size_t i = 0; i < this->_bytesCount; i++) {
        sscanf(buffer + 2 * i, "%02hhx", &this->_bytes[this->_bytesCount - i - 1]);
    }
}

void BitVector::WriteToStream(std::ostream& out) const {
    for (size_t i = 0; i < this->_bitCount; i++) {
        out << (this->Get(this->_bitCount - i - 1) ? '1' : '0');
    }
}

void BitVector::WriteToStreamHex(std::ostream& out) const {
    if (this->_bytesCount == 0) {
        return;
    }

    // Mask the last byte
    size_t lastByte = this->_bitCount / 8;
    size_t relevantBits = this->_bitCount % 8;
    std::byte mask = std::byte{ 0xFF } >> (8 - relevantBits);

    out << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(this->_bytes[lastByte] & mask);

    for (size_t i = 0; i < lastByte; i++) {
        out << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(this->_bytes[lastByte - i - 1]);
    }
}

std::ostream& panga::operator<<(std::ostream& out, const BitVector& bv) {
    bv.WriteToStream(out);
    return out;
}

BitVector::HexFormatWrapper panga::operator<<(std::ostream& out, const BitVector::HexFormat_t&) {
    return {out};
}

std::ostream& panga::operator<<(const BitVector::HexFormatWrapper& wrapper, const BitVector& bv) {
    std::ostream& out = wrapper._os;
    bv.WriteToStreamHex(out);
    return out;
}
