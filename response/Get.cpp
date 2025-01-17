#include "Response.hpp"

#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <dirent.h>

void    Response::isDir(HttpRequest& request, int fd){
    std::string htmlData = "";
    std::map<std::string,std::string > files_path;
    std::string path = requests_map[fd].config.Config["root"]  + request.path;
    struct dirent* entry;

    DIR* dir;
    if(requests_map[fd].config.Config["index"] != "")
    {
        std::cout << "index = " <<requests_map[fd].config.Config["index"] <<  std::endl;
        request.path += requests_map[fd].config.Config["index"];
        Response::isFile(request, fd);
        return ;
    }
    dir = opendir(path.c_str());
    if (dir == NULL)
        perror("opendir failed");
    htmlData += "HTTP/1.1 200 OK\n\n <html><head><title>Index of " + request.path + "</title></head><body><h1>index of : " + request.path + "</h1><hr><pre>";
    while ((entry = readdir(dir)) != NULL){
        htmlData += "<a href=\"" + request.path  + entry->d_name + "\">" + entry->d_name + "</a><br>";
        if (strncmp(entry->d_name, "index.", 6) == 0) {
            request.path += entry->d_name;
            Response::isFile(request, fd); 
        closedir(dir);
        return ;
        }
    }
    htmlData += "</pre></html>";
    closedir(dir);
    if(requests_map[fd].config.Config["autoindex"] == "on")
        send(fd, htmlData.c_str(), htmlData.length(), 0);
    else
        this->errorResponse(requests_map[fd], request, 403, "Forbidden");

}
 
void    Response::isFile(HttpRequest& request,int fd){
    if(request.path.find(".php") != std::string::npos) {
        std::string cgi_response = run_cgi(request, this->requests_map[fd].config);
        if (cgi_response == "")
            this->errorResponse(requests_map[fd], request, 500, "Internal Server Error");
        else
            send(fd, cgi_response.c_str(), cgi_response.length(), 0);
    }else
       this->generateResponseFile(requests_map[fd], request, 200, "OK");
    
}


void    Response::Get(HttpRequest& request, int fd)
{
    struct stat stats;
    std::string path = requests_map[fd].config.Config["root"]  + request.path;
    if(stat(path.c_str(),&stats) == 0){
        if(S_ISREG(stats.st_mode)){
           this->isFile(request, fd);
        }else if (S_ISDIR(stats.st_mode)){
            if(request.path.back() == '/') 
                this->isDir(request, fd);
            else {
                std::string httpRes = "HTTP/1.1 301 Moved Permanently\nLocation: " + request.path + "/\n\n";
                send(fd, httpRes.c_str(), httpRes.length(), 0);
            }
        }
    }else
        this->errorResponse(requests_map[fd], request, 404, "Not Found");
}