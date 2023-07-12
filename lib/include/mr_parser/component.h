#pragma once
#include "namable.h"
#include <vector>
#include <stdexcept>
#include <string.h>
using namespace std;

struct Component : public Namable
{
private:
  uint32_t lengthInBuffer;
public:
  
  // 2 - string
  // 3 - binary data
  // 4 - uint32_t
  // 5 - vec3
  // 6 - *.mod (extra executable code)
  // 11 - bool
  // 13 - int32_t
  // 14 - color
  // 15 - 2x vec3
  // 16 - *.vab (sound)
  // 17 - worldData
  // 18 - weapProbArray
  // 21 - difficultyFactorArray
  // 26 - array[vec3]
  uint8_t componentType = 0xff;
  uint16_t typeParameter = 0xffff; // such as array length, etc. When unused it is 0xff 0xff

  vector<byte_t> data;

  Component(const vector<byte_t> &buffer, size_t offset);
  Component(string componentName);

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
  void assginDataFromZlib(const vector<byte_t> &buffer, size_t offset, uint32_t unpackedZlibLength);
  void assignDataFromBuffer(const vector<byte_t> &buffer, size_t offset);
};
