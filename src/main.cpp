#include "mr_parser/mr_parser.h"
#include <iostream>
using namespace std;

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " file.mr\n";
    }
    cout << argv[1] << endl;
    Node *root = LoadFromFile(argv[1]);
    root->printRecursively();
    Node *node = root->getChildByPath("objects/teleporter/instances/startup/south");
    if (node == nullptr)
    {
        cout << "NOT FOUND";
    }
    else
    {
        auto &data = node->getAttributeByName("pos")->data;
        int16_t x = GetIntFromBuffer<int16_t>(data, 0);
        int16_t y = GetIntFromBuffer<int16_t>(data, 2);
        int16_t z = GetIntFromBuffer<int16_t>(data, 4);
        cout << x << " x " << y << " x " << z << endl;
        x = 600;
        data[0] = x & 0xFF;
        data[1] = ((x >> 8) & 0xFF);
    }
    char tmp[1000];
    strcpy(tmp, argv[1]);
    strcat(tmp, "_");
    SaveToFile(root, tmp);
}