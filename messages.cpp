#include "evtsim_messages.h"
#include <evt_util.h> // to_base36, from_base36
#include <cstring> // std::memcpy, std::strlen

namespace evt {
namespace ouch {

  constexpr uint64_t MAX_OID = 2176782335; ///< six chars in base36
  constexpr int MAX_GENERATIONS = 1295UL; ///< two chars in base36

  void
  Clordid::set(const char* in) {
    std::memcpy(this, in, sizeof(Clordid));
  }


  bool
  Clordid::set_oid(OrderID order_oid) {
    if(order_oid > MAX_OID)
      return false;

    char buf[sizeof(this->oid)];
    const int len = evt::to_base36(buf, sizeof(this->oid), order_oid);
    if(len <= 0)
      return false;

    evt::fill_and_copy(buf, buf + len, this->oid, this->oid + sizeof(this->oid), '0');
    return true;
  }


  bool
  Clordid::set_prefix(const std::string& str, char padding_char) {
    evt::copy_and_fill(str.cbegin(), str.cend(), this->prefix, this->prefix + sizeof(this->prefix),
                       padding_char);
    return true;
  }


  /// \return new generation, -1 on error
  int
  Clordid::increment_generation() {
    uint64_t gen = 0;
    if(!evt::from_base36(gen, this->generation, sizeof(this->generation)))
      return -1;

    if(gen == MAX_GENERATIONS)
      return -1;

    char gen_buf[sizeof(this->generation)];
    const int new_gen = ++gen;
    if(!evt::to_base36(gen_buf, sizeof(gen_buf), new_gen))
      return -1;

    printf("gen_buf[%.*s]\n", (int)sizeof(gen_buf), gen_buf);

    // strlen is needed here because the actual value may be 1-3 chars
    evt::fill_and_copy(gen_buf, gen_buf + std::strlen(gen_buf), this->generation,
                       this->generation + sizeof(this->generation), '0');
    return new_gen;
  }


  const std::string
  Clordid::str() const {
    return std::string(&this->hpr_marker, sizeof(Clordid));
  }


  /// \return -1 on error
  int
  Clordid::get_generation() const {
    uint64_t gen = 0;
    if(!evt::from_base36(gen, this->generation, sizeof(this->generation)))
      return -1;

    return static_cast<int>(gen);
  }


  OrderID
  Clordid::get_oid() const {
    OrderID out = 0;
    if(!evt::from_base36(out, this->oid, sizeof(this->oid)))
      return 0;
    return out;
  }


} // ouch
} // evt

