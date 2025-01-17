#include "Response.hpp"

void Response::uploadFile(t_response & __unused res, HttpRequest & __unused request, size_t i, int r)
{
    std::string upload_dir;
    std::cout << "file name: " << request.file_name[r] << std::endl;

    if (res.config.Config["root"].back() != '/')
        res.config.Config["root"] += "/";
    if (res.config.Config["upload_dir"].back() != '/')
        res.config.Config["upload_dir"] += "/";
    upload_dir = res.config.Config["root"] + res.config.Config["upload_dir"];
    std::cout << "UPL:  " << upload_dir << std::endl;
    std::ofstream outfile(upload_dir + request.file_name[r], std::ios::binary);
    outfile.write(request.form_data[i].data(), request.form_data[i].size());
    outfile.close();
}

void Response::Post(t_response & __unused res, HttpRequest & __unused request)
{
    int r = 0;
    if (request.if_post_form_type == FORM_DATA && request.path.find(".php") == SIZE_T_MAX && !res.config.Config["upload_dir"].empty())
    {
        for(size_t i = 0; i < request.form_data.size(); i++)
        {
            if (request.content_type[i] == "file_upload")
            {
                this->uploadFile(res, request, i, r);
                r++;
            }
            else
            {
                std::cout << request.content_names[i] << " : " << request.form_data[i] << std::endl; 
            }
        }
    }
    else
    {
        std::string checkPath = res.config.Config["root"] + request.path;
        struct stat fileStat;
        if (stat(checkPath.c_str(), &fileStat) != 0)
        {
            this->errorResponse(res, request, 404, "Not Found");
            return ;
        }
        if (this->isDirectory(checkPath))
            this->Get(request, res.client_fd);
        else if (request.path.find(".php") != SIZE_T_MAX)
        {
            std::string resp = run_cgi(request, res.config);
            if (resp == "")
                this->errorResponse(res, request, 500, "Internal Server Error");
            else
                send(res.client_fd, resp.c_str(), resp.length(), 0);
        }
        else if (request.path.find(".php") == SIZE_T_MAX)
            this->Get(request, res.client_fd);
    }
}
