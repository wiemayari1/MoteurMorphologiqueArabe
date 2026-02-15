#include "http_server.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

namespace morpho {

HttpServer::HttpServer(int port) : port_(port), running_(false), server_fd_(-1) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::get(const std::string& path, Handler handler) {
    routes_.push_back({"GET", path, handler});
}

void HttpServer::post(const std::string& path, Handler handler) {
    routes_.push_back({"POST", path, handler});
}

void HttpServer::del(const std::string& path, Handler handler) {
    routes_.push_back({"DELETE", path, handler});
}

// NOUVEAU: Méthode PUT
void HttpServer::put(const std::string& path, Handler handler) {
    routes_.push_back({"PUT", path, handler});
}

void HttpServer::start() {
    running_ = true;
    
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "❌ Erreur création socket\n";
        return;
    }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "❌ Erreur bind port " << port_ << "\n";
        close(server_fd_);
        return;
    }

    if (listen(server_fd_, 10) < 0) {
        std::cerr << "❌ Erreur listen\n";
        close(server_fd_);
        return;
    }

    std::cout << "🚀 Serveur HTTP démarré sur http://localhost:" << port_ << "\n";

    while (running_) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_fd < 0) continue;

        std::thread([this, client_fd]() {
            handleClient(client_fd);
        }).detach();
    }
}

void HttpServer::stop() {
    running_ = false;
    if (server_fd_ >= 0) {
        close(server_fd_);
        server_fd_ = -1;
    }
}

void HttpServer::handleClient(int client_fd) {
    char buffer[4096] = {0};
    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }

    std::string request(buffer);
    auto [method, path, body, params] = parseRequest(request);
    
    std::string response = "HTTP/1.1 404 Not Found\r\n"
                          "Content-Type: application/json; charset=utf-8\r\n"
                          "Access-Control-Allow-Origin: *\r\n"
                          "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
                          "Access-Control-Allow-Headers: Content-Type\r\n"
                          "\r\n"
                          "{\"success\":false,\"error\":\"Not Found\"}";

    if (method == "OPTIONS") {
        response = "HTTP/1.1 200 OK\r\n"
                  "Access-Control-Allow-Origin: *\r\n"
                  "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
                  "Access-Control-Allow-Headers: Content-Type\r\n"
                  "\r\n";
    } else {
        for (const auto& [route_method, route_path, handler] : routes_) {
            if (route_method == method && matchPath(route_path, path)) {
                std::string result = handler(method, path, body, params);
                response = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: application/json; charset=utf-8\r\n"
                          "Access-Control-Allow-Origin: *\r\n"
                          "\r\n" + result;
                break;
            }
        }
    }

    send(client_fd, response.c_str(), response.length(), 0);
    close(client_fd);
}

auto HttpServer::parseRequest(const std::string& request) 
    -> std::tuple<std::string, std::string, std::string, std::map<std::string, std::string>> {
    
    std::istringstream stream(request);
    std::string line;
    
    std::getline(stream, line);
    std::istringstream first_line(line);
    std::string method, path, version;
    first_line >> method >> path >> version;

    std::map<std::string, std::string> params;
    size_t q_pos = path.find('?');
    if (q_pos != std::string::npos) {
        std::string query = path.substr(q_pos + 1);
        path = path.substr(0, q_pos);
        params = parseQuery(query);
    }

    int content_length = 0;
    while (std::getline(stream, line) && line != "\r") {
        if (line.find("Content-Length:") == 0) {
            content_length = std::stoi(line.substr(16));
        }
    }

    std::string body;
    if (content_length > 0) {
        body = request.substr(request.length() - content_length);
    }

    return {method, path, body, params};
}

std::map<std::string, std::string> HttpServer::parseQuery(const std::string& query) {
    std::map<std::string, std::string> result;
    std::istringstream stream(query);
    std::string pair;
    
    while (std::getline(stream, pair, '&')) {
        size_t eq = pair.find('=');
        if (eq != std::string::npos) {
            result[pair.substr(0, eq)] = pair.substr(eq + 1);
        }
    }
    
    return result;
}

bool HttpServer::matchPath(const std::string& pattern, const std::string& path) {
    auto pattern_parts = split(pattern, '/');
    auto path_parts = split(path, '/');
    
    if (pattern_parts.size() != path_parts.size()) return false;
    
    for (size_t i = 0; i < pattern_parts.size(); i++) {
        if (!pattern_parts[i].empty() && pattern_parts[i][0] == ':') continue;
        if (pattern_parts[i] != path_parts[i]) return false;
    }
    
    return true;
}

std::vector<std::string> HttpServer::split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::istringstream stream(s);
    std::string item;
    while (std::getline(stream, item, delim)) {
        if (!item.empty()) result.push_back(item);
    }
    return result;
}

} // namespace morpho
