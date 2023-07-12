#include <iostream>
#include <fstream>
#include "mr_parser/utils.h"
#include "mr_parser/node.h"

Node::Node(const vector<byte_t> &buffer, size_t offset)
{
  loadName(buffer, offset, 4);
  uint8_t numChildren = buffer[offset + 1];
  uint16_t numAttributes;
  memcpy(&numAttributes, buffer.data() + offset + 2, sizeof(numAttributes));
  offset += getNameLengthInBuffer() + 4;

  for (int i = 0; i < numAttributes; i++)
  {
    Component componentToPush = Component(buffer, offset);
    offset += componentToPush.bufferSize();
    components.push_back(componentToPush);
  }

  for (int i = 0; i < numChildren; i++)
  {
    Node childToPush = Node(buffer, offset);
    offset += childToPush.bufferSize();
    children.push_back(childToPush);
  }
}

Node::Node(string nodeName) {
  name = nodeName;
}

void Node::printRecursively(int depth) const
{
  for (int i = 0; i < depth; i++)
  {
    cout << "  ";
  }
  cout << name << " cmp: ";
  for (size_t i = 0; i < components.size(); i++)
  {
  //   cout << components[i].name;
  //   if (components[i].unpackedZlibLength)
  //   {
  //     cout << "(" << components[i].lengthInBuffer << "," << components[i].data.size() << "), ";
  //   }
  //   else
  //   {
      cout << "(" << components[i].data.size() << "), ";
    // }
  }
  cout << endl;
  for (size_t i = 0; i < children.size(); i++)
  {
    children[i].printRecursively(depth + 1);
  }
}

int Node::bufferSize() const
{
  int ret = 4 + getNameLengthInBuffer();
  for (size_t i = 0; i < components.size(); i++)
  {
    ret += components[i].bufferSize();
  }
  for (size_t i = 0; i < children.size(); i++)
  {
    ret += children[i].bufferSize();
  }
  return ret;
}

void Node::appendToFile(vector<byte_t> &data) const
{
  data.push_back((byte_t)getNameLengthInBuffer());
  data.push_back((byte_t)children.size());
  uint16_t numComponents = components.size();
  data.push_back((byte_t)(numComponents & 0xff));
  data.push_back((byte_t)((numComponents >> 8) & 0xff));
  appendNameToFile(data);
  for (auto &attribute : components)
  {
    attribute.appendToFile(data);
  }
  for (auto &child : children)
  {
    child.appendToFile(data);
  }
}

Node *Node::getChildByPath(vector<string> splitted)
{
  if (splitted.size() == 0)
  {
    return nullptr;
  }
  for (auto &child : children)
  {
    if (child.name == splitted[0])
    {
      if (splitted.size() == 1)
      {
        return &child;
      }
      else
      {
        return child.getChildByPath(vector<string>(splitted.begin() + 1, splitted.end()));
      }
    }
  }
  return nullptr;
}

Node *Node::getChildByPath(string path)
{
  return getChildByPath(splitString(path, "/"));
}

Component *Node::getAttributeByName(string name)
{
  for (auto &component : components)
  {
    if (component.name == name)
    {
      return &component;
    }
  }
  return nullptr;
}

Node *LoadFromFile(const char *path, bool printSize)
{
  auto buffer = loadFileToBuffer(path);
  if (buffer.size() == 0) {
    throw runtime_error("Empty file");
  }
  if (buffer[0] != 0x67 || buffer[1] != 0x00 ||
      (buffer[2] != 0xCC && buffer[2] != 0x00) ||
      (buffer[3] != 0xCC && buffer[3] != 0x00))
  {
    throw runtime_error("File magic header doesn't match");
  }
  buffer.erase(buffer.begin(), buffer.begin() + 4);
  if (printSize)
    cout << buffer.size() << " bytes\n";

  return new Node(buffer);
}

void SaveToFile(Node *node, const char *fileName)
{
  ofstream out(fileName, ios::binary);
  vector<byte_t> data;
  // Magic header
  data.push_back(0x67);
  data.push_back(0x00);
  data.push_back(0xCC);
  data.push_back(0xCC);
  // other data
  node->appendToFile(data);
  writeBytesToFile(out, data);
}