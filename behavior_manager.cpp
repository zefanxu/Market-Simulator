#include "behavior_manager.h"
using namespace std;
using namespace evt;

BehaviorManager::BehaviorManager(asio::io_service * io_service, evtsim::Logger * logger, int admin_port){
  l = logger;
  _admin.init(io_service, logger, admin_port, string("AdminServer"));
  _admin.register_admin("bm login_default", "", "", boost::bind(&BehaviorManager::set_login_to_default, this, _1));
  _admin.register_admin("bm login_random", "[prob]", "", boost::bind(&BehaviorManager::set_login_to_random, this, _1));
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

void BehaviorManager::set_login_to_random(AdminContext& ctx){
  if (ctx.args.size() != 1){
    ctx.response << "need a probability parameter" << endl;
    return;
  }
  try{
    double prob = stod(ctx.args[0]);
    rb.set_login_prob(prob);
    login_behavior = &rb;
    ctx.response << "set to random, prob=" << prob << endl;
  }catch(exception& e){
    l->write_warning(e.what());
  }
}
