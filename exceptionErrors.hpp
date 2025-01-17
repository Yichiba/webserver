#ifndef EXCEPTIONERRORS_HPP
#define EXCEPTIONERRORS_HPP

#include "main.h"


class HttpConfig : public std::exception {
  public : 
    const char *what() const throw() {
      return "Exception Error : bad http config";
  }
};

class confFileError : public std::exception {
  public : 
    const char *what() const throw() {
      return "Exception Error : config file";
  }
};

class MethodsException : public std::exception {
  public : 
    const char *what() const throw() {
      return "ConfigFile : methods";
  }
};



#endif
