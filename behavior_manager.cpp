#include "behavior_manager.h"
using namespace std;
using namespace evt;

BehaviorManager::BehaviorManager(asio::io_service * io_service, evtsim::Logger * logger, int admin_port){
  l = logger;
  _admin.init(io_service, logger, admin_port, string("AdminServer"));
  _admin.register_admin("bm set", "[action] [params...]", "", bind(&BehaviorManager::set_action, this, _1));
  _admin.register_admin("bm reset", "", "", bind(&BehaviorManager::reset, this, _1));
  _admin.register_admin("bm execute", "[qty]", "", bind(&BehaviorManager::set_execution_qty, this, _1));
  _admin.register_admin("bm status", "", "", bind(&BehaviorManager::display_status, this, _1));
  _admin.register_admin("bm cancel_all", "", "", bind(&BehaviorManager::cancel_all, this, _1));
  reset_to_default();
}

void BehaviorManager::register_status_function(function<string()> display_status_func){
  assert(display_status_func);
  this->display_status_funcs.push_back(display_status_func);
}
void BehaviorManager::register_cancel_function(function<int()> cancel_func){
  assert(cancel_func);
  this->cancel_funcs.push_back(cancel_func);
}

void BehaviorManager::display_status(AdminContext& ctx){
  if (!display_status_funcs.size()) ctx.response << "No status function registered" << endl;
  for (const auto& each_func : display_status_funcs){
    ctx.response << each_func() << endl;
  }
}
void BehaviorManager::cancel_all(AdminContext& ctx){
  if (!cancel_funcs.size()) ctx.response << "No status function registered" << endl;
  for (const auto& each_func : cancel_funcs){
    ctx.response << each_func() << " order(s) canceled" << endl;
  }
}

void BehaviorManager::set_action(AdminContext& ctx){
  if (ctx.args.size() < 1){
    ctx.response << "need at least one parameter" << endl;
    return;
  }
  if (ctx.args[0] == "reset")
    reset(ctx);
  else if (ctx.args[0] == "default")
    set_to_default(ctx);
  else if (ctx.args[0] == "count")
    set_count(ctx);
  else if (ctx.args[0] == "random")
    set_random(ctx);
  else ctx.response << "unsupported action" << endl;
}

void BehaviorManager::reset_to_default(){
  login_behavior = &_db;
  logout_behavior = &_db;
  new_order_behavior = &_db;
  modify_order_behavior = &_db;
  replace_order_behavior = &_db;
  cancel_order_behavior = &_db;
  execution_behavior = &_db;
}

void BehaviorManager::reset(AdminContext& ctx){
  reset_to_default();
  ctx.response << "all set to default" << endl;
}

void BehaviorManager::set_to_default(AdminContext& ctx){
  if (ctx.args.size() != 2){
    ctx.response << "need two parameters to set to default" << endl;
    return;
  }
  if (ctx.args[1] == "login")
    login_behavior = &_db;
  else if (ctx.args[1] == "logout")
    logout_behavior = &_db;
  else if (ctx.args[1] == "new_order")
    new_order_behavior = &_db;
  else if (ctx.args[1] == "modify_order")
    modify_order_behavior = &_db;
  else if (ctx.args[1] == "replace_order")
    replace_order_behavior = &_db;
  else if (ctx.args[1] == "cancel_order")
    cancel_order_behavior = &_db;
  else if (ctx.args[1] == "execution"){
      execution_behavior = &_db;
      exe_qty = -1;
  }
  else{
    ctx.response << "invalid parameters" << endl;
    return;
  }
  ctx.response << ctx.args[1] << " set to default" << endl;
}

void BehaviorManager::set_count(AdminContext& ctx){
  if (ctx.args.size() != 3){
    ctx.response << "need three parameters" << endl;
    return;
  }
  int x;
  try{
    x = stoi(ctx.args[2]);
    if (x < 0){
      ctx.response << "need a integer parameter > 0" << endl;
      return;
    }
  }catch(exception& e){
    ctx.response << "failed" << endl;
    ctx.response << "need a integer parameter > 0" << endl;
    l->write_warning(e.what());
    return;
  }
  if (ctx.args[1] == "login"){
    _cb.set_login_times(x);
    login_behavior = &_cb;
  }
  else if (ctx.args[1] == "logout"){
    _cb.set_logout_times(x);
    logout_behavior = &_cb;
  }
  else if (ctx.args[1] == "new_order"){
    _cb.set_new_order_times(x);
    new_order_behavior = &_cb;
  }
  else if (ctx.args[1] == "modify_order"){
    _cb.set_modify_order_times(x);
    modify_order_behavior = &_cb;
  }
  else if (ctx.args[1] == "replace_order"){
    _cb.set_replace_order_times(x);
    replace_order_behavior = &_cb;
  }
  else if (ctx.args[1] == "cancel_order"){
    _cb.set_cancel_order_times(x);
    cancel_order_behavior = &_cb;
  }
  else if (ctx.args[1] == "execution"){
    _cb.set_execution_times(x);
    execution_behavior = &_cb;
    exe_qty = -1;
  }
  else{
      ctx.response << "invalid parameters" << endl;
      return;
  }
  ctx.response << ctx.args[1] << " " << ctx.args[2] << " times" << endl;
}

void BehaviorManager::set_random(AdminContext& ctx){
  if (ctx.args.size() != 3){
    ctx.response << "need three parameters" << endl;
    return;
  }
  double x;
  try{
    x = stod(ctx.args[2]);
    if (x < 0 or x > 1){
      ctx.response << "need a prob parameter (0<=x<=1)" << endl;
      return;
    }
  }catch(exception& e){
    ctx.response << "failed" << endl;
    ctx.response << "need a prob parameter (0<=x<=1)" << endl;
    l->write_warning(e.what());
    return;
  }
  if (ctx.args[1] == "login"){
    _rb.set_login_prob(x);
    login_behavior = &_rb;
  }
  else if (ctx.args[1] == "logout"){
    _rb.set_logout_prob(x);
    logout_behavior = &_rb;
  }
  else if (ctx.args[1] == "new_order"){
    _rb.set_new_order_prob(x);
    new_order_behavior = &_rb;
  }
  else if (ctx.args[1] == "modify_order"){
    _rb.set_modify_order_prob(x);
    modify_order_behavior = &_rb;
  }
  else if (ctx.args[1] == "replace_order"){
    _rb.set_replace_order_prob(x);
    replace_order_behavior = &_rb;
  }
  else if (ctx.args[1] == "cancel_order"){
    _rb.set_cancel_order_prob(x);
    cancel_order_behavior = &_rb;
  }
  else if (ctx.args[1] == "execution"){
    _rb.set_execution_prob(x);
    execution_behavior = &_rb;
    exe_qty = -1;
  }
  else{
      ctx.response << "invalid parameters" << endl;
      return;
  }
  ctx.response << ctx.args[1] << " set to random, prob=" << x << endl;
}

void BehaviorManager::set_execution_qty(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a integer qty > 0" << endl;
    return;
  }
  try{
    int qty = stoi(ctx.args[0]);
    if (qty <= 0){
      ctx.response << "need a integer qty > 0" << endl;
      return;
    }
    execution_behavior = &_db;
    exe_qty = qty;
    ctx.response << "execution qty=" << qty << endl;
  }catch(exception& e){
    ctx.response << "failed" << endl;
    l->write_warning(e.what());
  }
}
