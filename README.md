
# PrivHost

**PrivHost** is a two-part system that enables secure file hosting and automatic folder monitoring. It consists of:

- **RustHTTPServer**: An HTTP server built in Rust to handle file uploads, encryption, downloads with decryption, and file listing.
- **FEFolderMonitor**: A C++ utility that monitors a folder for new files, uploads them to the server, and deletes them after successful upload.

## Features

### RustHTTPServer
- **/upload**: Uploads and encrypts files.
- **/download/{filename}**: Downloads and decrypts files.
- **/list**: Lists all available files on the server.

### FEFolderMonitor
- Monitors a folder for new files.
- Automatically uploads new files to RustHTTPServer using `libcurl`.
- Deletes files locally after a successful upload.

## Getting Started

### RustHTTPServer
1. Clone the repository and build the RustHTTPServer:
   ```bash
   cargo build --release
   ```
2. Run the server:
   ```bash
   ./target/release/RustHTTPServer
   ```

### FEFolderMonitor
1. Compile **FEFolderMonitor** using a C++ compiler:
   ```bash
   g++ -o FEFolderMonitor FEFolderMonitor.cpp CRUD.cpp -lcurl
   ```
2. Set the folder to be monitored inside `FEFolderMonitor.cpp` and run it:
   ```bash
   ./FEFolderMonitor
   ```

## Example Code

### RustHTTPServer

The RustHTTPServer includes functionality for uploading, encrypting, and downloading files. The server routes include:

- **/upload**: Handles file uploads and encrypts them on the server.
- **/download/{filename}**: Allows downloading and decrypting files.
- **/list**: Returns a list of all available files.

Example snippet:
```rust
// Handle file upload and encryption
fn upload_file(file: File) {
    // Encryption logic
}

// Handle file download and decryption
fn download_file(filename: String) {
    // Decryption logic
}
```

### FEFolderMonitor

The **FEFolderMonitor** uses `ReadDirectoryChangesW` to watch a folder and automatically uploads files using `libcurl`. Once a file is uploaded, it is deleted from the local directory.

```cpp
void WatchDir(const std::wstring& directoryPath) {
    HANDLE hDir = CreateFileW(directoryPath.c_str(), ...);

    // Monitor for new files and upload
    while (true) {
        if (ReadDirectoryChangesW(hDir, ...)) {
            // Upload file and delete after success
            uploadFile(filePath);
            deleteFile(filePath);
        }
    }
}
```

## License

This project is licensed under the MIT License.
