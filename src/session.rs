use axum::{
    http::HeaderMap,
    response::{Html, IntoResponse, Response},
};
use chrono::{Duration, Utc};
use jsonwebtoken::{
    Algorithm, DecodingKey, EncodingKey, Header, Validation, decode, encode, errors::Error,
};
use reqwest::header;
use serde::{Deserialize, Serialize};

const SECRET_KEY: &[u8] = b"kmra8Zorx5pe9BBHXyjxhw";

#[derive(Serialize, Deserialize)]
pub struct Claims {
    sub: String, // Subject (User ID)
    exp: usize,  // Expiration timestamp
}

pub fn create(id: &str, expiary: Duration) -> String {
    let expiration = Utc::now() + expiary;

    let claims = Claims {
        sub: id.to_owned(),
        exp: expiration.timestamp() as usize,
    };

    encode(
        &Header::default(),
        &claims,
        &EncodingKey::from_secret(SECRET_KEY),
    )
    .expect("JWT encoding failed")
}

pub fn verify(token: &str) -> Result<Claims, Error> {
    let token_data = decode::<Claims>(
        token,
        &DecodingKey::from_secret(SECRET_KEY),
        &Validation::new(Algorithm::HS256),
    )?;

    Ok(token_data.claims)
}
