#include "Response.hpp"
std::vector<std::string> parseMethos(std::string & methods) {
    std::vector<std::string> result;
    std::string tmp;
    for (size_t i = 0; i < methods.length(); i++) {
        for (; methods[i] == ' ' && i < methods.length(); i++);
        for (; methods[i] != ' ' && i < methods.length(); i++) {
            tmp += methods[i];
        }
        result.push_back(tmp);
        tmp.clear();
    }
    return result;
}

int isMethodAllowed(E_METHOD & method, std::vector<std::string> & methods) {
    for (size_t i = 0; i < methods.size(); i++) {
        if (methods[i] == "GET" && method == GET)
            return 1;
        if (methods[i] == "POST" && method == POST)
            return 1;
        if (methods[i] == "DELETE" && method == DELETE)
            return 1;
    }
    return 0;
}

int Response::checkMethods(HttpRequest & req, t_response & response) {
    std::vector<std::string> methods = parseMethos(response.config.Config["methods"]);
    if (req.method != GET && req.method != POST && req.method != DELETE) {
        this->errorResponse(response, req, 501, "Not Implemented"); //because we don't suport methos other than GET, POST, DELETE
        return 0;
    }
    else if (!isMethodAllowed(req.method, methods)) {
        this->errorResponse(response, req, 405, "Method Not Allowed"); //if the  method GET, POST, not set in the config file
        return 0;
    }
    return 1;
}

int Response::checkHeaders(HttpRequest & req, t_response & response) {
    if (req.method == POST) {
        if (!req.headers["Transfer-Encoding"].empty() && req.headers["Transfer-Encoding"] != "chunked") {
            this->errorResponse(response, req, 501, "Not Implemented");
            return 0;
        }else if (req.headers["Content-Length"].empty() && req.headers["Transfer-Encoding"].empty()) {
            this->errorResponse(response, req, 400, "Bad Request");
            return 0;
        }else if (_atoi_(req.headers["Content-Length"]) > _atoi_(response.config.Config["max_body_size"])) {
            this->errorResponse(response, req, 413, "Request Entity Too Large");
            return 0;
        }
    }
    if (req.path.size() > 2048) {
        this->errorResponse(response, req, 414, "URI Too Long");
        return 0;
    }
    return 1;
}

int Response::checkRequest(std::string & request, t_response & response) {
    HttpRequest req = parseHttpRequest(request, response.config);
    if (!this->checkMethods(req, response))
        return 0;
    if (!this->checkHeaders(req, response))
        return 0;
    return 1;
}