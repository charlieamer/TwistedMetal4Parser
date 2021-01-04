#pragma once
#include "namable.h"

struct Component : public Namable
{
  uint32_t lengthInBuffer;
  uint32_t unpackedZlibLength;
  vector<byte_t> data;

  Component(const vector<byte_t> &buffer, size_t offset);

  int bufferSize() const;
  void appendToFile(vector<byte_t> &fileBuffer) const;

private:
  void assginDataFromZlib(const vector<byte_t> &buffer, size_t offset);
  void assignDataFromBuffer(const vector<byte_t> &buffer, size_t offset);
};
