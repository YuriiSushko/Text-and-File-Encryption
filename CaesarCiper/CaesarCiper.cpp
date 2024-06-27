#include <Windows.h>
#include <minwinbase.h>
#include <libloaderapi.h>
#include <handleapi.h>
#include <iostream>
using namespace std;

typedef char* (__stdcall* encrypt_ptr)(char*, int);
typedef char* (__stdcall* decrypt_ptr)(char*, int);

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

        encrypt_ptr encrypt = (encrypt_ptr)GetProcAddress(handle, "ToEncrypt");
        if (encrypt == nullptr)
        {
            cout << "Function not found" << endl;
            return;
        }
        decrypt_ptr decrypt = (decrypt_ptr)GetProcAddress(handle, "ToDecrypt");
        if (decrypt == nullptr)
        {
            cout << "Function not found" << endl;
            return;
        }
    }

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



int main()
{
    CaesarCiper caesar("DLLCaesar.dll");

    char message[] = "HELLO everyone this is DLL work";
    int key = 2;

    char* encryptedMessage = caesar.Encrypt(message, key);
    cout << "Encrypted Message: " << encryptedMessage << endl;

    char* decryptedMessage = caesar.Decrypt(encryptedMessage, key);
    cout << "Decrypted Message: " << decryptedMessage << endl;

    return 0;
}
