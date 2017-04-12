
#include <iostream>
#include <iomanip>
#include <boost/bind.hpp>
#include <boost/enum.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

#include <evt_config.h>

#include "adminserver.h"

using namespace std;
using namespace evt;
namespace bip = boost::asio::ip;

void
AdminSession::start() {
  string s_peer = socket.remote_endpoint().address().to_string() + ":" + boost::lexical_cast<string>(socket.remote_endpoint().port());

  _name = "adminsession_"+s_peer;
  _fd = socket.native_handle();
  bip::tcp::no_delay nd(false);
  socket.set_option(nd);
  send("version admin-1\n", /*raw*/true);
  send("auth password\n", /*raw*/true);
  set_state(SessionState::PendingAuth);
  arm_read();
}

void
AdminSession::send(string msg, bool raw) {
  StringRP srp = StringRP(new string(msg));

  if(!raw)
    srp->append(".\n");

  const char* send_head = srp->c_str();
  size_t msg_size = srp->size();

  using namespace boost::asio;
  async_write(socket, buffer(send_head, msg_size), boost::bind(&AdminSession::handle_write, this,
							       boost::asio::placeholders::error,
							       srp,
							       boost::asio::placeholders::bytes_transferred));
}

void
AdminSession::handle_write(const boost::system::error_code& ec, StringRP srp, size_t bytes_sent) {
  if(ec) {
    //_logger->log(Log::WARN, "%s handle_write ec=%s", _name.c_str(), ec.message().c_str());
    return;
  }
}

void
AdminSession::arm_read() {
  _input.prepare(1024);
  boost::asio::async_read_until(socket, _input, '\n', boost::bind(&AdminSession::handle_read, shared_from_this(), _1));
}

void
AdminSession::handle_read(const boost::system::error_code& ec) {
  if(ec || !_input.size()) {
    //_logger->log(Log::INFO, "%s fd=%d disconnect ec=%s", _name.c_str(), _fd, ec.message().c_str());
    socket.close();
    return;
  }

  istream is(&_input);
  string input;
  getline(is, input);
  boost::trim(input);
  //_logger->log(Log::INFO, "%s fd=%d input=%s", _name.c_str(), _fd, input.c_str());

  typedef boost::char_separator<char> Separator;
  typedef boost::tokenizer<Separator> btok;
  Separator sep(" \t", "");
  btok tok(input, sep);

  vector<string> tokens;
  for(auto& token : tok)
    tokens.push_back(token);

  if(tokens.size() && tokens[0]=="login") {
    handle_login(tokens);

  } else if(tokens.size()==1 && (tokens[0]=="help" || tokens[0]=="?")) {
    handle_help();

  } else if(tokens.size()==1 && tokens[0]=="quit") {
    //_logger->log(Log::INFO, "%s fd=%d quit", _name.c_str(), _fd);
    socket.close();
    return;

  } else {
    handle_command(tokens);

  }

  arm_read();
}

void
AdminSession::handle_command(const vector<string>& tokens) {
  if(_state!=SessionState::Running) {
    send("please authenticate\n", /*raw*/true);
    return;
  }

  if(tokens.size()<2) {
    send("invalid input\n");
    return;
  }

  for(const AdminServer::AdminCmd& cmd : _server->_command_list) {
    if(cmd.ctx==tokens[0] && cmd.cmd==tokens[1]) {
      AdminContext context;
      context.ctx = cmd.ctx;
      context.cmd = cmd.cmd;
      for(size_t i=2; i<tokens.size(); i++)
	context.args.push_back(tokens[i]);
      cmd.callback(context);

      string response = context.response.str();
      send(response);
      //_logger->log(Log::INFO, "%s fd=%d response len=%d", _name.c_str(), _fd, (int)response.size());
      return;
    }
  }

  send("unknown command\n");
}

void
AdminSession::handle_login(const vector<string>& tokens) {
  send("ok\n", /*raw*/true);
  set_state(SessionState::Running);
}

void
AdminSession::handle_help() {
  if(_state!=SessionState::Running) {
    send("please authenticate\n", /*raw*/true);
    return;
  }

  const vector<size_t>& limits = _server->_limits;
  stringstream ss;
  for(const AdminServer::AdminCmd& cmd : _server->_command_list) {
    ss << left << " ";
    ss << setw(limits[0]) << cmd.ctx << setw(limits[1]) << cmd.cmd << setw(limits[2]) << cmd.args << setw(limits[3]) << cmd.help << endl;
  }
  send(ss.str());
}

void
AdminSession::set_state(const AdminSession::SessionState& state) {
  //_logger->log(Log::INFO, "%s fd=%d set_state=%s", _name.c_str(), _fd, state.str());
  _state = state;
}

void
AdminServer::init(boost::asio::io_service * iosvc, Logger * l, int port, string name) {
  _iosvc = iosvc;
  _logger = l;
  _name = name;
  try {
    _acceptor = new bip::tcp::acceptor(*_iosvc, bip::tcp::endpoint(bip::tcp::v4(), port));
    _acceptor->set_option(bip::tcp::acceptor::reuse_address(true));
  } catch(boost::system::system_error& e) {
    throw evt_error("adminserver::init: "+string(e.what()));
  }

  AdminSessionRP session = std::make_shared<AdminSession>(this, _iosvc, _logger);
  _acceptor->async_accept(session->socket, boost::bind(&AdminServer::handle_accept, this, session, boost::asio::placeholders::error));
  _acceptor->listen();
  cout << _name << " listening on port=" << port << endl;
}

void
AdminServer::handle_accept(AdminSessionRP session, const boost::system::error_code& ec) {
  session->start();

  AdminSessionRP new_session = std::make_shared<AdminSession>(this, _iosvc, _logger);
  _acceptor->async_accept(new_session->socket, boost::bind(&AdminServer::handle_accept, this, new_session,
							   boost::asio::placeholders::error));
}

void
AdminServer::register_admin(cstr& cmd, cstr& args, cstr& help, admin_callback callback) {
  string::size_type spacesep = cmd.find(' ');
  if(spacesep==string::npos || spacesep>(cmd.size()-1))
    throw evt_error("register_admin: malformed command="+cmd);

  AdminCmd acmd;
  acmd.ctx = cmd.substr(0, spacesep);
  acmd.cmd = cmd.substr(spacesep+1);
  acmd.args = args;
  acmd.help = help;
  acmd.callback = callback;

  _command_list.push_back(acmd);
  recalculate_limits();
}

void
AdminServer::recalculate_limits() {
  _limits.clear();
  _limits.resize(4, 0);
  for(const AdminCmd& cmd : _command_list) {
    _limits[0] = max(_limits[0], cmd.ctx.size()+2);
    _limits[1] = max(_limits[1], cmd.cmd.size()+2);
    _limits[2] = max(_limits[2], cmd.args.size()+2);
    _limits[3] = max(_limits[3], cmd.help.size()+2);
  }
}
