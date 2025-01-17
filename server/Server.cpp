#include "Server.hpp"

Server::Server(){
}

void set_nonblock(int socket) {
    int flags = fcntl(socket, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(socket, F_SETFL, flags);
}

int Server::createServerFd(t_config& conf, struct sockaddr_in & address){
    int socketFd;
    memset(&address, 0, sizeof(address));
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    if(socketFd == -1)
        throw std::runtime_error ("Socket Failure !!\n");
    set_nonblock(socketFd);
    setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(conf.Config["host_name"].c_str());
    address.sin_port = htons(_atoi_(conf.Config["port"]));
    if ( bind(socketFd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(socketFd);
        throw std::runtime_error("bind error\n");
    }
     if (listen(socketFd, BACKLOG) < 0)
        throw std::runtime_error ("listening Failure !!\n");
    this->server_fds.push_back(socketFd);
    std::cout << "SERVER READY ON : http://localhost:" + conf.Config["port"] << std::endl;
    return socketFd;
}


void    Server::fd_set_rest(fd_set &read, fd_set& write){
    FD_ZERO(&read);
    FD_ZERO(&write);
    for(size_t i = 0; i < this->server_fds.size();i++)
        FD_SET(this->server_fds[i], &read);
}

void Server::get_max_fd(int & max, fd_set & __unused readFd) {
    for (int i = 0; i < MAX_CLIENT; i++){
        if(this->client_fds[i] > 0) {
            FD_SET(this->client_fds[i], &readFd);
            if (this->client_fds[i] > max)
                max = this->client_fds[i];   
        }
    }
}

void    Server::setup_clients(fd_set& readFd, int &max, std::map<int, int> & clients_map){
     for(std::vector<int>::iterator it = this->server_fds.begin(); it != this->server_fds.end(); it++){
        if(FD_ISSET(*it, &readFd)){
            int clientFd = accept(*it, (sockaddr *) &this->address, &this->socklen);
            if(clientFd < 0)
            {
                std::cout << "accept error\n";
                continue;
            }
            set_nonblock(clientFd);
            t_response response;
            response.client_fd = clientFd;
            response.server_fd = *it;
            response.config = this->servers[response.server_fd];
            response.request = "";
            this->requests_map[clientFd] = response;
            clients_map[clientFd] = *it;
            for (int i = 0; i < MAX_CLIENT; i++){
                if(this->client_fds[i] == 0){
                    this->client_fds[i] = clientFd;   
                    break;
                }
            }
            this->get_max_fd(max, readFd);
        }
    }
}
 

void    Server::receve_request(std::map<int, int> & __unused clients_map, fd_set &readFd, fd_set &writeFd){
    for(int i = 0; i < MAX_CLIENT; i++) {
        char buffer[BUFFER_SIZE + 1];
        int rec = 0;
        if(FD_ISSET(this->client_fds[i], &readFd) && this->client_fds[i] > 0){
            int fd = this->client_fds[i];
            rec = recv(fd, buffer, sizeof(buffer) - 1, 0);
            if(rec > 0){
                buffer[rec] = '\0';
                FD_SET(this->client_fds[i], &writeFd);
            }else if(rec == 0) {
                // Client has closed the connection
                close(this->client_fds[i]);
                this->client_fds[i] = 0;
                continue;
            }
        }
        if (FD_ISSET(this->client_fds[i], &writeFd)) {
            //to check if the client already in the map and still connected
            this->requests_map[this->client_fds[i]].request.append(buffer, rec); //if the client already we append the new request to the old one
            if (this->isRequestFinished(this->requests_map[this->client_fds[i]])) {
                this->response(this->requests_map[this->client_fds[i]]);
                //clear the request after sending the response ad remove the client from the map
                FD_CLR(this->client_fds[i], &readFd);
                FD_CLR(this->client_fds[i], &writeFd);
                close(this->client_fds[i]);
                this->client_fds[i] = 0;
                this->requests_map.erase(this->client_fds[i]);
            } else
                FD_CLR(this->client_fds[i], &writeFd);
        }
    }
}

std::vector<std::string> splitPorts(std::string & ports) {
    std::vector<std::string> ports_vector;
    std::string port;
    for (size_t i = 0; i < ports.size(); i++) {
        while (ports[i] == ' ' && i < ports.size())
            i++;
        while (ports[i] != ' ' && i < ports.size()) {
            port += ports[i];
            i++;
        }
        ports_vector.push_back(port);
        port.clear();
    }
    return ports_vector;
}

void Server::runServer(){
    this->socklen = sizeof(this->address);
    std::map<int, int> clients_map;
    struct sockaddr_in  address;
    for(size_t i  = 0; i < this->http_config.size(); i++){
        t_config conf = this->http_config[i];
        std::vector<std::string> ports = splitPorts(conf.Config["port"]);
        for(size_t j = 0; j < ports.size(); j++){
            conf.Config["port"] = ports[j];
            int server_fd = createServerFd(conf, address);
            this->servers[server_fd] = conf;
        }

    }
    int max = *(std::max_element(this->server_fds.begin(),this->server_fds.end()));
    for (int i = 0; i < MAX_CLIENT; i++)
        this->client_fds[i] = 0;
    timeval timeout;
    while(1){
        timeout.tv_sec = 0;
        timeout.tv_usec = 1;
        signal(SIGPIPE, SIG_IGN);
        fd_set readFd, writeFd;
        this->fd_set_rest(readFd, writeFd);
        int select_ret = select(max + 1, &readFd, &writeFd, NULL , &timeout);
        if(select_ret < 0)
            throw std::runtime_error("select eroors\n");
        this->get_max_fd(max, readFd);
        this->setup_clients(readFd,max, clients_map);
        this->receve_request(clients_map, readFd, writeFd);
    }
    for (size_t i = 0; i < this->server_fds.size(); i++) {
        close(this->server_fds[i]);
    }
}

Server::~Server() {}