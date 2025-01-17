#include "Config.hpp"

Config::Config() {
  std::string mimeFile = "mime.types";
  std::ifstream file;
  std::string line;
  file.open(mimeFile);
  if (!file)
    throw std::runtime_error("Error: mime.types file not found");
  while (std::getline(file, line)) {
    std::string key;
    std::string value;
    size_t i = 0;
    while (line[i] == ' ')
      i++;
    if (i >= line.length())
      throw confFileError();
    while (line[i] != ' ' && i < line.length())
    {
      key += line[i];
      i++;
    }
    while (line[i] == ' ')
      i++;
    if (i >= line.length())
      throw confFileError();
    while (line[i] != ' ' && i < line.length())
    {
      value += line[i];
      i++;
    }
    this->mime_types[value] = key;
  }
}

std::string grepValue(std::string & key, std::string target) {
  size_t pos =0;
  while (pos < key.length() &&  key[pos] == ' ')
    pos++;
  pos += std::string(target).length();
  while (pos < key.length() &&  key[pos] == ' ')
    pos++;
  if (pos >= key.length() - 1)
  {
    throw confFileError();
  }
  size_t end_pos =  key.find(";") - pos;
  if (end_pos + pos == SIZE_T_MAX)
  {
    if (key.find("location") != SIZE_T_MAX)
      end_pos = key.length() - pos;
    else
      throw confFileError();
  }
  std::string port = key.substr(pos, end_pos);
  return port;
}

std::string grepLocationName(std::string &line)
{
  size_t i = line.find("location");

  if (i == SIZE_T_MAX)
    throw confFileError();
  i += 8;
  while (line[i] == ' ')
    i++;
  if (i >= line.length())
    throw confFileError();
  std::string location_name;
  while (line[i] != ' ' && line[i] != '{' && i < line.length())
  {
    location_name += line[i];
    i++;
  }
  return location_name;
}


std::string grepKey(std::string &line)
{
  std::string key;
  size_t i = 0;
  while (line[i] == ' ')
    i++;
  if (i >= line.length())
    throw confFileError();
  while (line[i] != ' ' && line[i] != ';' && i < line.length())
  {
    key += line[i];
    i++;
  }
  return key;
}


void Config::parsConfigFile(std::string confFile) {
  std::ifstream file;
  std::string line;
  file.open(confFile);
  if (!file)
    throw  confFileError();
  size_t server_position;
  while (std::getline(file, line)) {
    t_config config;
    server_position = line.find("server {");
    if (server_position != SIZE_T_MAX && server_position == 0) {
      while (std::getline(file, line) && line.find("}") == SIZE_T_MAX)
      {
        if (line.find("location") != SIZE_T_MAX) {
          t_location location;
          location.location_name = grepLocationName(line);
          while (std::getline(file, line) && line.find("}") == SIZE_T_MAX)
          {
            if (line.find("}") != SIZE_T_MAX)
              break;
            location.location[grepKey(line)] = grepValue(line, grepKey(line));
          }
          config.location.push_back(location);
        }else
          config.Config[grepKey(line)] = grepValue(line, grepKey(line));
      }
    }
    if (config.Config.size() > 0) {
      if (config.Config["host_name"].empty())
        config.Config["host_name"] = "127.0.0.1";
      if (config.Config["port"].empty())
        config.Config["port"] = "9090";
      this->http_config.push_back(config);
    }
  }
  file.close();
}


const std::vector<t_config> &  Config::getConfig() const {
  return this->http_config;
}

void  Config::generateDefaultConfig() {
  t_config config;
  config.Config["server_name"] = "localhost";
  config.Config["host_name"] = "127.0.0.1";
  config.Config["port"] = "9090";
  config.Config["root"] = ".";
  config.Config["autoindex"] = "off";
  config.Config["methods"] = "GET POST DELETE";
  this->http_config.push_back(config);
}

Config::~Config() {}