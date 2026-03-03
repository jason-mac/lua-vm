#pragma once

#include <cstdint>
#include <variant>

#define PAGE_SIZE 4096

using Literal = std::variant<std::monostate, double, std::string>;
using Byte = char;
