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

/**
 * @brief Represents a message exchanged in the chat.
 */
class chat_message;

/**
 * @brief Abstract class representing a participant in the chat.
 */
class chat_participant {
public:
  virtual ~chat_participant() {}

  /**
   * @brief Delivers a chat message to the participant.
   * @param msg The chat message to be delivered.
   */
  virtual void deliver(const chat_message& msg) = 0;
};

/**
 * @brief Represents a message exchanged in the chat.
 */
class chat_message {
public:
  /**
   * @brief Constructs a chat message.
   * @param src The content of the message.
   * @param origin The originator of the message.
   */
  chat_message(std::string src, const chat_participant& origin)
  : what_(std::move(src)), origin_(origin){};

  /**
   * @brief Retrieves the data of the message.
   * @return A pointer to the data of the message.
   */
  auto data() { return what_.data(); };

  /**
   * @brief Retrieves the size of the message.
   * @return The size of the message.
   */
  size_t size() { return what_.size(); };

  /**
   * @brief Retrieves the originator of the message.
   * @return A reference to the originator of the message.
   */
  const chat_participant& origin() const { return origin_; }

private:
  std::string what_;
  const chat_participant& origin_;
};
using chat_message_queue = std::deque<chat_message>;

//----------------------------------------------------------------------

/**
 * @brief Represents a chat room where participants join and exchange messages.
 */
class chat_room {
public:
  /**
   * @brief Adds a participant to the chat room.
   * @param participant Pointer to the participant to be added.
   */
  void join(chat_participant* participant) {
    participants_.insert(participant);
    for (const auto& msg : recent_msgs_) participant->deliver(msg);
  }

  /**
   * @brief Removes a participant from the chat room.
   * @param participant Pointer to the participant to be removed.
   */
  void leave(chat_participant* participant) {
    participants_.erase(participant);
  }

  /**
   * @brief Delivers a message to all participants in the chat room.
   * @param msg The message to be delivered.
   */
  void deliver(const chat_message& msg);

private:
  std::set<chat_participant*> participants_;
  enum { max_recent_msgs = 100 };
  chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

/**
 * @brief Represents a session in the chat for a single participant.
 */
class chat_session : public chat_participant,
public std::enable_shared_from_this<chat_session> {
public:
  /**
   * @brief Constructs a chat session.
   * @param socket The socket associated with the session.
   * @param room The chat room the session belongs to.
   */
  chat_session(tcp::socket socket, chat_room& room)
  : socket_(std::move(socket)), room_(room) {}

  /**
   * @brief Starts the chat session.
   */
  void start() {
    room_.join(this);
    read_msg();
  }

  /**
   * @brief Delivers a message to the session.
   * @param msg The message to be delivered.
   */
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

/**
 * @brief Represents a chat server that manages multiple chat sessions.
 */
class chat_server {
public:
  /**
   * @brief Constructs a chat server.
   */
  chat_server() : acceptor_(ctx_) {}
  chat_server(const chat_server&) = delete;
  chat_server(chat_server&&) = delete;
  chat_server& operator=(const chat_server&) = delete;
  chat_server& operator=(chat_server&&) = delete;
  ~chat_server() { stop();}

  /**
   * @brief Starts the chat server on the specified port.
   * @param port The port number to start the server on.
   */
  void start(uint16_t port);

  /**
   * @brief Stops the chat server.
   */
  void stop();

  [[nodiscard]] tcp::endpoint local_endpoint() const {return acceptor_.local_endpoint();}

private:
  void accept();

  int thread_count_ = 2;
  std::vector<std::thread> thread_pool_;
  asio::io_context ctx_;
  tcp::acceptor acceptor_;
  chat_room room_;
};

}  // namespace greeter
