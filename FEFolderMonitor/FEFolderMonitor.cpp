// FEFolderMonitor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include <string>
#include "CRUD.h"

void WatchDir(const std::wstring& directoryPath) {
	HANDLE hDir = CreateFileW(
		directoryPath.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_DELETE | FILE_SHARE_WRITE | FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL
	);

	if (hDir == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to open monitoring directory." << std::endl;
		return;
	}

	char buffer[1024];
	DWORD bytesReturned;
	FILE_NOTIFY_INFORMATION* notifyInfo;

	while (true) {
		// CRUD watch
		if (ReadDirectoryChangesW(
			hDir,
			&buffer, sizeof(buffer),
			TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
			&bytesReturned,
			NULL,
			NULL
		)) {
			// Buffer --> Notification
			notifyInfo = (FILE_NOTIFY_INFORMATION*)buffer;

			do {
				std::wstring fileName(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));
				if (notifyInfo->Action == FILE_ACTION_ADDED) {
					std::wcout << L"New File: " << fileName << std::endl;

					std::wstring filePath = directoryPath + L"\\" + fileName;

					if (uploadFile(filePath))
					{
						std::wcout << L"File uploaded successfully!" << std::endl;
						// If upload successful, delete the file
						if (deleteFile(filePath)) {
							std::wcout << L"File deleted after upload." << std::endl;
						}
						else {
							std::wcerr << L"Failed to delete file: " << filePath << std::endl;
						}
					}
					else
					{
						std::wcerr << L"File upload failed: " << filePath << std::endl;
					}
				}

				notifyInfo = notifyInfo->NextEntryOffset ? (FILE_NOTIFY_INFORMATION*)((char*)notifyInfo + notifyInfo->NextEntryOffset) : NULL;
			} while (notifyInfo != NULL);
		}
		else
		{
			std::cerr << "Failed to read directory changes" << std::endl;
			break;
		}
	}

	CloseHandle(hDir);
}

int main()
{
	std::wstring MonitoringFolder = L"D:\\businesses\\privhost\\Hello";
	WatchDir(MonitoringFolder);
	return 0;
}