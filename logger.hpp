#pragma once
#include <iostream>
#include <sstream>
#include <string>

class LogInstance {
public:
  LogInstance() = default;
  ~LogInstance() {
#ifdef ENABLE_LOGGING
    std::cout << ss_.str() << std::endl;
#endif
  }

  template <typename T> LogInstance &operator<<(const T &data) {
#ifdef ENABLE_LOGGING
    ss_ << data;
#endif
    return *this;
  }

private:
  std::stringstream ss_;
};

#define LOG_INFO LogInstance() << "[INFO] "
#define LOG_WARNING LogInstance() << "[WARNING] "
#define LOG_ERROR LogInstance() << "[ERROR] "
