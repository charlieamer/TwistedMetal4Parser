#pragma once
#include "component.h"
#include "namable.h"

struct Node : public Namable
{
  uint8_t numChildren;
  uint16_t numAttributes;
  vector<Component> components;
  vector<Node> children;
  Node(const vector<byte_t> &buffer, size_t offset = 0);
  void printRecursively(int depth = 0) const;
  int bufferSize() const;
  void appendToFile(vector<byte_t> &data) const;
  Node *getChildByPath(vector<string> splitted);
  Node *getChildByPath(string path);
  Component *getAttributeByName(string name);
};

Node *LoadFromFile(const char *path);
void SaveToFile(Node *node, const char *fileName);