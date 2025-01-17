#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "../../main.h"

class Config {
  protected :
    std::vector<t_config> http_config;
    std::map<std::string, std::string> mime_types;
  public :
    Config();
    void parsConfigFile(std::string confFile);
    const std::vector<t_config> & getConfig() const;
    void  generateDefaultConfig();
    ~Config();
};

#endif