#pragma once

#include <stdexcept>
namespace core {

class FontNotFoundException : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

}
