#include "mr_parser/mr_parser.h"
#include <iostream>
using namespace std;

int main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " file.mr\n";
        return 1;
    }
    Node *root = LoadFromFile(argv[1]);
    root->printRecursively();
}