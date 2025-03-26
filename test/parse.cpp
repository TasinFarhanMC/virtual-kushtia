#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <unordered_map>

template <typename T, size_t size> using Array = std::array<T, size>;
using StringView = std::string_view;
using String = std::string;
using Map = std::unordered_map<String, String>;

Map decode_url(const String &body, const size_t keys) {
  constexpr auto percent_decode = [](const StringView &input, char *buffer) {
    constexpr auto table = []() consteval {
      Array<unsigned char, 256> table {};
      table.fill(255);
      for (int i = '0'; i <= '9'; ++i) table[i] = i - '0';
      for (int i = 'A'; i <= 'F'; ++i) table[i] = i - 'A' + 10;
      for (int i = 'a'; i <= 'f'; ++i) table[i] = i - 'a' + 10;
      return table;
    }();

    size_t str = 0;

    for (size_t i = 0; i < input.size(); ++i) {
      char c = input[i];
      if (c == '%' && i + 2 < input.size()) {
        unsigned char v1 = table[input[i + 1]];
        unsigned char v2 = table[input[i + 2]];

        if (v1 == 255 || v2 == 255) return;
        buffer[str++] = (v1 << 4) | v2;
        i += 2;
      } else {
        buffer[str++] = c;
      }
    }

    buffer[str] = 0;
  };

  char key_buf[1024], val_buf[1024];

  Map form_data;
  size_t start = 0, end;

  while (start != String::npos) {
    size_t end = body.find('&', start);
    size_t pos = body.find('=', start);

    if (pos != String::npos && (end == String::npos || pos < end)) {
      percent_decode(StringView(body).substr(start, pos - start), key_buf);
      percent_decode(
          StringView(body).substr(
              pos + 1, (end == String::npos ? body.size() : end) - pos - 1
          ),
          val_buf
      );

      form_data[key_buf] = val_buf;
    }

    start = (end == String::npos) ? String::npos : end + 1;
  }

  return form_data;
}

Map parse_url_encode(const String &body, const size_t keys) {
  constexpr auto percent_decode = [](const StringView &input) -> String {
    constexpr auto table = []() consteval {
      Array<unsigned char, 256> table;
      table.fill(255);
      for (int i = '0'; i <= '9'; ++i) table[i] = i - '0';
      for (int i = 'A'; i <= 'F'; ++i) table[i] = i - 'A' + 10;
      for (int i = 'a'; i <= 'f'; ++i) table[i] = i - 'a' + 10;
      return table;
    }();

    String output;
    output.reserve(input.size());

    char *data = output.data();
    size_t str = 0;

    for (size_t i = 0; i < input.size(); ++i) {
      char c = input[i];
      if (c == '%') {
        if (i + 2 >= input.size()) { return ""; }

        unsigned char v1 = table[input[i + 1]];
        unsigned char v2 = table[input[i + 2]];

        if (v1 == 255 || v2 == 255) { return ""; }

        data[str++] = (v1 << 4) | v2;
        i += 2;
      } else {
        data[str++] = c;
      }
    }

    output.resize(str);

    return output;
  };

  Map form_data(keys);
  size_t start = 0, end;

  while (start != String::npos) {
    size_t end = body.find('&', start);
    size_t pos = body.find('=', start);

    if (pos != String::npos && (end == String::npos || pos < end)) {
      form_data.emplace(
          std::move(percent_decode(StringView(body).substr(start, pos - start))),
          std::move(percent_decode(StringView(body).substr(
              pos + 1, (end == String::npos ? body.size() : end) - pos - 1
          )))
      );
    }

    start = (end == String::npos) ? String::npos : end + 1;
  }

  return form_data;
}

Map new_decode_url(const String &body, const size_t keys) {
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

TEST_CASE("Benchmark decode_url") {
  const std::string body = "key1=value1&key2=value2&key3=value3&key4=value4";
  const size_t keys = 4;

  // Verify output matches
  REQUIRE(decode_url(body, keys) == new_decode_url(body, keys));

  BENCHMARK("Small") { return decode_url(body, keys); };
  BENCHMARK("New Small") { return new_decode_url(body, keys); };

  const std::string large_body =
      std::string(500, 'a') + "=value&" + std::string(500, 'b') + "=value";

  REQUIRE(decode_url(large_body, 2) == new_decode_url(large_body, 2));

  BENCHMARK("Large") { return decode_url(large_body, 2); };
  BENCHMARK("New Large") { return new_decode_url(large_body, 2); };

  std::string long_body;
  for (int i = 0; i < 500; ++i) {
    long_body += "key" + std::to_string(i) + "=value" + std::to_string(i) + "&";
  }
  long_body.pop_back();

  REQUIRE(decode_url(long_body, 500) == new_decode_url(long_body, 500));

  BENCHMARK("Long") { return decode_url(long_body, 500); };
  BENCHMARK("New Long") { return new_decode_url(long_body, 500); };
}
