#include <greeter/greeter.h>
#include <greeter/version.h>

#include <atomic>
#include <cxxopts.hpp>
#include <spdlog/spdlog.h>

#include <boost/asio.hpp>
#include <deque>
#include <istream>
#include <memory>
#include <thread>

namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;


// ConnectionHandler is the type that is going to be instantiated for
// each of these connections.
template <typename ConnectionHandler> class asio_generic_server {
  using shared_handler_t = std::shared_ptr<ConnectionHandler>;

public:
  asio_generic_server(int thread_count = 1)
  : thread_count_(thread_count), acceptor_(ctx_) {}

  void start_server([[maybe_unused]] uint16_t port) {
    tcp::endpoint endpoint{tcp::v4(), port};
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    spdlog::info("Should be listening on {}", port);

    for (int i = 0; i < thread_count_; ++i) {
      thread_pool_.emplace_back([=]{ctx_.run();});
    }
  }

  void stop_server() {
    ctx_.stop();
    for (auto& thread : thread_pool_)
      if (thread.joinable()) thread.join();
    thread_pool_.clear();
  }

private:
  void accept() {
    acceptor_.async_accept([=](auto ec, auto connsocket) {
      handle_new_connection(std::move(connsocket), ec);
    });
  }
  template <typename Socket>
  void handle_new_connection(Socket&& connsocket,
                             boost::system::error_code const& error) {
    auto ep = connsocket.remote_endpoint();
    spdlog::info("Handling new connection from {}:{}",
                 ep.address().to_string(),
                 ep.port());
    auto handler = std::make_shared<ConnectionHandler>(ctx_,
                                                       std::move(connsocket));

    if (error) {
      spdlog::info("Returning from handle_new_connection()");
      return;
    }

    handler->start();

    accept();
  }
  int thread_count_;
  std::vector<std::thread> thread_pool_;
  asio::io_context ctx_;
  tcp::acceptor acceptor_;
};

class chat_handler : public std::enable_shared_from_this<chat_handler> {
public:
  chat_handler(asio::io_context& ctx, tcp::socket&& socket)
  : ctx_(ctx), socket_{std::move(socket)}, write_strand_(ctx) {}
  tcp::socket& socket() { return socket_; }
  void start() { read_packet(); }

private:
  void read_packet(){
    spdlog::info("Will try to read a packet till EOL");
    asio::async_read_until(socket_, in_packet_, '\n',
                           [me = shared_from_this()](auto ec, size_t xfer) {
                             me->read_packet_done(ec, xfer);
                           });
  };

  void read_packet_done(boost::system::error_code ec, [[maybe_unused]]
                                                      size_t xfer) {
    if (ec) {
      spdlog::info("Returning from read_packet_done()");
      return;
    }

    std::istream stream(&in_packet_);
    std::string str;
    stream >> str;

    spdlog::info("Read a packet '{}'", str);

    read_packet();

  }

  [[maybe_unused]] asio::io_context& ctx_;
  tcp::socket socket_;
  asio::io_service::strand write_strand_;
  asio::streambuf in_packet_;
  std::deque<std::string> send_packet_queue_;
};

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
      asio_generic_server<chat_handler> server{};
      server.start_server(8888);

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
      server.stop_server();
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
