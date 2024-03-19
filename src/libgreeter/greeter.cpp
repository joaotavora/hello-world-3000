#include <fmt/format.h>
#include <greeter/greeter.h>

#include <boost/asio.hpp>
#include <spdlog/spdlog.h>

using namespace greeter;

Greeter::Greeter(std::string _name) : name(std::move(_name)) {}

std::string Greeter::greet(LanguageCode lang) const {
  switch (lang) {
    default:
    case LanguageCode::EN:
      return fmt::format("Hello, {}!", name);
    case LanguageCode::DE:
      return fmt::format("Hallo {}!", name);
    case LanguageCode::ES:
      return fmt::format("¡Hola {}!", name);
    case LanguageCode::FR:
      return fmt::format("Bonjour {}!", name);
  }
}

void greeter::chat_session::read_msg() {
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

void greeter::chat_session::do_write() {
  asio::async_write(socket_,
                    boost::asio::buffer(write_msgs_.front().data(),
                                        write_msgs_.front().size()),
                    [this, self = shared_from_this()](
                        boost::system::error_code ec, std::size_t /*length*/) {
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

void greeter::chat_server::accept() {
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

void greeter::chat_server::start(uint16_t port) {
  tcp::endpoint ep{tcp::v4(), port};
  acceptor_.open(ep.protocol());
  acceptor_.set_option(tcp::acceptor::reuse_address(true));
  acceptor_.bind(ep);
  acceptor_.listen();
  spdlog::info("Should be listening on {}", port);

  accept();
  for (int i = 0; i < thread_count_; ++i) {
    thread_pool_.emplace_back([this] { ctx_.run(); });
  }
}

void greeter::chat_server::stop() {
  ctx_.stop();
  for (auto& thread : thread_pool_)
    if (thread.joinable()) thread.join();
  thread_pool_.clear();
}

void greeter::chat_room::deliver(const chat_message& msg) {
  recent_msgs_.push_back(msg);
  while (recent_msgs_.size() > max_recent_msgs) recent_msgs_.pop_front();

  for (auto participant : participants_) {
    if (&msg.origin() != participant) participant->deliver(msg);
  }
}

void greeter::chat_session::deliver(const chat_message& msg) {
  bool write_in_progress = !write_msgs_.empty();
  write_msgs_.push_back(msg);
  if (!write_in_progress) {
    do_write();
  }
}
