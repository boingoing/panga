//-------------------------------------------------------------------------------------------------------
// Copyright (C) Taylor Woll and panga contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for
// full license information.
//-------------------------------------------------------------------------------------------------------

#include "Genome.h"

#include <cassert>
#include <climits>

namespace {

constexpr size_t BitsPerByte = CHAR_BIT;

}  // namespace

namespace panga {

void Genome::SetBooleanGeneCount(size_t boolean_gene_count) {
  boolean_gene_count_ = boolean_gene_count;
}

void Genome::AddBooleanGenes(size_t count) { boolean_gene_count_ += count; }

size_t Genome::GetFirstBooleanGeneBitIndex() const {
  return first_boolean_gene_bit_index_;
}

size_t Genome::GetFirstBooleanGeneIndex() const { return genes_.size(); }

size_t Genome::GetGeneCount() const {
  return genes_.size() + boolean_gene_count_;
}

size_t Genome::GetGeneStartBitIndex(size_t gene_index) const {
  assert(gene_index < GetGeneCount());

  return gene_index >= GetFirstBooleanGeneIndex()
             ? first_boolean_gene_bit_index_ + genes_.size() - gene_index
             : genes_[gene_index].start_bit_index_;
}

size_t Genome::GetGeneBitWitdh(size_t gene_index) const {
  assert(gene_index < GetGeneCount());

  return gene_index >= GetFirstBooleanGeneIndex()
             ? 1
             : genes_[gene_index].bit_width_;
}

size_t Genome::GetBooleanGeneCount() const { return boolean_gene_count_; }

size_t Genome::BitsRequired() const {
  if (genes_.empty()) {
    return boolean_gene_count_;
  }

  const Gene& gene = genes_.back();
  return gene.start_bit_index_ + gene.bit_width_ + boolean_gene_count_;
}

size_t Genome::AddGene(size_t bit_width, bool byte_align) {
  assert(bit_width != 0);

  // First gene starts at bit index 0.
  // Subsequent genes start at the previous index + the previous bit width.
  size_t bit_start_index = 0;
  if (!genes_.empty()) {
    const Gene& gene = genes_.back();
    bit_start_index = gene.start_bit_index_ + gene.bit_width_;
  }

  // Align the bits underlying the gene such that it begins and ends at a byte
  // border.
  if (byte_align) {
    // If the gene would start after a byte border, adjust the start index
    // forward to the next byte.
    const size_t bit_gap_start_index = bit_start_index % BitsPerByte;
    if (bit_gap_start_index != 0) {
      bit_start_index += BitsPerByte - bit_gap_start_index;
    }
    // If the gene would end in the middle of a byte, adjust the width such that
    // it will end at a byte border instead.
    const size_t bit_gap_width = bit_width % BitsPerByte;
    if (bit_gap_width != 0) {
      bit_width += BitsPerByte - bit_gap_width;
    }
    // Sanity check the index and width are byte-aligned.
    assert(bit_start_index % BitsPerByte == 0);
    assert(bit_width % BitsPerByte == 0);
  }

  // Put the new gene at the end of the gene vector.
  const size_t gene_index = genes_.size();
  genes_.push_back({bit_start_index, bit_width});

  // Boolean genes are always at the end of the chromosome.
  // Update the bit index for the start of the boolean genes to point just after
  // the new gene we added.
  first_boolean_gene_bit_index_ = bit_start_index + bit_width;

  return gene_index;
}

}  // namespace panga
