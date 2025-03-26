#include <chrono>
#include <iostream>
#include <ostream>
#include <print>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

using String = std::string;
using Map = std::unordered_map<String, String>;

// Original implementation (as a reference for benchmarking)
Map parse_url_encode(const String &body);

// Optimized implementation
Map parse_url(const String &body);

// Generate random URL-encoded strings for benchmarking
String generate_random_query(size_t length) {
  const std::string chars =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789%=&";
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<size_t> dist(0, chars.size() - 1);

  String result;
  result.reserve(length);
  for (size_t i = 0; i < length; ++i) { result += chars[dist(gen)]; }
  return result;
}

// Benchmark function
void benchmark_parsing(size_t iterations, size_t query_length) {
  std::vector<String> test_cases;
  for (size_t i = 0; i < iterations; ++i) {
    test_cases.push_back(generate_random_query(query_length));
  }

  auto measure = [](auto func, const std::vector<String> &cases, const std::string &label) {
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto &query : cases) { func(query, 10); }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << label << " took " << duration.count() << " ms\n";
  };

  std::cout << "Benchmarking with " << iterations << " iterations, query length "
            << query_length << "\n";
  measure(parse_url_encode, test_cases, "Optimized Implementation");
  measure(parse_url, test_cases, "Original Implementation");
}

int main() {
  size_t iterations = 100000;
  size_t query_length = 100;

  for (const auto &item : parse_url_encode("Test=Hello&World%20Mine=Is")) {
    std::println("key: {}", item.first);
    std::println("value {}", item.second);
  }

  benchmark_parsing(iterations, query_length);
  return 0;
}
