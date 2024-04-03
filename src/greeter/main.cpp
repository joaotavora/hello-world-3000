// #include <fmt/format.h>
#include <fmt/chrono.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <future>
#include <greeter/__generator.hpp>

namespace fs = std::filesystem;

std::vector<fs::directory_entry> regulars;

void list_dir(fs::directory_entry dentry) {
  std::vector<std::future<void>> futures;
  spdlog::info("Looking at {}", dentry.path().string());
  for (const auto& e : std::filesystem::directory_iterator(dentry)) {
    if (e.path().filename() == ".git") continue;
    if (e.is_directory()) {
      auto ftr = std::async(std::launch::async, list_dir, e);
      futures.push_back(std::move(ftr));
    } else {
      regulars.push_back(e);
    }
  }
  for (auto& f : futures) f.wait();
}

int main() {
  auto start = std::chrono::system_clock::now();

  list_dir(fs::directory_entry{"c:/repo/tsw"});

  auto took = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now() - start);

  spdlog::info("Vector complete, took {}", took);
  return 0;
}
