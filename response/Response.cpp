#include "Response.hpp"
#include <sys/stat.h>
#include <dirent.h>
Response::Response() {}


int     Response::isRequestFinished(t_response & res) {
    HttpRequest request = parseHttpRequest(res.request, res.config);
    if (request.method == POST) {
        if (request.headers["Transfer-Encoding"] == "chunked") {
            if (!request.chunked_end)
                return 0;
        }
        else if (static_cast<size_t>(_atoi_(request.headers["Content-Length"])) > request.full_body.length())
            return 0;
    }
    return 1;
}


void Response::response(t_response & __unused res)
{
    HttpRequest request = parseHttpRequest(res.request, res.config);
    if (!this->changeLocation(request, res))
        return;
    if (request.method == POST)
        Post(res, request);
    else if (request.method == DELETE)
        Delete(res, request);
    else if (request.method == GET)
        this->Get(request,res.client_fd );
}

Response::~Response() {}