#pragma once
#include <string>
#include <variant>

struct NilValue
{
  bool operator==(const NilValue&) const
  {
    return true;
  }
};

struct Object;
using Value = std::variant<NilValue, bool, double, std::string, Object*>;

inline bool operator==(const Value& a, const Value& b)
{
  if (a.index() != b.index()) return false;
  return std::visit(
      [](const auto& x, const auto& y) -> bool
      {
        if constexpr (std::is_same_v<decltype(x), decltype(y)>) return x == y;
        else return false;
      },
      a,
      b);
}
