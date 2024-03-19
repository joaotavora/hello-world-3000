#pragma once

#include <string>
#include <deque>
#include <set>

#include <greeter/greeter.h>
#include <greeter/version.h>

#include <boost/asio.hpp>

namespace greeter {

/**  Language codes to be used with the Greeter class */
enum class LanguageCode { EN, DE, ES, FR };

/**
 * @brief A class for saying hello in multiple languages
 */
class Greeter {
  std::string name;

public:
  /**
   * @brief Creates a new greeter
   * @param name the name to greet
   */
  Greeter(std::string name);

  /**
   * @brief Creates a localized string containing the greeting
   * @param lang the language to greet in
   * @return a string containing the greeting
   */
  std::string greet(LanguageCode lang = LanguageCode::EN) const;
};


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

  void deliver(const chat_message& msg);

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

  void deliver(const chat_message& msg);

private:
  void read_msg();

  void do_write();

  tcp::socket socket_;
  chat_room& room_;
  asio::streambuf streambuf_{512};
  chat_message_queue write_msgs_;
};

//----------------------------------------------------------------------

class chat_server {
public:
  chat_server() : acceptor_(ctx_) {}

  void start(uint16_t port);

  void stop();

private:
  void accept();

  int thread_count_ = 2;
  std::vector<std::thread> thread_pool_;
  asio::io_context ctx_;
  tcp::acceptor acceptor_;
  chat_room room_;
};  

}  // namespace greeter
