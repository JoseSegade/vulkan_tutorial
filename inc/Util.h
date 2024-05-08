#ifndef INC_UTIL_H_
#define INC_UTIL_H_

#include <string>
#include <vector>

inline std::vector<std::string> split(
  const std::string& line,
  const std::string& delimiter
) {
  std::vector<std::string> split_line;

  std::string l = line;
  size_t pos = 0;
  std::string token;
  while ((pos = l.find(delimiter)) != std::string::npos) {
    token = l.substr(0, pos);
    split_line.push_back(token);
    l.erase(0, pos + delimiter.length());
  }

  split_line.push_back(l);

  return split_line;
}
#endif  // INC_UTIL_H_

