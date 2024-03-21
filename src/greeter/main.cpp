#include <greeter/greeter.h>
#include <greeter/version.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <boost/asio.hpp>
#include <cxxopts.hpp>
#include <memory>

std::atomic<bool> server_shutdown = false;

auto main(int argc, char** argv) -> int {
  try {
    const std::unordered_map<std::string, greeter::LanguageCode> languages{
        {"en", greeter::LanguageCode::EN},
        {"de", greeter::LanguageCode::DE},
        {"es", greeter::LanguageCode::ES},
        {"fr", greeter::LanguageCode::FR},
    };

    cxxopts::Options options(*argv, "A program to welcome the world!");

    std::string language;
    std::string name;

    // clang-format off
    options.add_options()
      ("h,help", "Show help")
      ("v,version", "Print the current version number")
      ("n,name", "Name to greet", cxxopts::value(name)->default_value("World"))
      ("l,lang", "Language code to use", cxxopts::value(language)->default_value("en"))
      ("s,server", "Start a HTTP server");
    // clang-format on

    auto result = options.parse(argc, argv);

    if (result["help"].as<bool>()) {
      std::cout << options.help() << std::endl;
      return 0;
    }

    if (result["version"].as<bool>()) {
      std::cout << "Greeter, version " << GREETER_VERSION << std::endl;
      return 0;
    }

    auto langIt = languages.find(language);
    if (langIt == languages.end()) {
      std::cerr << "unknown language code: " << language << std::endl;
      return 1;
    }

    if (result["server"].as<bool>()) {
      greeter::chat_server server{};
      server.start(8888);

      auto signal_handler = [](int signal) {
        spdlog::info("Received {}, going to stop", signal);
        server_shutdown = true;
      };

      // Setup signal handling for graceful shutdown
      signal(SIGINT, signal_handler);
      signal(SIGTERM, signal_handler);

      while (!server_shutdown) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
      }
      spdlog::info("Stopping");
      server.stop();
      exit(0);
    }

    greeter::Greeter greeter(name);
    std::cout << greeter.greet(langIt->second) << std::endl;

    return 0;
  } catch (std::exception& e) {
    std::cerr << "Whoops: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
