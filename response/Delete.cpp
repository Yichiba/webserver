#include "Response.hpp"
#include <iostream>

bool Response::isDirectory(const std::string& path)
{
    struct stat fileStat; // get the file status
    if (stat(path.c_str(), &fileStat) == 0) // check if the file exists
    {
        return S_ISDIR(fileStat.st_mode); // check if it is a directory
    }
    else
    {
        std::cerr << "Error: " << std::strerror(errno) << std::endl; // error
        return false;
    }
}

void Response::deleteFile(const std::string& filename, HttpRequest & __unused request, t_response & __unused res)
{
    std::string file = filename + request.path; // get the file path
    if (std::remove(filename.c_str()) != 0) // remove the file
    {
        this->errorResponse(res, request, 500, "Internal Server Error"); // error
    }
    else
        send(res.client_fd, "HTTP/1.1 204 No Content\r\n", 24, 0); // send the response
}

void Response::Delete(t_response & __unused res, HttpRequest & __unused request)
{
    std::string rootPath = res.config.Config["root"] + request.path; // get the root path
    if (access(rootPath.c_str(), F_OK) == -1) // check if file exists
    {
        this->errorResponse(res, request, 404, "Not Found");
        return;
    }
    if (isDirectory(rootPath)) // check if it is a directory
    {
        if (!request.path.empty() && request.path.back() == '/') // check if the path ends with '/'
        {
            if (access(rootPath.c_str(), W_OK) == 0) // check permission
                deleteFile(rootPath, request, res); // delete the file
            else 
                this->errorResponse(res, request, 403, "Forbidden"); // permission denied
        }
        else {
            this->errorResponse(res, request, 409, "Conflict"); // conflict
        }
    }
    else {

        deleteFile(rootPath, request, res); // delete the file
    }
}
