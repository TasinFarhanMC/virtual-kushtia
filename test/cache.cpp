#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>

using String = std::string;
using Map = std::unordered_map<String, String>;
namespace fs = std::filesystem;

void cache(Map &map, const char *path) {
  try {

    auto it = fs::recursive_directory_iterator(path);
    const size_t count = std::distance(
        fs::recursive_directory_iterator(path), fs::recursive_directory_iterator()
    );
    map.reserve(count);

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

// Function to create test files for benchmarking
void create_test_files(const std::string &dir, int num_files, int size_per_file) {
  fs::create_directories(dir);
  for (int i = 0; i < num_files; ++i) {
    std::ofstream file(dir + "/file" + std::to_string(i) + ".txt", std::ios::binary);
    file << std::string(size_per_file, 'X'); // Fill with dummy data
  }
}

// Function to clean up test files
void clean_test_files(const std::string &dir) {
  if (fs::exists(dir)) {
    fs::remove_all(dir); // Remove test directory and all its contents
  }
}

TEST_CASE("Benchmark cache function", "[cache]") {
  Map fileCache;
  std::string test_dir = "test_data";

  create_test_files(test_dir, 10, 1024);
  BENCHMARK("Cache 10 small files") {
    fileCache.clear();
    cache(fileCache, test_dir.c_str());
  };

  create_test_files(test_dir, 100, 10240);
  BENCHMARK("Cache 100 medium files") {
    fileCache.clear();
    cache(fileCache, test_dir.c_str());
  };

  create_test_files(test_dir, 500, 102400);
  BENCHMARK("Cache 500 large files") {
    fileCache.clear();
    cache(fileCache, test_dir.c_str());
  };

  fs::create_directories(test_dir + "/level1/level2/level3/level4");
  for (int i = 1; i <= 5; ++i) {
    create_test_files(test_dir + "/level" + std::to_string(i), 10, 4096);
  }
  BENCHMARK("Cache deep directory") {
    fileCache.clear();
    cache(fileCache, test_dir.c_str());
  };

  // Cleanup test files after each test case
  clean_test_files(test_dir);
}
