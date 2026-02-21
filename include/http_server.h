#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <functional>
#include <map>
#include <string>
#include <thread>
#include <vector>


namespace morpho {

using Handler = std::function<std::string(
    const std::string &method, const std::string &path, const std::string &body,
    const std::map<std::string, std::string> &params)>;

struct Route {
  std::string method;
  std::string path;
  Handler handler;
};

class HttpServer {
public:
  explicit HttpServer(int port);
  ~HttpServer();

  void get(const std::string &path, Handler handler);
  void post(const std::string &path, Handler handler);
  void del(const std::string &path, Handler handler);
  void put(const std::string &path, Handler handler);

  void start();
  void stop();

private:
  int port_;
  bool running_;
  int server_fd_;
  std::vector<Route> routes_;

  void handleClient(int client_fd);
  auto parseRequest(const std::string &request)
      -> std::tuple<std::string, std::string, std::string,
                    std::map<std::string, std::string>>;
  std::map<std::string, std::string> parseQuery(const std::string &query);
  bool matchPath(const std::string &pattern, const std::string &path);
  std::vector<std::string> split(const std::string &s, char delim);
};

} // namespace morpho

#endif