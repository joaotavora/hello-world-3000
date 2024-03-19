#include <greeter/greeter.h>
#include <greeter/version.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <boost/asio.hpp>
#include <cstddef>
#include <cxxopts.hpp>
#include <limits>
#include <memory>
#include <set>
#include <utility>

#include "boost/asio/buffer.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/streambuf.hpp"

namespace asio = boost::asio;
using asio::ip::tcp;

class chat_message;

class chat_participant {
public:
  virtual ~chat_participant() {}
  virtual void deliver(const chat_message& msg) = 0;
};

class chat_message {
public:
  chat_message(std::string src, const chat_participant& origin)
      : what_(std::move(src)), origin_(origin){};

  auto data() { return what_.data(); };
  size_t size() { return what_.size(); };

  const chat_participant& origin() const { return origin_; }

private:
  std::string what_;
  const chat_participant& origin_;
};
using chat_message_queue = std::deque<chat_message>;

typedef std::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room {
public:
  void join(chat_participant* participant) {
    participants_.insert(participant);
    for (const auto& msg : recent_msgs_) participant->deliver(msg);
  }

  void leave(chat_participant* participant) {
    participants_.erase(participant);
  }

  void deliver(const chat_message& msg) {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs) recent_msgs_.pop_front();

    for (auto participant : participants_) {
      if (&msg.origin() != participant) participant->deliver(msg);
    }
  }

private:
  std::set<chat_participant*> participants_;
  enum { max_recent_msgs = 100 };
  chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class chat_session : public chat_participant,
                     public std::enable_shared_from_this<chat_session> {
public:
  chat_session(tcp::socket socket, chat_room& room)
      : socket_(std::move(socket)), room_(room) {}

  void start() {
    room_.join(this);
    read_msg();
  }

  void deliver(const chat_message& msg) {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
      do_write();
    }
  }

private:
  void read_msg() {
    asio::async_read_until(
        socket_, streambuf_, "\r\n",
        [this, self = shared_from_this()](boost::system::error_code ec,
                                          std::size_t) {
          spdlog::info("Yeah got something...!");

          if (ec) {
            spdlog::warn("Ooops, error '{}', so leaving!", ec.to_string());
            self->room_.leave(this);
            return;
          }
          std::string str(boost::asio::buffers_begin(streambuf_.data()),
                          boost::asio::buffers_end(streambuf_.data()));
          streambuf_.consume(std::numeric_limits<int>::max());
          spdlog::info("Gonna deliver '{}'", str);
          self->room_.deliver(chat_message(str, *self));
          self->read_msg();
        });
  }

  void do_write() {
    asio::async_write(
        socket_,
        boost::asio::buffer(write_msgs_.front().data(),
                            write_msgs_.front().size()),
        [this, self = shared_from_this()](boost::system::error_code ec,
                                          std::size_t /*length*/) {
          if (!ec) {
            write_msgs_.pop_front();
            if (!write_msgs_.empty()) {
              do_write();
            }
          } else {
            room_.leave(this);
          }
        });
  }

  tcp::socket socket_;
  chat_room& room_;
  asio::streambuf streambuf_{512};
  chat_message_queue write_msgs_;
};

//----------------------------------------------------------------------

class chat_server {
public:
  chat_server() : acceptor_(ctx_) {}

  void start(uint16_t port) {
    tcp::endpoint ep{tcp::v4(), port};
    acceptor_.open(ep.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(ep);
    acceptor_.listen();
    spdlog::info("Should be listening on {}", port);

    accept();
    for (int i = 0; i < thread_count_; ++i) {
      thread_pool_.emplace_back([=] { ctx_.run(); });
    }
  }

  void stop() {
    ctx_.stop();
    for (auto& thread : thread_pool_)
      if (thread.joinable()) thread.join();
    thread_pool_.clear();
  }

private:
  void accept() {
    spdlog::info("In accept()");
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
          auto ep = socket.remote_endpoint();
          spdlog::info("Handling new connection from {}:{}",
                       ep.address().to_string(), ep.port());
          if (!ec) {
            spdlog::info("Starting a chat session");
            std::make_shared<chat_session>(std::move(socket), room_)->start();
          }

          accept();
        });
  }

  int thread_count_ = 2;
  std::vector<std::thread> thread_pool_;
  asio::io_context ctx_;
  tcp::acceptor acceptor_;
  chat_room room_;
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
      chat_server server{};
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
