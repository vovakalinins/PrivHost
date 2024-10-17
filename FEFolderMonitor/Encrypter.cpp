#include <iostream>
#include <fstream>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>
unsigned char encryptionKey[32];

bool encryptFile(const std::wstring& filePath, std::vector<unsigned char>& ciphertext, std::vector<unsigned char>& tag, std::vector<unsigned char>& nonce) {
    memcpy(encryptionKey, "ifyouseethisyouhavetosubscribeee", 32);

    std::wcout << L"Attempting to open file: " << filePath << std::endl;

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for encryption. Check file path or permissions." << std::endl;
        return false;
    }

    std::vector<unsigned char> plaintext((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        std::cerr << "Failed to create encryption context." << std::endl;
        return false;
    }

    nonce.resize(12);
    if (!RAND_bytes(nonce.data(), nonce.size())) {
        std::cerr << "Failed to generate nonce." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, encryptionKey, nonce.data()) != 1) {
        std::cerr << "Failed to initialize encryption." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    int len;
    ciphertext.resize(plaintext.size());
    if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext.size()) != 1) {
        std::cerr << "Failed to encrypt data." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    int ciphertext_len = len;
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len) != 1) {
        std::cerr << "Failed to finalize encryption." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    ciphertext_len += len;

    ciphertext.resize(ciphertext_len);

    tag.resize(16);
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data()) != 1) {
        std::cerr << "Failed to get GCM tag." << std::endl;
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    EVP_CIPHER_CTX_free(ctx);

    return true;
}

