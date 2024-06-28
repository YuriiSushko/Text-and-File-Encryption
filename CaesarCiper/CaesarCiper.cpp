#include <Windows.h>
#include <minwinbase.h>
#include <libloaderapi.h>
#include <handleapi.h>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <tuple>
#include <stack>
#include <iostream>
using namespace std;

typedef char* (__stdcall* encrypt_ptr)(char*, int);
typedef char* (__stdcall* decrypt_ptr)(char*, int);

struct CharPointer {
    int p;
    char ch;
};

class CaesarCiper {
private:
    HINSTANCE handle;
    encrypt_ptr encrypt;
    decrypt_ptr decrypt;
public:

    CaesarCiper(const char* DLLCaesar) : handle(nullptr), encrypt(nullptr), decrypt(nullptr)
    {
        handle = LoadLibraryA(DLLCaesar);
        if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
            cerr << "Failed to load DLL" << endl;
            return;
        }

        encrypt = (encrypt_ptr)GetProcAddress(handle, "ToEncrypt");
        if (encrypt == nullptr) {
            DWORD error = GetLastError();
            cerr << "Failed to find 'ToEncrypt' function: " << error << endl;
            FreeLibrary(handle);
            return;
        }

        decrypt = (decrypt_ptr)GetProcAddress(handle, "ToDecrypt");
        if (decrypt == nullptr) {
            DWORD error = GetLastError();
            cerr << "Failed to find 'ToDecrypt' function: " << error << endl;
            FreeLibrary(handle);
            return;
        }
    }

    CaesarCiper() : handle(nullptr), encrypt(nullptr), decrypt(nullptr) {}

    ~CaesarCiper()
    {
        if (handle != nullptr && handle != INVALID_HANDLE_VALUE) {
            FreeLibrary(handle);
        }
    }

    char* Encrypt(char* text, int key) {
        if (encrypt != nullptr) {
            return encrypt(text, key);
        }
        return nullptr;
    }

    char* Decrypt(char* text, int key) {
        if (decrypt != nullptr) {
            return decrypt(text, key);
        }
        return nullptr;
    }
};

class Cursor {
private:
    int* size;
    static int cursor_position;
public:
    Cursor() : size(nullptr) {}
    Cursor(int* size) : size(size) { }

    int move_right() {
        if (cursor_position < *size) {
            cursor_position++;
        }
        return cursor_position;
    }

    int move_left() {
        if (cursor_position > 0) {
            cursor_position--;
        }
        return cursor_position;
    }

    int get_position() {
        return cursor_position;
    }

    void set_cursor_position(tuple<int, int> position, CharPointer* dynamic_array) {
        int line = 0;
        int column = 0;

        for (int i = 0; i < *size; i++) {

            if (dynamic_array[i].ch == '\n') {
                line++;
            }

            column++;

            if (line == get<0>(position)) {
                int sub_column = 0;
                while (sub_column != get<1>(position)) {
                    sub_column++;
                    column++;
                }

                if (sub_column == get<1>(position)) {
                    if (get<0>(position) == 0) {
                        cursor_position = column - 1;
                    }
                    else
                    {
                        cursor_position = column;
                    }
                    break;
                }
                else
                {
                    cout << "There is no such column in line\n";
                    break;
                }

            }
        }
        if (line != get<0>(position)) {
            cout << "There is no such position in text\n";
        }
    }

};


int Cursor::cursor_position = -1;

class Text
{
private:
    CharPointer* dynamicArray;
    size_t capacity;
    int length;
    Cursor cursor;

public:
    Text() : capacity(64), length(0) {
        dynamicArray = (CharPointer*)calloc(capacity, sizeof(CharPointer));
        if (!dynamicArray) {
            perror("Failed to allocate");
            exit(EXIT_FAILURE);
        }
    }

    ~Text() {
        free(dynamicArray);
    }

    char* get() const {
        char* string = (char*)calloc(capacity, sizeof(char));
        for (int i = 0; i <= length; i++) {
            string[i] = dynamicArray[i].ch;
        }
        return string;
    }

    int getLength() const {
        return length;
    }

    int getCapacity() const{
        return static_cast<int>(capacity);
    }

    void to_realloc()
    {
        capacity *= 2;

        CharPointer* tempArray = (CharPointer*)realloc(dynamicArray, capacity * sizeof(CharPointer));
        if (!tempArray) {
            perror("Failed to reallocate");
            free(dynamicArray);
            exit(EXIT_FAILURE);
        }
        dynamicArray = tempArray;
    }

    void append(bool ifItNewLine, int start_position = -1) {
        int ch;
        static int index = 0;

        if (start_position != -1) {
            int ch;
            cout << "Enter text to insert:\n";
            while (!ifItNewLine && (ch = getchar()) != '\n') {
                dynamicArray[start_position++].ch = ch;
                Cursor cursor(&length);
                cursor.move_right();
            }
            return;
        }

        while (true) {
            if (ifItNewLine) {
                ch = '\n';
            }
            else {
                ch = getchar();
            }

            if (!ifItNewLine && ch == '\n') {
                break;
            }

            if (index < static_cast<int>(capacity) - 1) {
                dynamicArray[index].ch = ch;
                index++;
                dynamicArray[index].p = dynamicArray[index - 1].p + 1;
            }
            else {
                to_realloc();
                dynamicArray[index].ch = ch;
                index++;
                dynamicArray[index].p = dynamicArray[index - 1].p + 1;
            }

            if (ifItNewLine) {
                length = index;
                dynamicArray[index].p = 0;
                return;
            }
        }

        dynamicArray[index].ch = '\0';
        length = index;
    }

};


class TextEditor {
private:
    Text text;
    Cursor cursor;
    CaesarCiper ciper;
public:
    TextEditor() {
        CaesarCiper caesar("D:\\Programming paradigms\\Caesar_Encryption_DLL\\DLLCaesar\\x64\\Debug\\DLLCaesar.dll");
        int length = text.getLength();
    }

    void print(int cursor_position = -1) const {
        char* string = text.get();
        int length = text.getLength();

        for (int i = 0; i < length; i++) {
            if (cursor_position == i) {
                cout << "|";
            }
            cout << string[i];
        }
        if (cursor_position == length) {
            cout << "|";
        }
        cout << endl;
    }

    Text loadFromFile(const char* filename) {
        Text loadedFile;

        ifstream file(filename);
        if (file.is_open()) {
            char buffer[256];
            while (file.getline(buffer, sizeof(buffer))) {
                loadedFile.append(buffer);
            }
            file.close();
        }
        else {
            std::cerr << "Unable to open file: " << filename << std::endl;
        }

        return loadedFile;
    }

    void saveToFile(const char* filename, const Text& text) {
        ofstream file(filename);
        if (file.is_open()) {
            char* string = text.get();
            int length = text.getLength();

            for (int i = 0; i < length; ++i) {
                file << string[i];
            }

            file.close();
            std::cout << "Text saved to file: " << filename << std::endl;
        }
        else {
            std::cerr << "Unable to create file: " << filename << std::endl;
        }
    }


};


int main()
{
    CaesarCiper caesar("D:\\Programming paradigms\\Caesar_Encryption_DLL\\DLLCaesar\\x64\\Debug\\DLLCaesar.dll");

    char message[] = "HELLO everyone this is DLL work";
    int key = 2;

    char* encryptedMessage = caesar.Encrypt(message, key);
    cout << "Encrypted Message: " << encryptedMessage << endl;

    char* decryptedMessage = caesar.Decrypt(encryptedMessage, key);
    cout << "Decrypted Message: " << decryptedMessage << endl;

    return 0;
}
