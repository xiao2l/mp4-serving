
#include <string>
#include <map>

struct ServerContext {
  // The host and port of the current server.
  std::string host;
  int port;

  std::string file_path;
  // The size of the files
  std::map<std::string, size_t> size_map = {};

  // The cached file descriptors
  std::map<std::string, int> fd_map = {};

  std::map<std::string, int> fd_ttl = {};
};
