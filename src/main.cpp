#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <zlib.h>
using namespace std;

struct Namable {
    string name;
    int nameLength;
protected:
    void loadName(const char* buffer, size_t offset, int stringDataOffset) {
        nameLength = buffer[offset];

        for (int i=0; i<nameLength; i++) {
            const char letter = buffer[offset + stringDataOffset + i];
            if (!letter) {
                break;
            }
            name.push_back(letter);
        }
    }
};

struct Component : public Namable {
    uint32_t lengthInBuffer;
    uint32_t unpackedZlibLength;
    vector<char> data;

    Component(const char* buffer, size_t offset) {
        loadName(buffer, offset, 12);
        lengthInBuffer = *((uint32_t*)((void*)(buffer + offset + 4)));
        unpackedZlibLength = *((uint32_t*)((void*)(buffer + offset + 8)));
        if (unpackedZlibLength) {
            assginDataFromZlib(buffer, offset);
        } else {
            assignDataFromBuffer(buffer, offset);
        }
    }

    int bufferSize() {
        return lengthInBuffer + nameLength + 12;
    }
private:
    void assginDataFromZlib(const char* buffer, size_t offset) {
        Bytef* uncompressedData = new Bytef[unpackedZlibLength];
        uLongf unpackedZlibLengthL = unpackedZlibLength;
        int result = uncompress(
            uncompressedData,
            &unpackedZlibLengthL,
            (const Bytef*)(buffer + offset + nameLength + 12),
            lengthInBuffer
        );
        if (result != Z_OK) {
            throw domain_error("Error decompressing buffer");
        }
        data.assign(uncompressedData, uncompressedData + unpackedZlibLengthL);
        delete[] uncompressedData;
    }

    void assignDataFromBuffer(const char* buffer, size_t offset) {
        data.assign(
            buffer + offset + nameLength + 12,
            buffer + offset + nameLength + 12 + lengthInBuffer
        );
    }
};

struct Node : public Namable {
    int numChildren;
    int numAttributes;
    vector<Component> components;
    vector<Node> children;
    Node(const char* buffer, size_t offset = 0) {
        loadName(buffer, offset, 4);
        numChildren = buffer[offset + 1];
        numAttributes = buffer[offset + 2];
        offset += nameLength + 4;

        for (int i=0; i<numAttributes; i++) {
            Component componentToPush = Component(buffer, offset);
            offset += componentToPush.bufferSize();
            components.push_back(componentToPush);
        }

        for (int i=0; i<numChildren; i++) {
            Node childToPush = Node(buffer, offset);
            offset += childToPush.bufferSize();
            children.push_back(childToPush);
        }
    }
    void printRecursively(int depth = 0) {
        for (int i=0; i<depth; i++) {
            cout << "  ";
        }
        cout << name << " cmp: ";
        for (size_t i=0; i<components.size(); i++) {
            cout << components[i].name << ", ";
        }
        cout << endl;
        for (size_t i=0; i<children.size(); i++) {
            children[i].printRecursively(depth + 1);
        }
    }
    int bufferSize() {
        int ret = 4 + nameLength;
        for (size_t i=0; i<components.size(); i++) {
            ret += components[i].bufferSize();
        }
        for (size_t i=0; i<children.size(); i++) {
            ret += children[i].bufferSize();
        }
        return ret;
    }
};

Node* LoadFromFile(const char* path) {
    ifstream infile(path, ios::binary);

    infile.seekg(0, std::ios::end);
    size_t length = infile.tellg();
    infile.seekg(0, std::ios::beg);

    char* buffer = new char[length];
    infile.read(buffer, length);
    cout << length << " bytes\n";

    Node* ret = new Node(buffer + 4);
    delete[] buffer;
    return ret;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " file.mr\n";
    }
    cout << argv[1] << endl;
    Node* root = LoadFromFile(argv[1]);
    root->printRecursively();
}