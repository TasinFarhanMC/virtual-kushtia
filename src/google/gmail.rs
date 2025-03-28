pub async fn send_text(&self, title: &str, body: &str, to: &str) {
    // Build the MIME message content
    let message = format!(
        "To: {}\r\nSubject: {}\r\nContent-Type: text/plain; charset=UTF-8\r\n\r\n{}",
        to, title, body
    );

    // Base64 encode the message
    let encoded_message = general_purpose::STANDARD.encode(&message);

    // Construct the API request
    let client = Client::new();
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
    let client = Client::new();
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
