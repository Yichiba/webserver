#ifndef MAIN_H
#define MAIN_H

enum E_METHOD
{
  GET,
  POST,
  DELETE,
  NO_METHOD
};

#include <signal.h>
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sstream>
#include <sys/select.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <netdb.h>
#include <algorithm> 

typedef struct HttpRequest {
  E_METHOD method;
  std::string path;
  std::string version;
  bool is_valid;
  int ifnotvalid_code_status;
  bool is_chunked;
  int content_length;
  int has_body;
  int has_query;
  int if_post_form_type;
  int  chunked_end;
  std::vector<std::string> form_data;
  std::vector<std::string> file_name;
  std::vector<std::string> content_type;
  std::vector<std::string> content_names;
  std::map<std::string, std::string> query_params;
  std::string boundary_start;
  std::string boundary_end;
  bool is_ency_upl_file;
  std::map<std::string, std::string> headers;
  std::string query;
  std::string full_body;
  std::string old_req;
} HttpRequest;

typedef struct t_location {
  std::string location_name;
  std::map<std::string, std::string> location;
} t_location;



typedef struct t_config {
  std::map<std::string, std::string> Config;
  std::vector<t_location> location;
} t_config;

#include "parsing/config/Config.hpp"
#include "exceptionErrors.hpp"
// #include "newServer/Server.hpp"
#include "./cgi/cgi.hpp"
// #include "response/Response.hpp"

#define TIME_OUT 1000
#define BACKLOG 120
#define MAX_CLIENT  250
#define BUFFER_SIZE 3000
#define BUFFER_SIZE_BIG 4096
//form types
#define DEFAULT_FORM 200
#define FORM_DATA 201
#define TEXT_PLAIN 202
#define CHUNKED 203


int _atoi_(std::string  str);
void display_config(std::vector<std::vector<t_config> > http_config);
HttpRequest parseHttpRequest(const std::string& request,  t_config & config);
void clear_httprequest(HttpRequest & req);
std::string  run_cgi(HttpRequest & __unused req, t_config & data);
void handle_delete_request(HttpRequest & __unused req, int & __unused client_fd, const t_config  & __unused data);
std::string _itos_(int n);
// void response_errors(int client_fd, int code, const t_config & data);
std::string read_html_file(std::string fhtml, const t_config & data);
std::string enum_to_string(E_METHOD method);
bool isDirectory(const char* path);
char *current_date();
void directory_response(HttpRequest & req, int & client_fd, const t_config & data);
void SyntaxError(const std::string & filename);

#endif