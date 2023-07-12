#include "mr_parser/namable.h"

void Namable::loadName(const vector<byte_t> &buffer, size_t offset, int stringDataOffset)
{
  size_t nameLength = buffer[offset];

  for (size_t i = 0; i < nameLength; i++)
  {
    const byte_t letter = buffer[offset + stringDataOffset + i];
    if (!letter)
    {
      break;
    }
    name.push_back(letter);
  }
}

size_t Namable::getNameLengthInBuffer() const
{
  return (name.length() % 4) ? (name.length() / 4 * 4 + 4) : (name.length() + 4);
}

void Namable::appendNameToFile(vector<byte_t> &data) const
{
  for (size_t i = 0; i < getNameLengthInBuffer(); i++)
  {
    if (i < name.length())
    {
      data.push_back(name[i]);
    }
    else if (i == name.length())
    {
      data.push_back(0);
    }
    else
    {
      data.push_back(0xFD);
    }
  }
}