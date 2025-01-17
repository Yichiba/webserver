#include "../../main.h"

char hexToChar(const std::string & hex) {
    std::istringstream iss(hex);
    int value;
    iss >> std::hex >> value;
    return static_cast<char>(value);
}

std::string urlDecode(const std::string& input) {
    std::string result;
    result.reserve(input.length());

    for (std::size_t i = 0; i < input.length(); ++i) {
        if (input[i] == '%' && i + 2 < input.length()) {
            std::string hex = input.substr(i + 1, 2);
            result += hexToChar(hex);
            i += 2;
        }else if (input[i] == '+')
            result += ' ';
        else
          result += input[i];
    }

    return result;
}

std::map<std::string, std::string> get_header(std::string & key, HttpRequest & httpRequest) {
  for (std::map<std::string, std::string>::iterator it = httpRequest.headers.begin(); it != httpRequest.headers.end(); it++) {
    if (it->first == key)
      return httpRequest.headers;
  }
  std::map<std::string, std::string> empty_map;
  return empty_map;
}

void get_body(std::istringstream & stream, HttpRequest & httpRequest) {
  std::string body;
  if (httpRequest.is_chunked == true) {
    while (std::getline(stream, body)) {
      if (body == "\r")
        break;
      httpRequest.full_body+= body;
    }
  }else {
    while (std::getline(stream, body)) {
      httpRequest.full_body += body + "\n";
    }
  }
  if (httpRequest.full_body.size() == 0)
    httpRequest.has_body = false;
  else
    httpRequest.has_body = true;
}

std::string get_file_name(const std::string & requestBody) {
  int start = requestBody.find("filename=") + 10;
  std::string filename  = "";
  while (requestBody[start] != '"' && requestBody[start] != '\r' && requestBody[start] != '\n' && requestBody[start] != '\0') {
    filename += requestBody[start];
    start++;
  }
  return filename;
}

std::string get_boundary_value(const std::string & request) {
  std::string boundary = "";
  int start = request.find("boundary=") + 9;
  while (request[start] != '\r' && request[start] != '\n' && request[start] != '\0') {
    boundary += request[start];
    start++;
  }
  return boundary;
}

void pars_post_request(std::istringstream & stream, HttpRequest & __unused httpRequest) {
  std::string line;
  std::getline(stream, line);
  int conut = 0;
  while (1)  {
    std::getline(stream, line);
    if (line.find("boundary=-----") != SIZE_T_MAX && conut != 0)
      break;
    conut++;
  }
}

std::string get_key(std::string & line) {
  std::string key = "";
  int start = 0;
  while (line[start] != ':' && line[start] != '\0') {
    key += line[start];
    start++;
  }
  return key;
}

std::string get_value(std::string & line) {
  std::string value = "";
  int start = line.find(":") + 2;
  while (line[start] != '\0' && line[start] != '\r' && line[start] != '\n') {
    value += line[start];
    start++;
  }
  return value;
}

std::map<std::string, std::string> get_headers(std::istringstream & stream) {
  std::map<std::string, std::string> headers;
  std::string header;
  int count = 0;
  while (std::getline(stream, header)) {
    if (header == "\r" && count == 0)
      continue;
    else if (header == "\r" && count != 0)
      break;
    std::string key = get_key(header);
    std::string value = get_value(header);
    headers[key] = value;
    count++;
  }
  return headers;
}

std::string get_name(std::string & line) {
  int start = line.find("name=") + 6;
  std::string name = "";
  while (line[start] != '\"' && line[start] != '\r' && line[start] != '\n' && line[start] != '\0') {
    name += line[start];
    start++;
  }
  return name;
}

void split_body_encrypted_multi_form_data(HttpRequest & httpRequest, std::istringstream & stream) {
  std::string line;
  std::getline(stream, line);
  if (!stream)
    return;
  int i = 0;
  while (line.find(httpRequest.boundary_start) == SIZE_T_MAX)
    std::getline(stream, line);
  std::string form_data;
  line = "";
  while (std::getline(stream, form_data)) {
    if (form_data.find("filename=") != SIZE_T_MAX)
      httpRequest.file_name.push_back(get_file_name(form_data));
    else if (form_data.find("Content-Type:") != SIZE_T_MAX) {
        httpRequest.content_names.push_back(get_name(form_data));
        httpRequest.content_type.push_back(std::string("file_upload"));
    }else if (form_data.find("Content-Disposition: form-data") != SIZE_T_MAX) {
      httpRequest.content_names.push_back(get_name(form_data));
      httpRequest.content_type.push_back(std::string("text"));
    }
    else if (form_data.find(httpRequest.boundary_end) != SIZE_T_MAX) {
      httpRequest.form_data.push_back(line);
      break;
    }
    else if (form_data.find("--" + httpRequest.boundary_start) != SIZE_T_MAX) {
      httpRequest.form_data.push_back(line);
      line = "";
      form_data = "";
      i = 0;
    }
    else {
      if (i == 0)
        i++;
      else {
          line += form_data + "\n";
      }
    }
  }
}

void split_body(HttpRequest & httpRequest, std::istringstream & stream) {
  std::string line = "";
  std::string body = "";
  int count = 0;
  while(std::getline(stream, line)) {
    if (line == "\r" && count == 0)
      continue;
    else if (line == "\r" && count != 0)
      break;
    body += line + "\n";
    count++;
  }
  //split body key name  and value
  std::string key = "";
  std::string value = "";
  int start = 0;
  while (body[start] != '\0') {
    if (body[start] == '=') {
      start++;
      while (body[start] != '&' && body[start] != '\0') {
        value += body[start];
        start++;
      }
      key = httpRequest.if_post_form_type == TEXT_PLAIN ? key : urlDecode(key);
      httpRequest.content_names.push_back(key);
      httpRequest.content_type.push_back(std::string("text"));
      value = httpRequest.if_post_form_type == TEXT_PLAIN ? value : urlDecode(value);
      httpRequest.form_data.push_back(value);
      key = "";
      value = "";
    }
    else if (body[start] == '&') {
      start++;
      while (body[start] != '=' && body[start] != '\0') {
        key += body[start];
        start++;
      }
    }
    else {
      key += body[start];
      start++;
    }
  }
}

void split_body_text_plain(HttpRequest & httpRequest, std::istringstream & stream) {
  std::string key = "";
  std::string value = "";
  std::string line = "";
  int count = 0;
  int index = 0;
  if (!stream)
    return;
  while(std::getline(stream, line)) {
    if (line == "\r" && count == 0)
      continue;
    while (size_t(index) < line.length() && line[index] != '=') {
        key += line[index];
        index++;
    }
    index++;
    while (size_t(index) < line.length() && line[index] != '\0' && line[index] != '\r') {
        value.append(1, line[index]);
        index++;
    }
    httpRequest.content_names.push_back(key);
    httpRequest.content_type.push_back(std::string("text"));
    httpRequest.form_data.push_back(value);
    key = "";
    value = "";
    index = 0;
    count++;
  }
}

void handel_method_post(std::istringstream & stream, HttpRequest & httpRequest) {
  if (!stream)
    return;
  std::string type = "Content-Type";
  std::map<std::string, std::string> content_type_bound = get_header(type, httpRequest);
  httpRequest.content_length = _atoi_(httpRequest.headers["Content-Length"]);
  httpRequest.boundary_start = get_boundary_value(content_type_bound[type]);
  httpRequest.boundary_end = httpRequest.boundary_start + "--";
  std::string target = "Content-Type";
  std::string content_type = get_header(target, httpRequest)[target];
  if (content_type.find("multipart/form-data") != SIZE_T_MAX) {
    httpRequest.if_post_form_type = FORM_DATA;
    split_body_encrypted_multi_form_data(httpRequest, stream);
  }else if (content_type.find("application/x-www-form-urlencoded") != SIZE_T_MAX) {
    httpRequest.if_post_form_type = DEFAULT_FORM;
    split_body(httpRequest, stream);
  }else if (content_type.find("text/plain") != SIZE_T_MAX) {
    httpRequest.if_post_form_type = TEXT_PLAIN;
    split_body_text_plain(httpRequest, stream);
  }
}

void pars_post_query(std::string query, HttpRequest & httpRequest) {
  std::string key = "";
  std::string value = "";
  for (size_t i = 0; i < query.length(); i++) {
    while (i < query .length() && query[i] != '=') {
      key += query[i];
      i++;
    }
    i++;
    if (i >= query.length())
      break;
    while (i < query.length() && query[i] != '&')  {
      value += query[i];
      i++;
    }
    httpRequest.content_names.push_back(urlDecode(key));
    httpRequest.content_type.push_back(std::string("text"));
    httpRequest.form_data.push_back(urlDecode(value));
    key = "";
    value = "";
  }
}


void split_chunked_body(std::istringstream & stream, HttpRequest & __unused httpRequest) {
  int  __unused count = 0;
  std::string __unused body = "";
  std::string line = "";
  if (!stream)
    return;
  while (std::getline(stream, line)) {
    if (count == 0 && line == "\r") {
      count = 0;
      continue;
    }
    else if (count % 2 != 0) {
      //erase /r/n
      if (line !=  "\r" && line != "\n" && line != "") {
        line.erase(line.length() - 1, 1);
        body += line + "&";
      }
    }
    else if (count % 2 == 0 && line == "\r")
      break;
    count++;
  }

  std::string key = "";
  std::string value = "";
  size_t start = 0;
  while (start < body.length()) {
    while (start < body.length() && body[start] != '=') {
      key += body[start];
      start++;
    }
    start++;
    if (start >= body.length())
      break;
    while (start < body.length() && body[start] != '\0' && body[start] != '&') {
      value += body[start];
      start++;
    }
    httpRequest.content_names.push_back(key);
    httpRequest.content_type.push_back(std::string("text"));
    httpRequest.form_data.push_back(value);
    key = "";
    value = "";
    start++;
  }
}

void parst_get_query(std::string query, HttpRequest & httpRequest) {
  std::string key = "";
  std::string value = "";
  for (size_t i = 0; i < query.length(); i++) {
    while (i < query.length() && query[i] != '=') {
      key += query[i];
      i++;
    }
    i++;
    if (i >= query.length())
      break;
    while (i < query.length() && query[i] != '&')  {
      value += query[i];
      i++;
    }
    httpRequest.query_params[urlDecode(key)] = urlDecode(value);
    key = "";
    value = "";
  }
}


std::string turn_chunked_to_normal(std::string request) {
    std::string body = "";
    size_t pos = 0;
    while (pos < request.size()) {
        size_t chunkSizeEnd = request.find("\r\n", pos);
        u_long hexa_chunked_size = std::stoul(request.substr(pos, chunkSizeEnd - pos), nullptr, 16);
        
        if (hexa_chunked_size == 0)
            break;
        pos = chunkSizeEnd + 2; // Move past the current chunk size and CRLF
        
        std::string chunk = request.substr(pos, hexa_chunked_size);
        body += chunk;
        
        pos += hexa_chunked_size + 2; // Move past the current chunk
        if (request.substr(pos, 2) == "\r\n")
          pos += 2;
        // std::cout << "BPDY " << body << "\n";
    }
    while (body[body.length() - 2] == '\r' && body[body.length() - 1] == '\n')
      body.erase(body.length() - 2, 2);
    return body;
}

HttpRequest parseHttpRequest(const std::string & request,  t_config & config) {
  HttpRequest httpRequest;
  std::istringstream stream(request);
  std::string method;
  stream >> method >> httpRequest.path >> httpRequest.version;
  httpRequest.method = method == "GET" ? GET : method == "POST" ? POST : method == "DELETE" ? DELETE : NO_METHOD;
  httpRequest.headers = get_headers(stream);
  httpRequest.is_valid = true;
  httpRequest.is_valid = true;
  httpRequest.ifnotvalid_code_status = 0;
  if (httpRequest.method == POST) {
    std::string body = "";
    if (httpRequest.headers["Transfer-Encoding"] == "chunked")  {
      httpRequest.is_chunked = true;
      httpRequest.chunked_end = 0;
      if (request.find("\r\n0\r\n\r\n") == SIZE_T_MAX)
        return httpRequest;
      httpRequest.chunked_end = 1;
      std::string new_body = std::string(request).substr(request.find("\r\n\r\n") + 4);
      body = turn_chunked_to_normal(new_body);
    }
    if (httpRequest.path.find("?") != SIZE_T_MAX) {
      std::string query = httpRequest.path.substr(httpRequest.path.find("?") + 1);
      pars_post_query(query, httpRequest);
      httpRequest.path = httpRequest.path.substr(0, httpRequest.path.find("?"));
    }
    if (httpRequest.is_chunked && !body.empty()) {
      stream.clear();
      stream.str(body);
    }
    handel_method_post(stream, httpRequest);
    httpRequest.has_query = false;
    httpRequest.has_body = true;
    httpRequest.full_body = request.substr(request.find("\r\n\r\n") + 4);
    if (httpRequest.full_body.length() > (size_t)(_atoi_(config.Config["max_body_size"])))
      httpRequest.ifnotvalid_code_status = 413;
  }
  else {
    httpRequest.content_length = 0;
    httpRequest.has_body = false;
    httpRequest.is_chunked = false;
    httpRequest.has_query = false;
    if (httpRequest.path.find("?") != SIZE_T_MAX) {
      std::string query = httpRequest.path.substr(httpRequest.path.find("?") + 1);
      httpRequest.query = query;
      parst_get_query(query, httpRequest);
      httpRequest.path = httpRequest.path.substr(0, httpRequest.path.find("?"));
      httpRequest.has_query = true;
    }
  }
  return httpRequest;
}
