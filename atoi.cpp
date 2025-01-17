#include "main.h"

int _atoi_(std::string  str)
{
  int ismis = 1;
  size_t count = 0;
  int result = 0;
  if (str[0] == '-' || str[0] == '+') {
    if (str[0] == '-')
      ismis = -1;
    count++;
  }
  while (std::isdigit(str[count]) && count < str.length()) {
    result = (result * 10) + str[count] - 48;
    count++;
  }

  result = result * ismis;

  return result;
}

std::string _itos_(int n)
{
  std::string str;
  bool isNegative = false;

  if (n == 0)
      return "0";
  if (n < 0) {
      isNegative = true;
      n = -n;
  }
  while (n > 0) {
    str.insert(str.begin(), '0' + (n % 10));
    n = n / 10;
  }
  if (isNegative) {
      str.insert(str.begin(), '-');
  }
  return str;
}