use actix_cors::Cors;
use actix_multipart::Multipart;
use actix_web::{web, App, Error, HttpResponse, HttpServer};
use futures_util::StreamExt as _;
use serde::Serialize;
use std::fs::{self, File};
use std::io::{Read, Write};
use std::path::Path;
use std::time::{SystemTime, UNIX_EPOCH};
use uuid::Uuid;

async fn save_file(mut payload: Multipart) -> Result<HttpResponse, Error> {
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

        // put file content into buffer
        let mut buffer = Vec::new();
        while let Some(chunk) = field.next().await {
            let data = chunk?;
            buffer.extend_from_slice(&data);
        }

        // create if not exists dir
        fs::create_dir_all("./uploads")?;

        // Save file directly
        let filepath = format!("./uploads/{}", filename);
        let mut file = File::create(&filepath)?;
        file.write_all(&buffer)?;
    }

    Ok(HttpResponse::Ok().body("File uploaded successfully"))
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

    Ok(HttpResponse::Ok()
        .content_type("application/octet-stream")
        .body(contents))
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
