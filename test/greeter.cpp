#include <doctest/doctest.h>
#include <greeter/greeter.h>
#include <greeter/version.h>

#include <string>
#include <thread>

#include <boost/process.hpp>

TEST_CASE("Greeter") {
  using namespace greeter;

  Greeter greeter("Tests");

  CHECK(greeter.greet(LanguageCode::EN) == "Hello, Tests!");
  CHECK(greeter.greet(LanguageCode::DE) == "Hallo Tests!");
  CHECK(greeter.greet(LanguageCode::ES) == "¡Hola Tests!");
  CHECK(greeter.greet(LanguageCode::FR) == "Bonjour Tests!");
}

TEST_CASE("Greeter version") {
  static_assert(std::string_view(GREETER_VERSION) == std::string_view("1.0.0"));
  CHECK(std::string(GREETER_VERSION) == std::string("1.0.0"));
}


TEST_CASE("Start up the chat server") {
  greeter::chat_server srv{};
  srv.start(0);  // This lets the OS choose an available port
  [[maybe_unused]] auto port = srv.local_endpoint().port();

  using namespace std::chrono_literals;
  std::this_thread::sleep_for(2s);
  srv.stop();
}

namespace bp = boost::process;

TEST_CASE("Test nc subprocess communication") {
  // Assume the server is already running on localhost:8888
  greeter::chat_server srv{};
  srv.start(0);  // This lets the OS choose an available port
  [[maybe_unused]] auto port = std::to_string(srv.local_endpoint().port());

  // Start two clients connecting to localhost:8888
  bp::opstream client1Input;
  bp::child client1(bp::search_path("nc"), "localhost", port, bp::std_in < client1Input);

  bp::ipstream client2Output;
  bp::child client2(bp::search_path("nc"), "localhost", port, bp::std_out > client2Output);

  // Send a dummy string from client1 to client2
  std::string dummyString = "Hello, world!";
  client1Input << dummyString << std::endl;

  // Read the output from client2
  std::string output;
  std::getline(client2Output, output);

  // // Clean up
  client1Input.pipe().close();
  client1.terminate();
  client2Output.pipe().close();
  client2.terminate();

  // Check if the output matches the dummy string
  CHECK(output == dummyString);
}
