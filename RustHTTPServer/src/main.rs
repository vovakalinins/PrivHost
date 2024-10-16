use actix_cors::Cors;
use actix_multipart::Multipart;
use actix_web::{web, App, Error, HttpResponse, HttpServer};
use aes_gcm::aead::{Aead, KeyInit}; // Import the KeyInit trait for .new() method
use aes_gcm::{Aes256Gcm, Key, Nonce}; // AES-GCM for encryption
use futures_util::StreamExt as _;
use rand::Rng;
use serde::{Deserialize, Serialize};
use std::fs::Metadata;
use std::fs::{self, File};
use std::io::{Read, Write};
use std::path::Path;
use std::time::{SystemTime, UNIX_EPOCH};
use uuid::Uuid;

const ENCRYPTION_KEY: &[u8; 32] = b"ifyouseethisyouhavetosubscribeee"; // 32 bytes

async fn save_file(mut payload: Multipart) -> Result<HttpResponse, Error> {
    // Make cipher
    let key = Key::<Aes256Gcm>::from_slice(ENCRYPTION_KEY); // Fix the generic type and pass a reference

    let cipher = Aes256Gcm::new(&key);

    // Iterate multipart stream
    while let Some(field) = payload.next().await {
        let mut field = field?;

        // filename
        let content_disposition = field.content_disposition();
        let filename = if let Some(fname) = content_disposition.get_filename() {
            sanitize_filename::sanitize(fname)
        } else {
            format!("file_{}", Uuid::new_v4())
        };

        // gen a random nonce
        let nonce = rand::thread_rng().gen::<[u8; 12]>();

        // put file content into buffer
        let mut buffer = Vec::new();
        while let Some(chunk) = field.next().await {
            let data = chunk?;
            buffer.extend_from_slice(&data);
        }

        // Encryptize
        let ciphertext = cipher
            .encrypt(Nonce::from_slice(&nonce), buffer.as_ref())
            .map_err(|_| actix_web::error::ErrorInternalServerError("Encryption failed"))?;

        // create if not exists dir
        fs::create_dir_all("./uploads")?;

        // Save nonce and ciphertext
        let filepath = format!("./uploads/{}", filename);
        let mut file = File::create(&filepath)?;
        file.write_all(&nonce)?;
        file.write_all(&ciphertext)?;

        println!("Saved encrypted file: {}", filepath);
    }

    Ok(HttpResponse::Ok().json("File uploaded and encrypted successfully"))
}

async fn download_file(path: web::Path<String>) -> Result<HttpResponse, Error> {
    let filename = path.into_inner();

    let filepath = format!("./uploads/{}", sanitize_filename::sanitize(&filename));

    // does file exist check
    if !std::path::Path::new(&filepath).exists() {
        return Ok(HttpResponse::NotFound().body("File not found"));
    }

    // Read the file
    let mut file = File::open(&filepath)?;
    let mut contents = Vec::new();
    file.read_to_end(&mut contents)?;

    // Split nonce and ciphertext
    if contents.len() < 12 {
        return Ok(HttpResponse::InternalServerError().body("Invalid file format"));
    }
    let (nonce_bytes, ciphertext) = contents.split_at(12); // 12 bytes for nonce

    // safely create nonce
    let nonce = Nonce::from_slice(nonce_bytes);

    // init the cipher
    let key = Key::<Aes256Gcm>::from_slice(ENCRYPTION_KEY);
    let cipher = Aes256Gcm::new(key);

    // Decrypt
    let plaintext = cipher
        .decrypt(nonce, ciphertext)
        .map_err(|_| actix_web::error::ErrorInternalServerError("Decryption failed"))?;

    Ok(HttpResponse::Ok()
        .content_type("application/octet-stream")
        .body(plaintext))
}

#[derive(Serialize)]
struct FileInfo {
    name: String,
    size: u64,
    file_type: String,
    created: u64,
}

async fn list_files() -> Result<HttpResponse, Error> {
    let dir_path = "./uploads";

    // does uploads exists
    if !Path::new(dir_path).exists() {
        return Ok(HttpResponse::NotFound().body("Uploads directory not found"));
    }

    let mut files = Vec::new();
    for entry in fs::read_dir(dir_path)? {
        let entry = entry?;
        let path = entry.path();

        if path.is_file() {
            if let Some(file_name) = path.file_name() {
                if let Some(name_str) = file_name.to_str() {
                    let metadata = fs::metadata(&path)?;

                    // filesize
                    let size = metadata.len();

                    // filetype
                    let file_type = match path.extension() {
                        Some(ext) => ext.to_string_lossy().into_owned(),
                        None => String::from("unknown"),
                    };

                    // filedate
                    let created = metadata
                        .created()
                        .or_else(|_| metadata.modified())
                        .unwrap_or(SystemTime::UNIX_EPOCH)
                        .duration_since(UNIX_EPOCH)
                        .unwrap()
                        .as_secs();

                    files.push(FileInfo {
                        name: name_str.to_string(),
                        size,
                        file_type,
                        created,
                    });
                }
            }
        }
    }

    Ok(HttpResponse::Ok().json(files))
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    println!("File upload server is running at http://127.0.0.1:8080");

    HttpServer::new(|| {
        App::new()
            .wrap(
                Cors::default()
                    .allow_any_origin()
                    .allow_any_method()
                    .allow_any_header()
                    .max_age(3600),
            )
            .route("/upload", web::post().to(save_file))
            .route("/download/{filename}", web::get().to(download_file))
            .route("/list", web::get().to(list_files))
    })
    .bind("127.0.0.1:8080")?
    .run()
    .await
}
