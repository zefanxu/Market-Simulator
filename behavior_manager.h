#ifndef _B_MANAGER
#define _B_MANAGER
#include "adminserver.h"
#include <boost/asio.hpp>
#include <string>
#include <random>
#include <iostream>

namespace asio = boost::asio;

class Behavior{
public:
  virtual bool login_behavior()=0;
  virtual bool logout_behavior()=0;
  virtual bool neworder_behavior()=0;
  virtual bool modify_order_behavior()=0;
  virtual bool replace_order_behavior()=0;
  virtual bool cancel_order_behavior()=0;
  virtual bool execution_behavior()=0;
};

class DefaultBehavior : public Behavior{
public:
  virtual bool login_behavior(){return true;};
  virtual bool logout_behavior(){return true;};
  virtual bool neworder_behavior(){return true;};
  virtual bool modify_order_behavior(){return true;};
  virtual bool replace_order_behavior(){return true;};
  virtual bool cancel_order_behavior(){return true;};
  virtual bool execution_behavior(){return true;};
};

class RandomBehavior : public Behavior{
public:
  virtual bool login_behavior(){return (rand()%100) < (login_prob * 100);};
  virtual bool logout_behavior(){return (rand()%100) < (logout_prob * 100);};
  virtual bool neworder_behavior(){return (rand()%100) < (neworder_prob * 100);};
  virtual bool modify_order_behavior(){return (rand()%100) < (modify_order_prob * 100);};
  virtual bool replace_order_behavior(){return (rand()%100) < (replace_order_prob * 100);};
  virtual bool cancel_order_behavior(){return (rand()%100) < (cancel_order_prob * 100);};
  virtual bool execution_behavior(){return (rand()%100) < (execution_prob * 100);};
  void set_login_prob(double prob){login_prob = prob;};
  void set_logout_prob(double prob){logout_prob = prob;};
  void set_neworder_prob(double prob){neworder_prob = prob;};
  void set_modify_order_prob(double prob){modify_order_prob = prob;};
  void set_replace_order_prob(double prob){replace_order_prob = prob;};
  void set_cancel_order_prob(double prob){cancel_order_prob = prob;};
  void set_execution_prob(double prob){execution_prob = prob;};
private:
  double login_prob = 1;
  double logout_prob = 1;
  double neworder_prob = 1;
  double modify_order_prob = 1;
  double replace_order_prob = 1;
  double cancel_order_prob = 1;
  double execution_prob = 1;
};

class CustomBehavior : public Behavior{
public:
  virtual bool login_behavior();
  virtual bool logout_behavior();
  virtual bool neworder_behavior();
  virtual bool modify_order_behavior();
  virtual bool replace_order_behavior();
  virtual bool cancel_order_behavior();
  virtual bool execution_behavior();
};

class BehaviorManager{
public:
  BehaviorManager(asio::io_service * io_service, evtsim::Logger * logger, int admin_port);
  bool login(){return login_behavior->login_behavior();};
  bool logout(){return logout_behavior->logout_behavior();};
  bool neworder(){return neworder_behavior->neworder_behavior();};
  bool modify_order(){return modify_order_behavior->modify_order_behavior();};
  bool replace_order(){return replace_order_behavior->replace_order_behavior();};
  bool cancel_order(){return cancel_order_behavior->cancel_order_behavior();};
  bool execution(){return execution_behavior->execution_behavior();};

  void set_login_to_default(AdminContext& ctx);
  void set_logout_to_default(AdminContext& ctx);
  void set_neworder_to_default(AdminContext& ctx);
  void set_modify_order_to_default(AdminContext& ctx);
  void set_replace_order_to_default(AdminContext& ctx);
  void set_cancel_to_default(AdminContext& ctx);
  void set_execution_to_default(AdminContext& ctx);

  void set_login_to_random(AdminContext& ctx);
  void set_logout_to_random(AdminContext& ctx);
  void set_neworder_to_random(AdminContext& ctx);
  void set_modify_order_to_random(AdminContext& ctx);
  void set_replace_to_random(AdminContext& ctx);
  void set_cancel_to_random(AdminContext& ctx);
  void set_execution_to_random(AdminContext& ctx);
private:
  AdminServer _admin;
  evtsim::Logger * l;
  DefaultBehavior db;
  RandomBehavior rb;
  Behavior * login_behavior;
  Behavior * logout_behavior;
  Behavior * neworder_behavior;
  Behavior * modify_order_behavior;
  Behavior * replace_order_behavior;
  Behavior * cancel_order_behavior;
  Behavior * execution_behavior;
};
#endif
