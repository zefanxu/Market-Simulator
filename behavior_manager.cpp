#include "behavior_manager.h"
using namespace std;
using namespace evt;

BehaviorManager::BehaviorManager(asio::io_service * io_service, evtsim::Logger * logger, int admin_port){
  l = logger;
  _admin.init(io_service, logger, admin_port, string("AdminServer"));
  _admin.register_admin("bm set", "[action] [params...]", "", bind(&BehaviorManager::set_action, this, _1));
  _admin.register_admin("bm reset", "", "", bind(&BehaviorManager::reset, this, _1));
  reset_to_default();
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
  login_behavior = &db;
  logout_behavior = &db;
  new_order_behavior = &db;
  modify_order_behavior = &db;
  replace_order_behavior = &db;
  cancel_order_behavior = &db;
  execution_behavior = &db;
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
    login_behavior = &db;
  else if (ctx.args[1] == "logout")
    logout_behavior = &db;
  else if (ctx.args[1] == "new_order")
    new_order_behavior = &db;
  else if (ctx.args[1] == "modify_order")
    modify_order_behavior = &db;
  else if (ctx.args[1] == "replace_order")
    replace_order_behavior = &db;
  else if (ctx.args[1] == "cancel_order")
    cancel_order_behavior = &db;
  else if (ctx.args[1] == "execution"){
      execution_behavior = &db;
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
    xtb.set_login_times(x);
    login_behavior = &xtb;
  }
  else if (ctx.args[1] == "logout"){
    xtb.set_logout_times(x);
    logout_behavior = &xtb;
  }
  else if (ctx.args[1] == "new_order"){
    xtb.set_new_order_times(x);
    new_order_behavior = &xtb;
  }
  else if (ctx.args[1] == "modify_order"){
    xtb.set_modify_order_times(x);
    modify_order_behavior = &xtb;
  }
  else if (ctx.args[1] == "replace_order"){
    xtb.set_replace_order_times(x);
    replace_order_behavior = &xtb;
  }
  else if (ctx.args[1] == "cancel_order"){
    xtb.set_cancel_order_times(x);
    cancel_order_behavior = &xtb;
  }
  else if (ctx.args[1] == "execution"){
    xtb.set_execution_times(x);
    execution_behavior = &xtb;
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
    rb.set_login_prob(x);
    login_behavior = &rb;
  }
  else if (ctx.args[1] == "logout"){
    rb.set_logout_prob(x);
    logout_behavior = &rb;
  }
  else if (ctx.args[1] == "new_order"){
    rb.set_new_order_prob(x);
    new_order_behavior = &rb;
  }
  else if (ctx.args[1] == "modify_order"){
    rb.set_modify_order_prob(x);
    modify_order_behavior = &rb;
  }
  else if (ctx.args[1] == "replace_order"){
    rb.set_replace_order_prob(x);
    replace_order_behavior = &rb;
  }
  else if (ctx.args[1] == "cancel_order"){
    rb.set_cancel_order_prob(x);
    cancel_order_behavior = &rb;
  }
  else if (ctx.args[1] == "execution"){
    rb.set_execution_prob(x);
    execution_behavior = &rb;
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
    execution_behavior = &db;
    exe_qty = qty;
    ctx.response << "execution qty=" << qty << endl;
  }catch(exception& e){
    ctx.response << "failed" << endl;
    l->write_warning(e.what());
  }
}
