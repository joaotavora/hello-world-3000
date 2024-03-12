#include <greeter/greeter.h>
#include <greeter/version.h>

#include <cxxopts.hpp>
#include <iostream>
#include <string>

#define BOOST_ASIO_NO_DEPRECATED
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>
#include <string>
#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>



// Function to handle the HTTP request and send back a response
void handle_request([[maybe_unused]] http::request<http::string_body>&& req, http::response<http::string_body>& res) {
    nlohmann::json data;
    data["title"] = "Hello, World!";
    data["message"] = "This is a message from Boost.Beast and Inja.";

    inja::Environment env;
    std::string rendered_html = env.render_file("template.html", data);

    res.result(http::status::ok);
    res.set(http::field::content_type, "text/html");
    res.body() = rendered_html;
    res.prepare_payload();
}

// Asynchronous server session
class session : public std::enable_shared_from_this<session> {
    tcp::socket socket_;
    beast::flat_buffer buffer_;
    http::request<http::string_body> request_;
    http::response<http::string_body> response_;

public:
    explicit session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() { do_read(); }

private:
    void do_read() {
        auto self = shared_from_this();
        http::async_read(
            socket_, buffer_, request_,
            [this, self](beast::error_code ec,
                         [[maybe_unused]] std::size_t bytes_transferred) {
                             if (!ec) {
                                 handle_request(std::move(request_), response_);
                                 do_write();
                             }
                         });
    }

    void do_write() {
        auto self = shared_from_this();
        http::async_write(
            socket_, response_,
            [this, self](beast::error_code ec,
                         [[maybe_unused]] std::size_t bytes_transferred) {
                            socket_.shutdown(tcp::socket::shutdown_send, ec);
                          });
    }
};

// Listening server
class server {
    tcp::acceptor acceptor_;
    tcp::socket socket_;

public:
  server(net::io_context& ioc, short port)
      : acceptor_(ioc, {tcp::v4(), static_cast<net::ip::port_type>(port)}),
      socket_(ioc) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept(socket_,
                               [this](beast::error_code ec) {
                                   if (!ec) std::make_shared<session>(std::move(socket_))->start();
                                   do_accept();
                               });
    }
};


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
      auto const port = static_cast<unsigned short>(std::atoi("8080"));
      net::io_context ioc{1};

      server srv(ioc, port);
      ioc.run();
    }

    greeter::Greeter greeter(name);
    std::cout << greeter.greet(langIt->second) << std::endl;

    return 0;
  } catch (std::exception& e) {
    std::cerr << "Whoops: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
