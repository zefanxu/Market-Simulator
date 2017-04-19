#include "behavior_manager.h"
using namespace std;
using namespace evt;

BehaviorManager::BehaviorManager(asio::io_service * io_service, evtsim::Logger * logger, int admin_port){
  l = logger;
  _admin.init(io_service, logger, admin_port, string("AdminServer"));
  _admin.register_admin("bm set", "[action] [params...]", "", bind(&BehaviorManager::set_action, this, _1));
  // _admin.register_admin("bm login_default", "", "", bind(&BehaviorManager::set_login_to_default, this, _1));
  // _admin.register_admin("bm logout_default", "", "", bind(&BehaviorManager::set_logout_to_default, this, _1));
  // _admin.register_admin("bm new_order_default", "", "", bind(&BehaviorManager::set_new_order_to_default, this, _1));
  // _admin.register_admin("bm modify_order_default", "", "", bind(&BehaviorManager::set_modify_order_to_default, this, _1));
  // _admin.register_admin("bm replace_order_default", "", "", bind(&BehaviorManager::set_replace_order_to_default, this, _1));
  // _admin.register_admin("bm cancel_order_default", "", "", bind(&BehaviorManager::set_cancel_order_to_default, this, _1));
  // _admin.register_admin("bm execution_default", "", "", bind(&BehaviorManager::set_execution_to_default, this, _1));
  // _admin.register_admin("bm login_random", "[prob]", "", bind(&BehaviorManager::set_login_to_random, this, _1));
  // _admin.register_admin("bm logout_random", "[prob]", "", bind(&BehaviorManager::set_logout_to_random, this, _1));
  // _admin.register_admin("bm new_order_random", "[prob]", "", bind(&BehaviorManager::set_new_order_to_random, this, _1));
  // _admin.register_admin("bm modify_order_random", "[prob]", "", bind(&BehaviorManager::set_modify_order_to_random, this, _1));
  // _admin.register_admin("bm replace_order_random", "[prob]", "", bind(&BehaviorManager::set_replace_order_to_random, this, _1));
  // _admin.register_admin("bm cancel_order_random", "[prob]", "", bind(&BehaviorManager::set_cancel_order_to_random, this, _1));
  // _admin.register_admin("bm execution_random", "[prob]", "", bind(&BehaviorManager::set_execution_to_random, this, _1));
  // _admin.register_admin("bm login_x", "[x]", "", bind(&BehaviorManager::set_login_times, this, _1));
  // _admin.register_admin("bm logout_x", "[x]", "", bind(&BehaviorManager::set_logout_times, this, _1));
  // _admin.register_admin("bm new_order_x", "[x]", "", bind(&BehaviorManager::set_new_order_times, this, _1));
  // _admin.register_admin("bm modify_order_x", "[x]", "", bind(&BehaviorManager::set_modify_order_times, this, _1));
  // _admin.register_admin("bm replace_order_x", "[x]", "", bind(&BehaviorManager::set_replace_order_times, this, _1));
  // _admin.register_admin("bm cancel_order_x", "[x]", "", bind(&BehaviorManager::set_cancel_order_times, this, _1));
  // _admin.register_admin("bm execution_x", "[x]", "", bind(&BehaviorManager::set_execution_times, this, _1));
  // _admin.register_admin("bm execution_qty", "[qty]", "", bind(&BehaviorManager::set_execution_qty, this, _1));
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
  switch (ctx.args[1]) {
    case "login":
      login_behavior = &db;
      break;
    case "logout":
      logout_behavior = &db;
      break;
    case "new_order":
      new_order_behavior = &db;
      break;
    case "modify_order":
      modify_order_behavior = &db;
      break;
    case "replace_order":
      replace_order_behavior = &db;
      break;
    case "cancel_order":
      cancel_order_behavior = &db;
      break;
    case "execution":
      execution_behavior = &db;
      exe_qty = -1;
      break;
    default:
      ctx.response << "invalid parameters" << endl;
      return;
  }
  ctx.response << ctx.args[1] << "set to default" << endl;
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
    ctx.response << "login times=" << x << endl;
  }catch(exception& e){
    ctx.response << "failed" << endl;
    ctx.response << "need a integer parameter > 0" << endl;
    l->write_warning(e.what());
    return;
  }
  switch (ctx.args[1]) {
    case "login":
      xtb.set_login_times(x);
      login_behavior = &xtb;
      break;
    case "logout":
      xtb.set_logout_times(x);
      logout_behavior = &xtb;
      break;
    case "new_order":
      xtb.set_new_order_times(x);
      new_order_behavior = &xtb;
      break;
    case "modify_order":
      xtb.set_modify_order_times(x);
      modify_order_behavior = &xtb;
      break;
    case "replace_order":
      xtb.set_replace_order_times(x);
      replace_order_behavior = &xtb;
      break;
    case "cancel_order":
      xtb.set_cancel_order_times(x);
      cancel_order_behavior = &xtb;
      break;
    case "execution":
      xtb.set_execution_times(x);
      logout_behavior = &xtb;
      break;
    default:
      ctx.response << "invalid parameters" << endl;
      return;
  }
  ctx.response << ctx.args[1] << " " << ctx.args[2] << " times" << endl;
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
void BehaviorManager::set_logout_to_random(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a probability parameter(0<=x<=1)" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability(0<=x<=1)" << endl;
      return;
    }
    rb.set_logout_prob(prob);
    logout_behavior = &rb;
    ctx.response << "set to random, prob=" << prob << endl;
  }catch(exception& e){
    ctx.response << "failed" << endl;
    l->write_warning(e.what());
  }
}
void BehaviorManager::set_new_order_to_random(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a probability parameter(0<=x<=1)" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability(0<=x<=1)" << endl;
      return;
    }
    rb.set_new_order_prob(prob);
    new_order_behavior = &rb;
    ctx.response << "set to random, prob=" << prob << endl;
  }catch(exception& e){
    ctx.response << "failed" << endl;
    l->write_warning(e.what());
  }
}
void BehaviorManager::set_modify_order_to_random(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a probability parameter(0<=x<=1)" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability(0<=x<=1)" << endl;
      return;
    }
    rb.set_modify_order_prob(prob);
    modify_order_behavior = &rb;
    ctx.response << "set to random, prob=" << prob << endl;
  }catch(exception& e){
    ctx.response << "failed" << endl;
    l->write_warning(e.what());
  }
}
void BehaviorManager::set_replace_order_to_random(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a probability parameter(0<=x<=1)" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability(0<=x<=1)" << endl;
      return;
    }
    rb.set_replace_order_prob(prob);
    replace_order_behavior = &rb;
    ctx.response << "set to random, prob=" << prob << endl;
  }catch(exception& e){
    ctx.response << "failed" << endl;
    l->write_warning(e.what());
  }
}
void BehaviorManager::set_cancel_order_to_random(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a probability parameter(0<=x<=1)" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability(0<=x<=1)" << endl;
      return;
    }
    rb.set_cancel_order_prob(prob);
    cancel_order_behavior = &rb;
    ctx.response << "set to random, prob=" << prob << endl;
  }catch(exception& e){
    ctx.response << "failed" << endl;
    l->write_warning(e.what());
  }
}
void BehaviorManager::set_execution_to_random(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a probability parameter(0<=x<=1)" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability(0<=x<=1)" << endl;
      return;
    }
    rb.set_execution_prob(prob);
    execution_behavior = &rb;
    exe_qty = -1;
    ctx.response << "set to random, prob=" << prob << endl;
  }catch(exception& e){
    ctx.response << "failed" << endl;
    l->write_warning(e.what());
  }
}

void BehaviorManager::set_login_to_random(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a probability parameter(0<=x<=1)" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability(0<=x<=1)" << endl;
      return;
    }
    rb.set_login_prob(prob);
    login_behavior = &rb;
    ctx.response << "set to random, prob=" << prob << endl;
  }catch(exception& e){
    ctx.response << "failed" << endl;
    l->write_warning(e.what());
  }
}
