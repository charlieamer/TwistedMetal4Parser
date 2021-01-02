#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <zlib.h>
using namespace std;

typedef uint8_t byte_t;

template<typename T>
void AppendToBuffer(vector<byte_t>& data, T value) {
    byte_t buffer[sizeof(value)];
    memcpy(buffer, &value, sizeof(value));
    for (int i=0; i<sizeof(value); i++) {
        data.push_back(buffer[i]);
    }
}

struct Namable {
    string name;
    size_t nameLength;
protected:
    void loadName(const vector<byte_t>& buffer, size_t offset, int stringDataOffset) {
        nameLength = buffer[offset];

        for (size_t i=0; i<nameLength; i++) {
            const byte_t letter = buffer[offset + stringDataOffset + i];
            if (!letter) {
                break;
            }
            name.push_back(letter);
        }
    }
    size_t getNameLengthInBuffer() const {
        return (name.length() % 4) ? (name.length() / 4 * 4 + 4) : (name.length() + 4);
    }
    void appendNameToFile(vector<byte_t>& data) const {
        for (size_t i=0; i<getNameLengthInBuffer(); i++) {
            if (i < name.length()) {
                data.push_back(name[i]);
            } else if (i == name.length()) {
                data.push_back(0);
            } else {
                data.push_back(0xFF);
            }
        }
    }
};

struct Component : public Namable {
    uint32_t lengthInBuffer;
    uint32_t unpackedZlibLength;
    vector<byte_t> data;

    Component(const vector<byte_t>& buffer, size_t offset) {
        loadName(buffer, offset, 12);
        lengthInBuffer = *((uint32_t*)((void*)(buffer.data() + offset + 4)));
        unpackedZlibLength = *((uint32_t*)((void*)(buffer.data() + offset + 8)));
        if (unpackedZlibLength) {
            assginDataFromZlib(buffer, offset);
        } else {
            assignDataFromBuffer(buffer, offset);
        }
    }

    int bufferSize() const {
        return lengthInBuffer + nameLength + 12;
    }

    void appendToFile(vector<byte_t>& fileBuffer) const {
        fileBuffer.push_back((byte_t)getNameLengthInBuffer());
        fileBuffer.push_back(0xFF);
        fileBuffer.push_back(0xFF);
        fileBuffer.push_back(0xFF);

        uLongf sizeDataCompressed = (uLongf)((data.size() * 1.1) + 12);
        byte_t* dataCompressed = (byte_t*)new byte_t[sizeDataCompressed];
        if (compress(dataCompressed, &sizeDataCompressed, data.data(), data.size()) != Z_OK) {
            throw runtime_error("Error compressing data");
        }
        
        if (sizeDataCompressed <= data.size()) {
            AppendToBuffer<uint32_t>(fileBuffer, sizeDataCompressed);
            AppendToBuffer<uint32_t>(fileBuffer, data.size());
            appendNameToFile(fileBuffer);
            for (uLongf i=0; i<sizeDataCompressed; i++) {
                fileBuffer.push_back(dataCompressed[i]);
            }
        } else {
            AppendToBuffer<uint32_t>(fileBuffer, data.size());
            AppendToBuffer<uint32_t>(fileBuffer, 0);
            appendNameToFile(fileBuffer);
            for (size_t i=0; i<data.size(); i++) {
                fileBuffer.push_back(data[i]);
            }
        }
        delete[] dataCompressed;
    }
private:
    void assginDataFromZlib(const vector<byte_t>& buffer, size_t offset) {
        Bytef* uncompressedData = new Bytef[unpackedZlibLength];
        uLongf unpackedZlibLengthL = unpackedZlibLength;
        int result = uncompress(
            uncompressedData,
            &unpackedZlibLengthL,
            (const Bytef*)(buffer.data() + offset + nameLength + 12),
            lengthInBuffer
        );
        if (result != Z_OK) {
            throw domain_error("Error decompressing buffer");
        }
        data.assign(uncompressedData, uncompressedData + unpackedZlibLengthL);
        delete[] uncompressedData;
    }

    void assignDataFromBuffer(const vector<byte_t>& buffer, size_t offset) {
        data.assign(
            buffer.begin() + offset + nameLength + 12,
            buffer.begin() + offset + nameLength + 12 + lengthInBuffer
        );
    }
};

struct Node : public Namable {
    int numChildren;
    int numAttributes;
    vector<Component> components;
    vector<Node> children;
    Node(const vector<byte_t>& buffer, size_t offset = 0) {
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
    void printRecursively(int depth = 0) const {
        for (int i=0; i<depth; i++) {
            cout << "  ";
        }
        cout << name << " cmp: ";
        for (size_t i=0; i<components.size(); i++) {
            cout << components[i].name;
            if (components[i].unpackedZlibLength) {
                cout << "(" << components[i].lengthInBuffer << "," << components[i].data.size() << "), ";
            } else {
                cout << "(" << components[i].data.size() << "), ";
            }
        }
        cout << endl;
        for (size_t i=0; i<children.size(); i++) {
            children[i].printRecursively(depth + 1);
        }
    }
    int bufferSize() const {
        int ret = 4 + nameLength;
        for (size_t i=0; i<components.size(); i++) {
            ret += components[i].bufferSize();
        }
        for (size_t i=0; i<children.size(); i++) {
            ret += children[i].bufferSize();
        }
        return ret;
    }
    void appendToFile(vector<byte_t>& data) const {
        data.push_back((byte_t)getNameLengthInBuffer());
        data.push_back((byte_t)children.size());
        data.push_back((byte_t)components.size());
        data.push_back(0);
        appendNameToFile(data);
        for (auto& attribute : components) {
            attribute.appendToFile(data);
        }
        for (auto& child : children) {
            child.appendToFile(data);
        }
    }
};

Node* LoadFromFile(const char* path) {
    ifstream infile(path, ios::binary);

    infile.seekg(0, std::ios::end);
    size_t length = (size_t)infile.tellg();
    infile.seekg(0, std::ios::beg);

    vector<byte_t> buffer;
    buffer.insert(buffer.begin(), istreambuf_iterator<char>(infile), istreambuf_iterator<char>());
    if (buffer[0] != 0x67 || buffer[1] != 0x00 || buffer[2] != 0xCC || buffer[3] != 0xCC) {
        throw runtime_error("File magic header doesn't match");
    }
    buffer.erase(buffer.begin(), buffer.begin() + 4);
    cout << length << " bytes\n";

    return new Node(buffer);
}

void SaveToFile(Node* node, const char* fileName) {
    ofstream out(fileName, ios::binary);
    vector<byte_t> data;
    // Magic header
    data.push_back(0x67);
    data.push_back(0x00);
    data.push_back(0xCC);
    data.push_back(0xCC);
    // other data
    node->appendToFile(data);
    for (const char c : data) {
        out << c;
    }
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " file.mr\n";
    }
    cout << argv[1] << endl;
    Node* root = LoadFromFile(argv[1]);
    root->printRecursively();
}