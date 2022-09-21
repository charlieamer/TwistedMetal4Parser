#pragma once
#include "types.h"
#include <string>
#include <vector>
using namespace std;

struct Namable
{
  string name;

protected:
  void loadName(const vector<byte_t> &buffer, size_t offset, int stringDataOffset);
  size_t getNameLengthInBuffer() const;
  void appendNameToFile(vector<byte_t> &data) const;
};