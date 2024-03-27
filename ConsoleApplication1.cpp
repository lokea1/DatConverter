#include "xor.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <cctype>

using namespace std;

int XorCoS = 0;
int XorS = 0;

vector<unsigned char> lineBytes;
streampos previousPosition;

int DecConvertToInt(const vector<unsigned char>& bytesToConvert) {
    vector<unsigned char> cstr(4, 0);
    for (int i = 0; i < 4; ++i) {
        if (XorCoS > 1012) {
            XorCoS = XorCoS - 1013;
        }
        cstr[i] = ~bytesToConvert[i];
        cstr[i] ^= xorKeys[XorCoS];
        cstr[i] += 0x49;
        XorCoS++;
    }
    int result = (cstr[0] << 0) | (cstr[1] << 8) | (cstr[2] << 16) | (cstr[3] << 24);
    return result;
}

void DecCharacters(const vector<unsigned char>& bytes, int startIndex, int length, ofstream& outputFile) {
    vector<unsigned char> decryptedChar(length, 0);
    int j = 0;
    for (int i = startIndex; i < startIndex + length; ++i) {
        if (XorS > 1012) {
            XorS = XorS - 1013;
        }
        decryptedChar.at(j) = ((~bytes[i] ^ xorKeys[XorS])) + 0x49;
        j++;
        XorS++;
    }
    XorS += 4;

    decryptedChar.push_back(0x0A);

    for (int i = 0; i < decryptedChar.size(); ++i) {
        outputFile << decryptedChar[i];
    }
}

void EncConvertToByte(int value, ofstream& outputFile) {
    vector<unsigned char> bytesCoS(4);
    bytesCoS.at(0) = (value >> 0) & 0xFF;
    bytesCoS.at(1) = (value >> 8) & 0xFF;
    bytesCoS.at(2) = (value >> 16) & 0xFF;
    bytesCoS.at(3) = (value >> 24) & 0xFF;

    for (int i = 0; i < 4; ++i) {
        if (XorCoS > 1012) {
            XorCoS = XorCoS - 1013;
        }

        bytesCoS.at(i) = ~((bytesCoS.at(i) - 0x49) ^ xorKeys[XorCoS]);
        XorCoS++;
    }
    for (int i = 0; i < 4; i++) {
        outputFile << bytesCoS.at(i);
    }
}

void EncCharacters(int length, ofstream& outputFile) {
    for (int i = 0; i < length; ++i) {
        if (XorS > 1012) {
            XorS = XorS - 1013;
        }
        lineBytes.at(i) = ~((lineBytes.at(i) - 0x49) ^ xorKeys[XorS]);
        XorS++;
    }
    XorS += 4;

    for (int i = 0; i < lineBytes.size(); i++) {
        outputFile << lineBytes.at(i);
    }
}

int readLineBytesUntilCrLf(ifstream& file) {
    previousPosition = file.tellg();

    lineBytes.clear();
    int bytesRead = 0;

    unsigned char byte;
    while (file.read(reinterpret_cast<char*>(&byte), sizeof(unsigned char))) {
        bytesRead++;

        if (byte == 0x0D) {
            if (file.peek() == 0x0A) {
                file.ignore();
                return bytesRead - 1;
            }
        }

        if (byte != 0x0D) {
            lineBytes.push_back(byte);
        }
    }
    return bytesRead - 1;
}

string toLower(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return tolower(c); });
    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 2 && argc != 3) {
        cout << "==========================================" << endl;
            cout << "             CF DAT Converter" << endl;
            cout << "                By: lokea" << endl;
            cout <<  "==========================================" << endl;
        cout << "                 Usage: " << endl;
        cout << "Dat_Converter inputfile.dat outputfile.lta " << endl;
        cout << "Dat_Converter inputfile.lta outputfile.dat " << endl;
        cout << "Or just transfer the file to this program " << endl;
        system("pause");
        return 1;
    }

    string inputFileName = argv[1]; 
    string outputFileName; 

    string inputExtension = inputFileName.substr(inputFileName.find_last_of(".") + 1); 

    bool decrypt = false;
    bool encrypt = false;

    if (argc == 2) {
        if (toLower(inputExtension) == "dat") {
            outputFileName = inputFileName.substr(0, inputFileName.size() - 4) + ".lta";
            decrypt = true;
        }
        else if (toLower(inputExtension) == "lta") {
            outputFileName = inputFileName.substr(0, inputFileName.size() - 4) + ".dat";
            encrypt = true;
        }
        else {
            cerr << "Invalid file format." << endl;
            return 1;
        }
    }
    else {
        string outputExtension;
        if (toLower(inputExtension) == "dat") {
            outputExtension = "lta";
            decrypt = true;
        }
        else if (toLower(inputExtension) == "lta") {
            outputExtension = "dat";
            encrypt = true;
        }
        else {
            cerr << "Invalid file format." << endl;
            return 1;
        }

        outputFileName = argv[2]; // Имя выходного файла
        string outputExtensionGiven = outputFileName.substr(outputFileName.find_last_of(".") + 1); // Расширение выходного файла

        if (toLower(outputExtensionGiven) != outputExtension) {
            cerr << "Output file format must be ." << outputExtension << endl;
            return 1;
        }
    }

    if (decrypt) {
        ifstream file(inputFileName, ios::binary);
        if (!file.is_open()) {
            cout << "Could not open file " << inputFileName << endl;
            return 1;
        }

        ofstream outputFile(outputFileName);
        if (!outputFile.is_open()) {
            cout << "Could not create output file." << endl;
            return 1;
        }

        while (true) {
            vector<unsigned char> bytesToConvert(4);
            file.read(reinterpret_cast<char*>(bytesToConvert.data()), 4);
            if (!file) {
                if (file.eof()) {
                    break;
                }
                else {
                    cout << "Error reading file." << endl;
                    return 1;
                }
            }

            int length = DecConvertToInt(bytesToConvert);
            vector<unsigned char> encryptedString(length);
            file.read(reinterpret_cast<char*>(encryptedString.data()), length);
            if (file.gcount() != length) {
                cout << "Error reading file: unexpected number of bytes." << endl;
                return 1;
            }
            DecCharacters(encryptedString, 0, length, outputFile);
        }
        file.close();
        outputFile.close();
    }

    if (encrypt) {
        ifstream file(inputFileName, ios::binary);
        if (!file.is_open()) {
            cout << "Could not open file " << inputFileName << endl;
            return 1;
        }

        ofstream outputFile(outputFileName, ios::binary);
        if (!outputFile.is_open()) {
            cout << "Could not create output file." << endl;
            return 1;
        }

        while (true) {
            unsigned int CoStr = readLineBytesUntilCrLf(file);
            if (file.eof()) {
                break;
            }

            EncConvertToByte(CoStr, outputFile);
            EncCharacters(CoStr, outputFile);
        }
        file.close();
        outputFile.close();
    }

    return 0;
}

