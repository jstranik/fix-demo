#include <fmt/core.h>
#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
#include <set>
#include <system_error>
#include <unordered_set>
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

using json = nlohmann::json;
using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

using symbol_t = std::string;
using price_t = double;

class update_map {
  mutable std::mutex mutex_;
  std::unordered_map<symbol_t,price_t> m_current_prices;
  std::unordered_set<symbol_t> m_updated_sybmols;

  public:
  void push_price(symbol_t s, price_t p) {
    std::lock_guard l(mutex_);
    price_t& curr_price = m_current_prices[s];
    if (curr_price != p) {
      curr_price = p;
      m_updated_sybmols.insert(s);
    }
  }

  template<class F>
  void iterate_changes(const F& f, bool reset) {
    std::lock_guard l(mutex_);
    for(auto& s : m_updated_sybmols) {
      f(s, m_current_prices[s]);
    }
    if (reset) m_updated_sybmols.clear();
  }

  std::unordered_map<symbol_t,price_t> get_changes(bool reset) {
    std::unordered_map<symbol_t, price_t> result;
    iterate_changes([&](auto& s, auto& p){
      result[s] = p;
    }, reset);
    return result;
  }

  template<class F>
  void with_state(const F& f) const {
    std::lock_guard l(mutex_);
    f(m_current_prices);
  }
};

class WSEndpoint final : private boost::asio::noncopyable {
  public:
  using server_t = websocketpp::server<websocketpp::config::asio>;
  using handler_t = std::function<void(const json&)>;
  WSEndpoint(uint16_t port, boost::asio::io_context& ctx, const handler_t& handler) : WSEndpoint(port, ctx) {
    handler_ = handler;
  }
  WSEndpoint(uint16_t port, boost::asio::io_context& ctx)
    : port_(port)
  {
    server_.init_asio(&ctx);
    server_.set_reuse_addr(true);

    using websocketpp::connection_hdl;
    server_.set_open_handler([this](connection_hdl hdl){
      connections_.insert(hdl);
    });
    server_.set_close_handler([this](connection_hdl hdl){
      connections_.erase(hdl);
    });
    server_.set_message_handler([this](connection_hdl hdl, server_t::message_ptr msg) {
      if (handler_) {
         json j = json::parse(msg->get_payload());
         handler_(j);
      }
    });
  }

  ~WSEndpoint() {
    std::error_code ec;
    set_running(false, ec);
  }

  void set_running(bool running, std::error_code& ec) {
    if (running==running_) return;
    running_ = running;
    if (running_) {
      server_.listen(port_, ec);
      server_.start_accept(ec);
    } else {
      server_.stop_listening(ec);
    }
  }

  void send(const json& value) {
    auto v = value.dump();
    int i = 0;
    for (auto& c : connections_) {
      fmt::print("Sending to connection {}",i++);
      std::error_code e;
      server_.send(c, v, websocketpp::frame::opcode::text, e);
      if (e) {
        fmt::print("Error sending message: {}\n", e.message());
      }
    }
  }

  handler_t& handler() {return handler_;}

  private:
  server_t server_;
  bool running_ = false;
  uint16_t port_;
  handler_t handler_;
  std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> connections_;
};

class PriceServer {
public:
  PriceServer(update_map& prices, boost::asio::io_context& ctx):
    prices_(prices), timer_(ctx)
  {
    m_server.init_asio(&ctx);
    m_server.set_reuse_addr(true);
    m_server.set_open_handler(bind(&PriceServer::on_open, this, _1));
    m_server.set_close_handler(bind(&PriceServer::on_close, this, _1));
    m_server.set_message_handler(bind(&PriceServer::on_message, this, _1, _2));
  }

  ~PriceServer() {
    stop();
  }

  void run(uint16_t port) {
    if (running_) return;
    m_server.listen(port);
    m_server.start_accept();
    running_ = true;

    kick_off_timer();
  }

  void stop() {
    if (m_server.is_listening())
      m_server.stop_listening();
    timer_.cancel();
    for(auto client: m_connections) {
      m_server.close(client, 1001, "Shutting down");
    }
  }

private:
  void kick_off_timer() {
    timer_.expires_from_now(boost::asio::chrono::milliseconds(100));
    timer_.async_wait([this](const boost::system::error_code& error){
      if (error) return ; // we were cancelled
      update_prices();
    });
  }

  void on_open(websocketpp::connection_hdl hdl) {
    std::lock_guard lock(m_mutex);
    m_connections.insert(hdl);
    std::unordered_map<symbol_t, price_t> current_prices;
    prices_.with_state([&current_prices](auto f){current_prices = f;});
    json j(current_prices);
    try_send(hdl, j.dump() );
  }

  void on_close(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_connections.erase(hdl);
    m_subscriptions.erase(hdl);
  }

  void on_message(websocketpp::connection_hdl hdl, WSEndpoint::server_t::message_ptr msg) {
    try {
      json j = json::parse(msg->get_payload());

      if (j["action"] == "subscribe") {
        fmt::print("Subscribe action invoked");
        std::lock_guard lock(m_mutex);
        m_subscriptions[hdl] = j["symbols"].get<std::set<std::string>>();
      }
    } catch (const std::exception &e) {
      fmt::print("Error parsing message: {}\n", e.what());
    }
  }
  bool try_send(websocketpp::connection_hdl hdl, const std::string& txt) {
    std::error_code e;
    m_server.send(hdl, txt, websocketpp::frame::opcode::text, e);
    if (e) {
      fmt::print("Error sending message: {}\n", e.message());
      return false;
    }
    return true;
  }
  void update_prices() {
    std::lock_guard lock(m_mutex);

    std::unordered_map<symbol_t, price_t> changed = prices_.get_changes(true);
    if (!changed.empty()) {
      for (const auto &connection : m_connections) {
        // if ( !m_subscriptions.count(connection) )
        //   continue;
        json update(changed);
        // for(auto& s : m_subscriptions[connection]) {
        //   if (auto i = changed.find(s); i != changed.end()) update[s] =
        //   i->second;
        // }

        try_send(connection, update.dump());
      }
    }

    kick_off_timer();
  }

  WSEndpoint::server_t m_server;
  std::set<websocketpp::connection_hdl,
           std::owner_less<websocketpp::connection_hdl>>
      m_connections;
  std::map<websocketpp::connection_hdl, std::set<std::string>,
           std::owner_less<websocketpp::connection_hdl>>
      m_subscriptions;
  update_map& prices_;
  std::mutex m_mutex;
  boost::asio::steady_timer timer_;
  bool running_ = false;
};

