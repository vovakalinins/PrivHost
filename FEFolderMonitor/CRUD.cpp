#include <curl/curl.h>
#include <Windows.h>
#include <string>
#include <iostream>
#include <codecvt>

long getFileSize(FILE* file) {
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	return size;
}

bool uploadFile(const std::wstring& filePath)
{
    CURL* curl;
    CURLcode res;
    struct curl_httppost* form = nullptr;
    struct curl_httppost* lastptr = nullptr;

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string filePathStr = converter.to_bytes(filePath);

    curl = curl_easy_init();
    if (curl) {
        std::string url = "http://127.0.0.1:8080/upload";

        curl_formadd(&form, &lastptr,
            CURLFORM_COPYNAME, "file",
            CURLFORM_FILE, filePathStr.c_str(),
            CURLFORM_END);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, form);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Upload failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            curl_formfree(form);
            return false;
        }

        // Cleanup yayy
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