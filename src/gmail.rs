use base64::{Engine, engine::general_purpose};
use oauth2::{
    ClientId, ClientSecret, RefreshToken, TokenResponse, TokenUrl, basic::BasicClient, reqwest,
};
use reqwest::Client;
use serde::Deserialize;

#[derive(Debug)]
pub struct Gmail {
    token_file: String,
    access_token: String,
}

async fn exchange_token(token_file: &str) -> String {
    #[allow(dead_code)]
    #[derive(Deserialize)]
    struct Token {
        token_uri: String,
        client_id: String,
        client_secret: String,
        refresh_token: String,
    }

    let token: Token =
        serde_json::from_str(std::fs::read_to_string(token_file).unwrap().as_str()).unwrap();

    let client_id = ClientId::new(token.client_id);
    let client_secret = ClientSecret::new(token.client_secret);
    let refresh_token = RefreshToken::new(token.refresh_token);
    let token_uri = TokenUrl::new(token.token_uri).unwrap();

    let client = BasicClient::new(client_id)
        .set_token_uri(token_uri)
        .set_client_secret(client_secret);

    client
        .exchange_refresh_token(&refresh_token)
        .request_async(&Client::new())
        .await
        .unwrap()
        .access_token()
        .secret()
        .to_owned()
}

impl Gmail {
    pub async fn new(token_file: &str) -> Self {
        Self {
            access_token: exchange_token(token_file).await,
            token_file: token_file.to_owned(),
        }
    }

    pub async fn send_text(&self, title: &str, body: &str, to: &str) {
        // Build the MIME message content
        let message = format!(
            "To: {}\r\nSubject: {}\r\nContent-Type: text/plain; charset=UTF-8\r\n\r\n{}",
            to, title, body
        );

        // Base64 encode the message
        let encoded_message = general_purpose::STANDARD.encode(&message);

        // Construct the API request
        let client = reqwest::Client::new();
        let request_body = serde_json::json!({
            "raw": encoded_message
        });

        let response = client
            .post("https://gmail.googleapis.com/gmail/v1/users/me/messages/send")
            .header("Authorization", format!("Bearer {}", &self.access_token))
            .json(&request_body)
            .send()
            .await
            .unwrap();

        if !response.status().is_success() {
            println!(
                "Failed to send email: \"{}\",\n{}",
                title,
                response.text().await.unwrap()
            );
        }
    }

    pub async fn send_html(&self, title: &str, body: &str, to: &str) {
        // Build the MIME message content
        let message = format!(
            "To: {}\r\nSubject: {}\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n{}",
            to, title, body
        );

        // Base64 encode the message
        let encoded_message = general_purpose::STANDARD.encode(&message);

        // Construct the API request
        let client = reqwest::Client::new();
        let request_body = serde_json::json!({
            "raw": encoded_message
        });

        let response = client
            .post("https://gmail.googleapis.com/gmail/v1/users/me/messages/send")
            .header("Authorization", format!("Bearer {}", &self.access_token))
            .json(&request_body)
            .send()
            .await
            .unwrap();

        if !response.status().is_success() {
            println!(
                "Failed to send email: \"{}\",\n{}",
                title,
                response.text().await.unwrap()
            );
        }
    }
}
