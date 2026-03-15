#pragma once

#include <string>
#include <variant>

#define PAGE_SIZE 4096

using Literal = std::variant<std::monostate, double, std::string, bool>;

using Byte = uint8_t;
