#include <array>
#include <crow/app.h>

using namespace crow;

namespace fs = std::filesystem;

template <typename T, size_t size> using Array = std::array<T, size>;
using String = std::string;
using Map = std::unordered_map<String, String>;

void cache(Map &map, const char *path) {
  try {
    for (const auto &item : fs::recursive_directory_iterator(path)) {
      if (!item.is_regular_file()) continue;
      const size_t size = item.file_size();

      std::ifstream file(item.path(), std::ios::binary);
      if (!file) continue;

      String &buffer = map.try_emplace(item.path(), size, '\0').first->second;
      file.read(buffer.data(), size);
    }
  } catch (const std::exception &e) { std::cerr << "Error: " << e.what() << '\n'; }
}

Map decode_url(const String &body, const size_t keys) {
  constexpr auto table = []() consteval {
    Array<unsigned char, 256> table {};
    table.fill(255);
    for (int i = '0'; i <= '9'; ++i) table[i] = i - '0';
    for (int i = 'A'; i <= 'F'; ++i) table[i] = i - 'A' + 10;
    for (int i = 'a'; i <= 'f'; ++i) table[i] = i - 'a' + 10;
    return table;
  }();

  char key_buf[512], val_buf[512];

  Map form_data;
  form_data.reserve(keys);
  size_t start = 0;

  while (start != String::npos) {
    size_t end = body.find('&', start);
    size_t pos = body.find('=', start);

    if (pos != String::npos && (end == String::npos || pos < end)) {
      size_t key_size = 0, val_size = 0;

      // Corrected size calculation
      size_t key_end = pos;
      for (size_t i = start; i < key_end; ++i) {
        char c = body[i];
        if (c == '%' && i + 2 < key_end) {
          unsigned char v1 = table[body[i + 1]];
          unsigned char v2 = table[body[i + 2]];

          if (v1 == 255 || v2 == 255) continue;
          key_buf[key_size++] = (v1 << 4) | v2;
          i += 2;
        } else {
          key_buf[key_size++] = c;
        }
      }

      // Corrected size calculation
      size_t val_end = (end == String::npos) ? body.size() : end;
      for (size_t i = pos + 1; i < val_end; ++i) {
        char c = body[i];
        if (c == '%' && i + 2 < val_end) {
          unsigned char v1 = table[body[i + 1]];
          unsigned char v2 = table[body[i + 2]];

          if (v1 == 255 || v2 == 255) continue;
          val_buf[val_size++] = (v1 << 4) | v2;
          i += 2;
        } else {
          val_buf[val_size++] = c;
        }
      }

      form_data.emplace(
          std::piecewise_construct, std::forward_as_tuple(key_buf, key_size),
          std::forward_as_tuple(val_buf, val_size)
      );
    }

    start = (end == String::npos) ? String::npos : end + 1;
  }

  return form_data;
}

int main(int argc, char *argv[]) {
  const int port = argc > 1 ? std::atoi(argv[1]) : 8000;

  SimpleApp server;

  /*server.port(port).multithreaded().run();*/
  for (const auto &a : decode_url("Hello%20World=Mine&This=1", 2)) {
    CROW_LOG_INFO << "Key: " << a.first;
    CROW_LOG_INFO << "Value: " << a.second;
  }

  return 0;
}
