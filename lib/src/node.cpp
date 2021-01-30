#include <iostream>
#include <fstream>
#include "mr_parser/utils.h"
#include "mr_parser/node.h"

Node::Node(const vector<byte_t> &buffer, size_t offset)
{
  loadName(buffer, offset, 4);
  numChildren = buffer[offset + 1];
  memcpy(&numAttributes, buffer.data() + offset + 2, sizeof(numAttributes));
  offset += nameLength + 4;

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

void Node::printRecursively(int depth) const
{
  for (int i = 0; i < depth; i++)
  {
    cout << "  ";
  }
  cout << name << " cmp: ";
  for (size_t i = 0; i < components.size(); i++)
  {
    cout << components[i].name;
    if (components[i].unpackedZlibLength)
    {
      cout << "(" << components[i].lengthInBuffer << "," << components[i].data.size() << "), ";
    }
    else
    {
      cout << "(" << components[i].data.size() << "), ";
    }
  }
  cout << endl;
  for (size_t i = 0; i < children.size(); i++)
  {
    children[i].printRecursively(depth + 1);
  }
}

int Node::bufferSize() const
{
  int ret = 4 + nameLength;
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
  data.push_back((byte_t)components.size());
  data.push_back(0);
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

Node *LoadFromFile(const char *path)
{
  ifstream infile(path, ios::binary);

  if (infile.bad()) {
    throw runtime_error("Unable to open file");
  }

  infile.seekg(0, std::ios::end);
  size_t length = (size_t)infile.tellg();
  infile.seekg(0, std::ios::beg);

  vector<byte_t> buffer;
  buffer.insert(buffer.begin(), istreambuf_iterator<char>(infile), istreambuf_iterator<char>());
  if (buffer[0] != 0x67 || buffer[1] != 0x00 || buffer[2] != 0xCC || buffer[3] != 0xCC)
  {
    //throw runtime_error("File magic header doesn't match");
  }
  buffer.erase(buffer.begin(), buffer.begin() + 4);
  cout << length << " bytes\n";

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
  for (const char c : data)
  {
    out << c;
  }
}