using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Net.Http;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using Guna.UI2.WinForms;

namespace FEFileExplorer
{
    internal class ServerCommunicator
    {
        public async Task DownloadFileWithProgress(string filename, Guna2ProgressBar progressBar)
        {
            Decryptor decryptor = new Decryptor();
            string url = $"http://127.0.0.1:8080/download/{filename}";
            string downloadPath = Path.Combine("downloads", filename);

            using (HttpClient client = new HttpClient())
            {
                HttpResponseMessage response = await client.GetAsync(url, HttpCompletionOption.ResponseHeadersRead);

                if (response.IsSuccessStatusCode)
                {
                    var totalBytes = response.Content.Headers.ContentLength ?? -1L;
                    var downloadedBytes = 0L;

                    using (var contentStream = await response.Content.ReadAsStreamAsync())
                    using (var memoryStream = new MemoryStream())
                    {
                        var buffer = new byte[8192];
                        int bytesRead;
                        progressBar.Value = 0;

                        // Download the encrypted file content
                        while ((bytesRead = await contentStream.ReadAsync(buffer, 0, buffer.Length)) > 0)
                        {
                            await memoryStream.WriteAsync(buffer, 0, bytesRead);
                            downloadedBytes += bytesRead;

                            if (totalBytes != -1)
                            {
                                progressBar.Value = (int)(downloadedBytes * 100 / totalBytes);
                            }
                        }

                        // Get the full encrypted content (nonce + ciphertext + tag)
                        byte[] encryptedContent = memoryStream.ToArray();

                        // Ensure the file has at least the nonce (12 bytes), ciphertext, and tag (16 bytes)
                        if (encryptedContent.Length <= 28)
                        {
                            throw new Exception("The encrypted file is invalid or corrupted.");
                        }

                        // Decrypt the encrypted content using the Decryptor class
                        byte[] decryptedContent = decryptor.Decrypt(encryptedContent);

                        // Write the decrypted file to disk
                        File.WriteAllBytes(downloadPath, decryptedContent);

                        Console.WriteLine("File downloaded and decrypted successfully!");
                    }
                }
                else
                {
                    Console.WriteLine("Failed to download the file.");
                }
            }
        }

        public async Task ListFiles(ListView fileListView)
        {
            using (HttpClient client = new HttpClient())
            {
                // Contact Rust server
                var response = await client.GetStringAsync("http://127.0.0.1:8080/list");
                var files = JsonConvert.DeserializeObject<List<FileInfo>>(response);

                fileListView.Items.Clear();

                foreach (var file in files)
                {
                    // UNIX timestamp -> DateTime
                    DateTime fileDate = DateTimeOffset.FromUnixTimeSeconds(file.Created).DateTime;

                    var item = new ListViewItem(new[]
                    {
                        file.Name,
                        fileDate.ToString("yyyy-MM-dd HH:mm:ss"),
                        file.file_type,
                        $"{file.Size / 1024} KB" // bytes -> KB
                    });

                    fileListView.Items.Add(item);
                }
            }
        }

        public class FileInfo
        {
            public string Name { get; set; }
            public long Size { get; set; }
            public string file_type { get; set; }
            public long Created { get; set; } // UNIX
        }

    }
}
