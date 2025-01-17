#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <map>
#include <unistd.h>

class cgi
{
    private:
        std::map<std::string, std::string> env;
        char **envp;
        std::string head;
        std::string body;
    public:
        cgi(std::string head, std::string body, std::string SCRIPT_NAME, std::string SCRIPT_FILENAME, std::string CONTENT_TYPE, std::string REQUEST_METHOD, std::string CONTENT_LENGTH, std::string QUERY_STRING, std::string SERVER_PROTOCOL, std::string SERVER_SOFTWARE, std::string SERVER_NAME, std::string GATEWAY_INTERFACE, std::string REDIRECT_STATUS);
        ~cgi();
        std::string fill_env(std::string SCRIPT_FILENAME, std::string CGI);
};
#endif