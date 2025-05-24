use axum::http::Response;
use axum::{Router, response::Html, routing::get};
use axum::{
    http::{StatusCode, header},
    response::IntoResponse,
};
use serde::Serialize;
use std::{path::PathBuf, sync::Arc};
use tera::{Context, Tera};
use tokio::{fs, net::TcpListener, signal};
use tower_http::services::ServeDir;

#[derive(Serialize)]
struct Section {
    header: String,
    buttons: Vec<Button>,
}

#[derive(Serialize)]
struct Button {
    name: String,
    icon: String,
    url: String,
}

#[tokio::main]
async fn main() {
    // Load templates from disk at runtime
    let tera = match Tera::new("templates/**/*.html") {
        Ok(t) => t,
        Err(e) => {
            eprintln!("Template parsing error(s): {}", e);
            std::process::exit(1);
        }
    };

    let tera = Arc::new(tera);

    let app = Router::new()
        .route("/", get(|| async { load_html("login").await }))
        .route("/heritage", get(|| async { load_html("heritage").await }))
        .route("/place", get(|| async { load_html("place").await }))
        .route("/history", get(|| async { load_html("history").await }))
        .route("/pride", get(|| async { load_html("pride").await }))
        .route("/kushtia", get(|| async { load_html("pdf/kushtia.pdf") }))
        .route("/dashboard", get(move || render_dashboard()))
        .fallback_service(ServeDir::new("static"));

    let listener = TcpListener::bind("0.0.0.0:3000").await.unwrap();

    axum::serve(listener, app)
        .with_graceful_shutdown(shutdown_signal())
        .await
        .unwrap();
}

async fn render_dashboard() -> Html<String> {
    let tera = match Tera::new("templates/**/*.html") {
        Ok(t) => t,
        Err(e) => return Html(format!("Template error: {}", e)),
    };

    let circular_buttons = vec![
        Button {
            icon: "/icon/amarkushtia.jpg".to_string(),
            name: "আমাদের কুষ্টিয়া".to_string(),
            url: "/kushtia".to_string(),
        },
        Button {
            icon: "/icon/pride.jpg".to_string(),
            name: "Pride".to_string(),
            url: "/pride".to_string(),
        },
        Button {
            icon: "/icon/culture.jpg".to_string(),
            name: "ঐতিহ্য ও ভাষা".to_string(),
            url: "/heritage".to_string(),
        },
        Button {
            icon: "/icon/spot.jpeg".to_string(),
            name: "দর্শনীয়  স্থান ও স্থাপনা".to_string(),
            url: "/place".to_string(),
        },
        Button {
            icon: "/icon/history.jpeg".to_string(),
            name: "ইতিহাস".to_string(),
            url: "/history".to_string(),
        },
        Button {
            icon: "/icon/news.jpeg".to_string(),
            name: "খবর".to_string(),
            url: "https://dainikkushtia.net".to_string(),
        },
        Button {
            icon: "/icon/book.jpeg".to_string(),
            name: "বই".to_string(),
            url: "https://boiwave.blogspot.com".to_string(),
        },
    ];

    let sections = vec![
        Section {
            header: "সরকারি সেবা".into(),
            buttons: vec![
                Button {
                    icon: "/icon/problem.jpeg".into(),
                    name: "সমস্যা জানান".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/ec.png".into(),
                    name: "নির্বাচন কমিশন".into(),
                    url: "https://example.com/2".into(),
                },
                Button {
                    icon: "/icon/land.jpeg".into(),
                    name: "ভূমি মন্ত্রণালয়".into(),
                    url: "https://example.com/3".into(),
                },
                Button {
                    icon: "/icon/tax.jpg".into(),
                    name: "কর প্রদান".into(),
                    url: "https://example.com/3".into(),
                },
                Button {
                    icon: "/icon/pass.jpeg".into(),
                    name: "পাসপোর্ট অফিস".into(),
                    url: "https://example.com/3".into(),
                },
                Button {
                    icon: "/icon/elec.jpeg".into(),
                    name: "পল্লী বিদ্যুৎ".into(),
                    url: "https://example.com/3".into(),
                },
                Button {
                    icon: "/icon/post.jpeg".into(),
                    name: "পোস্ট অফিস".into(),
                    url: "https://example.com/3".into(),
                },
                Button {
                    icon: "/icon/cort.jpeg".into(),
                    name: "আদালত".into(),
                    url: "https://example.com/3".into(),
                },
                Button {
                    icon: "/icon/police.jpeg".into(),
                    name: "থানা".into(),
                    url: "https://example.com/3".into(),
                },
                Button {
                    icon: "/icon/land.jpeg".into(),
                    name: "জেলা প্রশাসন".into(),
                    url: "https://example.com/3".into(),
                },
                Button {
                    icon: "/icon/citi.jpeg".into(),
                    name: "নাগরিক অনুসন্ধান".into(),
                    url: "https://example.com/3".into(),
                },
            ],
        },
        Section {
            header: "জরুরী সেবা".into(),
            buttons: vec![
                Button {
                    icon: "/icon/hotline.jpeg".into(),
                    name: "জরুরী হট লাইন".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/blood.jpg".into(),
                    name: "ব্লাড ডোনার".into(),
                    url: "https://example.com/2".into(),
                },
                Button {
                    icon: "/icon/police.jpeg".into(),
                    name: "পুলিশ".into(),
                    url: "https://example.com/3".into(),
                },
                Button {
                    icon: "/icon/amb.jpeg".into(),
                    name: "অ্যাম্বুলেন্স".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/hosp.jpeg".into(),
                    name: "হাসপাতাল".into(),
                    url: "https://example.com/2".into(),
                },
                Button {
                    icon: "/icon/fire.jpeg".into(),
                    name: "ফায়ার সার্ভিস".into(),
                    url: "https://example.com/2".into(),
                },
            ],
        },
        Section {
            header: "স্বাস্থ্য সেবা".into(),
            buttons: vec![
                Button {
                    icon: "/icon/blood.jpg".into(),
                    name: "ব্লাড ডোনার".into(),
                    url: "https://example.com/2".into(),
                },
                Button {
                    icon: "/icon/amb.jpeg".into(),
                    name: "অ্যাম্বুলেন্স".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/hosp.jpeg".into(),
                    name: "হাসপাতাল".into(),
                    url: "https://example.com/2".into(),
                },
                Button {
                    icon: "/icon/doctor.jpeg".into(),
                    name: "ডাক্তার".into(),
                    url: "https://example.com/3".into(),
                },
            ],
        },
        Section {
            header: "আইন ও নিরাপত্তা".into(),
            buttons: vec![
                Button {
                    icon: "/icon/police.jpeg".into(),
                    name: "পুলিশ".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/cyber.jpeg".into(),
                    name: "সাইবার সিকিউরিটি".into(),
                    url: "https://example.com/2".into(),
                },
                Button {
                    icon: "/icon/journal.jpeg".into(),
                    name: "সাংবাদিক".into(),
                    url: "https://example.com/2".into(),
                },
                Button {
                    icon: "/icon/law.jpeg".into(),
                    name: "আইনজীবী".into(),
                    url: "https://example.com/1".into(),
                },
            ],
        },
        Section {
            header: "নাগরিক সেবা".into(),
            buttons: vec![
                Button {
                    icon: "/icon/govt1.svg".into(),
                    name: "নাগরিক অনুসন্ধান".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt2.svg".into(),
                    name: "অভিযোগ পোর্টাল".into(),
                    url: "https://example.com/2".into(),
                },
            ],
        },
        Section {
            header: "পরিবহন".into(),
            buttons: vec![
                Button {
                    icon: "/icon/bus.png".into(),
                    name: "বাস টিকেট".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/carrent.png".into(),
                    name: "কার রেন্ট".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/train.jpg".into(),
                    name: "ট্রেন টিকেট".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt1.svg".into(),
                    name: "বিমান টিকেট".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/courier.jpg".into(),
                    name: "কুরিয়ার সার্ভিস".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/travel.png".into(),
                    name: "ট্রাভেল এজেন্সি".into(),
                    url: "https://example.com/1".into(),
                },
            ],
        },
        Section {
            header: "বিনোদন".into(),
            buttons: vec![
                Button {
                    icon: "/icon/tour.jpg".into(),
                    name: "দর্শনীয় স্থান".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt1.svg".into(),
                    name: "পার্ক ও খেলার মাঠ".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt1.svg".into(),
                    name: "ধর্ম উপাসনালয়".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt1.svg".into(),
                    name: "সংগঠন".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt1.svg".into(),
                    name: "গুরুত্বপূর্ণ স্থান".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/theater.png".into(),
                    name: "মুভি থিয়েটার".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/gym.png".into(),
                    name: "Gym".into(),
                    url: "https://example.com/1".into(),
                },
            ],
        },
        Section {
            header: "হোটেল ও খাবার".into(),
            buttons: vec![
                Button {
                    icon: "/icon/govt1.svg".into(),
                    name: "শপিং মল".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt2.svg".into(),
                    name: "হোটেল".into(),
                    url: "https://example.com/2".into(),
                },
                Button {
                    icon: "/icon/resturant.png".into(),
                    name: "রেস্টুরেন্ট".into(),
                    url: "https://example.com/2".into(),
                },
                Button {
                    icon: "/icon/cafe.jpg".into(),
                    name: "ক্যাফে".into(),
                    url: "https://example.com/2".into(),
                },
            ],
        },
        Section {
            header: "শিক্ষা".into(),
            buttons: vec![
                Button {
                    icon: "/icon/school.png".into(),
                    name: "প্রাথমিক-মাধ্যমিক স্কুল".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/univercity.jpg".into(),
                    name: "কলেজ ইউনিভার্সিটি".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/vocation.png".into(),
                    name: "কারিগরি".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/madrasa.png".into(),
                    name: "মাদ্রাসা".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/trading.jpg".into(),
                    name: "ট্রেডিং ইনস্টিটিউট".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt1.svg".into(),
                    name: "কোচিং সেন্টার".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt1.svg".into(),
                    name: "আইটি সার্ভিস".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/ict.png".into(),
                    name: "আইটি ফার্ম".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt1.svg".into(),
                    name: "মোবাইল ইলেকট্রনিক্স".into(),
                    url: "https://example.com/1".into(),
                },
            ],
        },
        Section {
            header: "শ্রমিক ও সেবা".into(),
            buttons: vec![
                Button {
                    icon: "/icon/electrician.jpg".into(),
                    name: "Electritian".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt2.svg".into(),
                    name: "AC Mechanic".into(),
                    url: "https://example.com/2".into(),
                },
                Button {
                    icon: "/icon/plumber.png".into(),
                    name: "Plumber".into(),
                    url: "https://example.com/3".into(),
                },
                Button {
                    icon: "/icon/worker.png".into(),
                    name: "Labour".into(),
                    url: "https://example.com/3".into(),
                },
            ],
        },
        Section {
            header: "নিকটবর্তী".into(),
            buttons: vec![
                Button {
                    icon: "/icon/govt1.svg".into(),
                    name: "এটিএম ফিলিং স্টেশন".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt2.svg".into(),
                    name: "হাসপাতাল থানা".into(),
                    url: "https://example.com/2".into(),
                },
                Button {
                    icon: "/icon/govt3.svg".into(),
                    name: "পার্কিং বাস স্ট্যান্ড".into(),
                    url: "https://example.com/3".into(),
                },
                Button {
                    icon: "/icon/bank.png".into(),
                    name: "ব্যাংক".into(),
                    url: "https://example.com/1".into(),
                },
                Button {
                    icon: "/icon/govt1.svg".into(),
                    name: "পার্ক ও খেলার মাঠ".into(),
                    url: "https://example.com/1".into(),
                },
            ],
        },
    ];

    // Create a Tera context and insert the data
    let mut context = Context::new();
    context.insert("sections", &sections);
    context.insert("circular_buttons", &circular_buttons);

    // Render the template (dashboard.html) with the context
    match tera.render("dashboard.html", &context) {
        Ok(rendered) => Html(rendered),
        Err(err) => Html(format!("Template error: {}", err)),
    }
}

async fn load_html(file: &str) -> Html<String> {
    let path: PathBuf = ["web", file].iter().collect();
    let path = path.with_extension("html");

    match fs::read_to_string(path).await {
        Ok(contents) => Html(contents),
        Err(_) => Html("File not found or could not be read.".into()),
    }
}

// async fn load_pdf(file: &str) -> Response {
//     let path: PathBuf = "./static/kushtia.pdf".into(); // Path to your PDF
//
//     match fs::read(path).await {
//         Ok(contents) => ([(header::CONTENT_TYPE, "application/pdf")], contents).into_response(),
//         Err(_) => (StatusCode::NOT_FOUND, "PDF file not found".to_string()).into_response(),
//     }
// }

async fn shutdown_signal() {
    let ctrl_c = async {
        signal::ctrl_c()
            .await
            .expect("failed to install Ctrl+C handler");
    };

    #[cfg(unix)]
    let terminate = async {
        signal::unix::signal(signal::unix::SignalKind::terminate())
            .expect("failed to install signal handler")
            .recv()
            .await;
    };

    #[cfg(not(unix))]
    let terminate = std::future::pending::<()>();

    tokio::select! {
        _ = ctrl_c => {},
        _ = terminate => {},
    }
}
