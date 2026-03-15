#pragma once
#include "Value.hpp"
#include <cstdint>
#include <vector>

using Byte = uint8_t;

class Chunk
{
public:
  std::vector<Byte> code;
  std::vector<Value> constants;
  std::vector<int> lines;

  void write(uint8_t byte, int line)
  {
    code.push_back(byte);
    lines.push_back(line);
  }

  int addConstant(Value value)
  {
    constants.push_back(value);
    return constants.size() - 1;
  }
};
