#pragma once
#ifndef Encrypter_H
#define Encrypter_H

#include <iostream>
#include <fstream>
#include <vector>
#include <openssl/evp.h>

bool encryptFile(const std::wstring& filePath, std::vector<unsigned char>& ciphertext, std::vector<unsigned char>& tag, std::vector<unsigned char>& nonce);

#endif
