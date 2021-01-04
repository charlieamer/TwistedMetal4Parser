#include <vector>
#include <string>
using namespace std;
#include "types.h"

template <typename T>
void AppendToBuffer(vector<byte_t> &data, T value)
{
  byte_t buffer[sizeof(value)];
  memcpy(buffer, &value, sizeof(value));
  for (int i = 0; i < sizeof(value); i++)
  {
    data.push_back(buffer[i]);
  }
}

template <typename T>
T GetIntFromBuffer(const vector<byte_t> &data, size_t offset = 0)
{
  T ret;
  memcpy(&ret, data.data() + offset, sizeof(T));
  return ret;
}

vector<string> splitString(string str, string sep);