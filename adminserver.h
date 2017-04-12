#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/enum.hpp>

#include "evtsim_util.h"

namespace evt {
  struct AdminSession : std::enable_shared_from_this<AdminSession> {
    BOOST_ENUM(SessionState,
	       (Initial)
	       (PendingAuth)
	       (Running)
	       );

    typedef std::shared_ptr<std::string> StringRP;

  AdminSession(class AdminServer* server, boost::asio::io_service * _iosvc, evtsim::Logger* logger) :
    _server(server), _state(SessionState::Initial), socket(*_iosvc), _logger(logger), _fd(-1) {}
    void start();
    void set_state(const SessionState& state);
    void arm_read();
    void handle_login(const std::vector<std::string>& tokens);
    void handle_read(const boost::system::error_code& ec);
    void handle_write(const boost::system::error_code& ec, StringRP srp, size_t bytes_sent);
    void handle_help();
    void send(const std::string msg, bool raw=false);
    void handle_command(const std::vector<std::string>& tokens);
    std::shared_ptr<AdminSession> get_ptr() { return shared_from_this(); }

    class AdminServer* _server;
    SessionState _state;
    boost::asio::ip::tcp::socket socket;
    evtsim::Logger* _logger;
    std::string _name;
    boost::asio::streambuf _input;
    int _fd;
  };

  struct AdminContext {
  AdminContext() : error(false) {}
    std::string ctx;
    std::string cmd;
    std::vector<std::string> args;
    std::stringstream response;
    bool error;
  };

  class AdminServer {
    friend class AdminSession;

  public:
  AdminServer() : _acceptor(0) {}
    void init(boost::asio::io_service * iosvc, evtsim::Logger * l, int port, string name);

  public:
    typedef boost::function<void (AdminContext& ctx)> admin_callback;
    typedef std::shared_ptr<AdminSession> AdminSessionRP;

  public:
    void register_admin(string& cmd, string& args, string& help, admin_callback callback);

  private:
    void handle_accept(AdminSessionRP session, const boost::system::error_code& ec);
    void recalculate_limits();

  protected:
    struct AdminCmd {
      std::string ctx;
      std::string cmd;
      std::string args;
      std::string help;
      admin_callback callback;
      std::string key() const { return ctx+" "+cmd; }
    };

    boost::asio::io_service* _iosvc;
    std::vector<AdminCmd> _command_list;
    boost::asio::ip::tcp::acceptor* _acceptor;
    std::string _name;
    evtsim::Logger* _logger;
    std::vector<size_t> _limits;
  };
}
