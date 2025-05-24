use axum::{
    Router,
    response::{Html, IntoResponse},
    routing::get,
};
use gmail::Gmail;
use tokio::{net::TcpListener, signal};
use tower_http::services::ServeDir;

mod gmail;

#[tokio::main]
async fn main() {
    let app = Router::new()
        .route("/", get(test))
        .fallback_service(ServeDir::new("static"));
    let listener = TcpListener::bind("0.0.0.0:3000").await.unwrap();

    //let mail = Gmail::new("data/token.json").await;
    //mail.send_text("Test", "This is a test email", "tasinfarhan1016@gmail.com")
    //    .await;

    axum::serve(listener, app)
        .with_graceful_shutdown(shutdown_signal())
        .await
        .unwrap();
}

async fn test() -> impl IntoResponse {
    Html(std::fs::read_to_string("web/hello.html").unwrap())
}

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
