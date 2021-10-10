#pragma once
#include "namable.h"
#include <vector>
using namespace std;

struct Component : public Namable
{
  uint32_t lengthInBuffer;
  uint32_t unpackedZlibLength;
  vector<byte_t> data;

  Component(const vector<byte_t> &buffer, size_t offset);

  int bufferSize() const;
  void appendToFile(vector<byte_t> &fileBuffer) const;

  string getDataAsString() const;
  template<typename T>
  T getDataAs() const {
    if (sizeof(T) > data.size()) {
      throw runtime_error("Data cannot fit");
    }
    T ret;
    memcpy(&ret, data.data(), sizeof(T));
    return ret;
  }
  template<typename T>
  vector<T> getDataAsVector() const {
    vector<T> ret;
    ret.resize(data.size() / sizeof(T));
    memcpy(ret.data(), data.data(), data.size());
    return ret;
  }
private:
  void assginDataFromZlib(const vector<byte_t> &buffer, size_t offset);
  void assignDataFromBuffer(const vector<byte_t> &buffer, size_t offset);
};
