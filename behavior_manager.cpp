#include "behavior_manager.h"
using namespace std;
using namespace evt;

BehaviorManager::BehaviorManager(asio::io_service * io_service, evtsim::Logger * logger, int admin_port){
  l = logger;
  _admin.init(io_service, logger, admin_port, string("AdminServer"));
  _admin.register_admin("bm login_default", "", "", bind(&BehaviorManager::set_login_to_default, this, _1));
  _admin.register_admin("bm logout_default", "", "", bind(&BehaviorManager::set_logout_to_default, this, _1));
  _admin.register_admin("bm neworder_default", "", "", bind(&BehaviorManager::set_neworder_to_default, this, _1));
  _admin.register_admin("bm modify_order_default", "", "", bind(&BehaviorManager::set_modify_order_to_default, this, _1));
  _admin.register_admin("bm replace_order_default", "", "", bind(&BehaviorManager::set_replace_order_to_default, this, _1));
  _admin.register_admin("bm cancel_order_default", "", "", bind(&BehaviorManager::set_cancel_order_to_default, this, _1));
  _admin.register_admin("bm execution_default", "", "", bind(&BehaviorManager::set_execution_to_default, this, _1));
  _admin.register_admin("bm login_random", "[prob]", "", bind(&BehaviorManager::set_login_to_random, this, _1));
  _admin.register_admin("bm logout_random", "[prob]", "", bind(&BehaviorManager::set_logout_to_random, this, _1));
  _admin.register_admin("bm neworder_random", "[prob]", "", bind(&BehaviorManager::set_neworder_to_random, this, _1));
  _admin.register_admin("bm modify_order_random", "[prob]", "", bind(&BehaviorManager::set_modify_order_to_random, this, _1));
  _admin.register_admin("bm replace_order_random", "[prob]", "", bind(&BehaviorManager::set_replace_order_to_random, this, _1));
  _admin.register_admin("bm cancel_order_random", "[prob]", "", bind(&BehaviorManager::set_cancel_order_to_random, this, _1));
  _admin.register_admin("bm execution_random", "[prob]", "", bind(&BehaviorManager::set_execution_to_random, this, _1));
  login_behavior = &db;
  logout_behavior = &db;
  neworder_behavior = &db;
  modify_order_behavior = &db;
  replace_order_behavior = &db;
  cancel_order_behavior = &db;
  execution_behavior = &db;
}

void BehaviorManager::set_login_to_default(AdminContext& ctx){
  login_behavior = &db;
  ctx.response << "set to default\n";
}
void BehaviorManager::set_logout_to_default(AdminContext& ctx){
  logout_behavior = &db;
  ctx.response << "set to default\n";
}
void BehaviorManager::set_neworder_to_default(AdminContext& ctx){
  neworder_behavior = &db;
  ctx.response << "set to default\n";
}
void BehaviorManager::set_modify_order_to_default(AdminContext& ctx){
  modify_order_behavior = &db;
  ctx.response << "set to default\n";
}
void BehaviorManager::set_replace_order_to_default(AdminContext& ctx){
  replace_order_behavior = &db;
  ctx.response << "set to default\n";
}
void BehaviorManager::set_cancel_order_to_default(AdminContext& ctx){
  cancel_order_behavior = &db;
  ctx.response << "set to default\n";
}
void BehaviorManager::set_execution_to_default(AdminContext& ctx){
  execution_behavior = &db;
  ctx.response << "set to default\n";
}
void BehaviorManager::set_logout_to_random(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a probability parameter" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability" << endl;
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
void BehaviorManager::set_neworder_to_random(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a probability parameter" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability" << endl;
      return;
    }
    rb.set_neworder_prob(prob);
    neworder_behavior = &rb;
    ctx.response << "set to random, prob=" << prob << endl;
  }catch(exception& e){
    ctx.response << "failed" << endl;
    l->write_warning(e.what());
  }
}
void BehaviorManager::set_modify_order_to_random(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a probability parameter" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability" << endl;
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
    ctx.response << "need a probability parameter" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability" << endl;
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
    ctx.response << "need a probability parameter" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability" << endl;
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
    ctx.response << "need a probability parameter" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability" << endl;
      return;
    }
    rb.set_execution_prob(prob);
    execution_behavior = &rb;
    ctx.response << "set to random, prob=" << prob << endl;
  }catch(exception& e){
    ctx.response << "failed" << endl;
    l->write_warning(e.what());
  }
}

void BehaviorManager::set_login_to_random(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a probability parameter" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    if (prob > 1 or prob < 0){
      ctx.response << "invalid probability" << endl;
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
