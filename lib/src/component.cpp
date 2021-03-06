#include <stdexcept>
#include <zlib.h>
#include "mr_parser/component.h"
#include "mr_parser/utils.h"

Component::Component(const vector<byte_t> &buffer, size_t offset)
{
  loadName(buffer, offset, 12);
  lengthInBuffer = *((uint32_t *)((void *)(buffer.data() + offset + 4)));
  unpackedZlibLength = *((uint32_t *)((void *)(buffer.data() + offset + 8)));
  if (unpackedZlibLength)
  {
    assginDataFromZlib(buffer, offset);
  }
  else
  {
    assignDataFromBuffer(buffer, offset);
  }
}

int Component::bufferSize() const
{
  return lengthInBuffer + nameLength + 12;
}

void Component::appendToFile(vector<byte_t> &fileBuffer) const
{
  fileBuffer.push_back((byte_t)getNameLengthInBuffer());
  fileBuffer.push_back(0xFF);
  fileBuffer.push_back(0xFF);
  fileBuffer.push_back(0xFF);

  uLongf sizeDataCompressed = (uLongf)((data.size() * 1.1) + 12);
  byte_t *dataCompressed = (byte_t *)new byte_t[sizeDataCompressed];
  if (compress(dataCompressed, &sizeDataCompressed, data.data(), data.size()) != Z_OK)
  {
    throw runtime_error("Error compressing data");
  }

  if (sizeDataCompressed <= data.size())
  {
    AppendToBuffer<uint32_t>(fileBuffer, sizeDataCompressed);
    AppendToBuffer<uint32_t>(fileBuffer, data.size());
    appendNameToFile(fileBuffer);
    for (uLongf i = 0; i < sizeDataCompressed; i++)
    {
      fileBuffer.push_back(dataCompressed[i]);
    }
  }
  else
  {
    AppendToBuffer<uint32_t>(fileBuffer, data.size());
    AppendToBuffer<uint32_t>(fileBuffer, 0);
    appendNameToFile(fileBuffer);
    for (size_t i = 0; i < data.size(); i++)
    {
      fileBuffer.push_back(data[i]);
    }
  }
  delete[] dataCompressed;
}

void Component::assginDataFromZlib(const vector<byte_t> &buffer, size_t offset)
{
  Bytef *uncompressedData = new Bytef[unpackedZlibLength];
  uLongf unpackedZlibLengthL = unpackedZlibLength;
  int result = uncompress(
      uncompressedData,
      &unpackedZlibLengthL,
      (const Bytef *)(buffer.data() + offset + nameLength + 12),
      lengthInBuffer);
  if (result != Z_OK)
  {
    throw domain_error("Error decompressing buffer");
  }
  data.assign(uncompressedData, uncompressedData + unpackedZlibLengthL);
  delete[] uncompressedData;
}

void Component::assignDataFromBuffer(const vector<byte_t> &buffer, size_t offset)
{
  data.assign(
      buffer.begin() + offset + nameLength + 12,
      buffer.begin() + offset + nameLength + 12 + lengthInBuffer);
}