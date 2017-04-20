#ifndef _B_MANAGER
#define _B_MANAGER
#include "adminserver.h"
#include <boost/asio.hpp>
#include <string>
#include <random>
#include <iostream>
#include <functional>

namespace asio = boost::asio;

class Behavior{
public:
  virtual bool login_behavior()=0;
  virtual bool logout_behavior()=0;
  virtual bool new_order_behavior()=0;
  virtual bool modify_order_behavior()=0;
  virtual bool replace_order_behavior()=0;
  virtual bool cancel_order_behavior()=0;
  virtual bool execution_behavior()=0;
};

class DefaultBehavior : public Behavior{
public:
  virtual bool login_behavior(){return true;};
  virtual bool logout_behavior(){return true;};
  virtual bool new_order_behavior(){return true;};
  virtual bool modify_order_behavior(){return true;};
  virtual bool replace_order_behavior(){return true;};
  virtual bool cancel_order_behavior(){return true;};
  virtual bool execution_behavior(){return true;};
};

class RandomBehavior : public Behavior{
public:
  virtual bool login_behavior(){return (rand()%100) < (login_prob * 100);};
  virtual bool logout_behavior(){return (rand()%100) < (logout_prob * 100);};
  virtual bool new_order_behavior(){return (rand()%100) < (new_order_prob * 100);};
  virtual bool modify_order_behavior(){return (rand()%100) < (modify_order_prob * 100);};
  virtual bool replace_order_behavior(){return (rand()%100) < (replace_order_prob * 100);};
  virtual bool cancel_order_behavior(){return (rand()%100) < (cancel_order_prob * 100);};
  virtual bool execution_behavior(){return (rand()%100) < (execution_prob * 100);};
  void set_login_prob(double prob){login_prob = prob;};
  void set_logout_prob(double prob){logout_prob = prob;};
  void set_new_order_prob(double prob){new_order_prob = prob;};
  void set_modify_order_prob(double prob){modify_order_prob = prob;};
  void set_replace_order_prob(double prob){replace_order_prob = prob;};
  void set_cancel_order_prob(double prob){cancel_order_prob = prob;};
  void set_execution_prob(double prob){execution_prob = prob;};
private:
  double login_prob = 1;
  double logout_prob = 1;
  double new_order_prob = 1;
  double modify_order_prob = 1;
  double replace_order_prob = 1;
  double cancel_order_prob = 1;
  double execution_prob = 1;
};

//allow new_order/modify_order/execution... x times
class CountBehavior : public Behavior{
public:
  virtual bool login_behavior(){
    if (login_times > 0){
      login_times--; return true;
    }
    else return false;
  };
  virtual bool logout_behavior(){
    if (logout_times > 0){
      logout_times--; return true;
    }
    else return false;
  };
  virtual bool new_order_behavior(){
    if (new_order_times > 0){
      new_order_times--; return true;
    }
    else return false;
  };
  virtual bool modify_order_behavior(){
    if (modify_order_times > 0){
      modify_order_times--; return true;
    }
    else return false;
  };
  virtual bool replace_order_behavior(){
    if (replace_order_times > 0){
      replace_order_times--; return true;
    }
    else return false;
  };
  virtual bool cancel_order_behavior(){
    if (cancel_order_times > 0){
      cancel_order_times--; return true;
    }
    else return false;
  };
  virtual bool execution_behavior(){
    if (execution_times > 0){
      execution_times--; return true;
    }
    else return false;
  };
  void set_login_times(int x){login_times = x;};
  void set_logout_times(int x){logout_times = x;};
  void set_new_order_times(int x){new_order_times = x;};
  void set_modify_order_times(int x){modify_order_times = x;};
  void set_replace_order_times(int x){replace_order_times = x;};
  void set_cancel_order_times(int x){cancel_order_times = x;};
  void set_execution_times(int x){execution_times = x;};
private:
  int login_times = 0;
  int logout_times = 0;
  int new_order_times = 0;
  int modify_order_times = 0;
  int replace_order_times = 0;
  int cancel_order_times = 0;
  int execution_times = 0;
};

class BehaviorManager{
public:
  BehaviorManager(asio::io_service * io_service, evtsim::Logger * logger, int admin_port);

  void register_status_function(std::function<string()> display_status_func);
  void register_cancel_function(std::function<int()> cancel_func);

  bool login(){return login_behavior->login_behavior();};
  bool logout(){return logout_behavior->logout_behavior();};
  bool new_order(){return new_order_behavior->new_order_behavior();};
  bool modify_order(){return modify_order_behavior->modify_order_behavior();};
  bool replace_order(){return replace_order_behavior->replace_order_behavior();};
  bool cancel_order(){return cancel_order_behavior->cancel_order_behavior();};
  bool execution(){return execution_behavior->execution_behavior();};
  long long execution_qty(){
    if (exe_qty == -1) return -1;
    long long ret = exe_qty;
    exe_qty = 0;
    return ret;
  };

private:
  void reset_to_default();

  void reset(AdminContext& ctx);
  void set_action(AdminContext& ctx);
  void set_to_default(AdminContext& ctx);
  void set_count(AdminContext& ctx);
  void set_random(AdminContext& ctx);
  void set_execution_qty(AdminContext& ctx);
  void display_status(AdminContext& ctx);
  void cancel_all(AdminContext& ctx);

  AdminServer _admin;
  evtsim::Logger * l;
  vector<std::function<string()>> display_status_funcs;
  vector<std::function<int()>> cancel_funcs;

  DefaultBehavior _db;
  RandomBehavior _rb;
  CountBehavior _cb;

  Behavior * login_behavior;
  Behavior * logout_behavior;
  Behavior * new_order_behavior;
  Behavior * modify_order_behavior;
  Behavior * replace_order_behavior;
  Behavior * cancel_order_behavior;
  Behavior * execution_behavior;

  long long exe_qty = -1;
};
#endif
