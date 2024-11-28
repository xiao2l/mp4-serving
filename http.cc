#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/util.h>
#include <event2/visibility.h>

#include <fcntl.h>
#include <regex>
#include <signal.h>
#include <sys/stat.h>

#include "handlers/heartbeat.h"
#include "handlers/list_videos_handler.h"
#include "handlers/content_handler.h"
#include "handlers/player_handler.h"
#include "server_context.h"
#include "utils.h"

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/log.h"
#include "absl/strings/match.h"
#include "absl/strings/str_format.h"

#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>

ABSL_FLAG(int, port, 8001, "serving port, default 8001");
ABSL_FLAG(std::string, file_path, "/home/shawn/Videos/",
          "serving port, default 8001");

void sigUsr1Handler(int sig) {
  fprintf(stderr, "Exiting on SIGUSR1\n");
  void (*_mcleanup)(void);
  _mcleanup = (void (*)(void))dlsym(RTLD_DEFAULT, "_mcleanup");
  if (_mcleanup == NULL)
    fprintf(stderr, "Unable to find gprof exit hook\n");
  else
    _mcleanup();
  _exit(0);
}

void notfound(struct evhttp_request *request, void *params) {
  evhttp_send_error(request, HTTP_NOTFOUND, "Not Found");
}

int main(int argc, char **argv) {
  signal(SIGPIPE, SIG_IGN);
  signal(SIGINT, sigUsr1Handler);

  absl::ParseCommandLine(argc, argv);

  int port = absl::GetFlag(FLAGS_port);
  std::string path = absl::GetFlag(FLAGS_file_path);

  auto host_or = get_host_ip();
  if (!host_or || host_or->empty()) {
    LOG(ERROR) << "Failed to get the host name";
    return 0;
  }
  auto host = std::move(host_or).value();
  LOG(INFO) << "Serving on http://" << host << ":" << port;

  // Create a new event handler
  struct event_base *ebase = event_base_new();
  if (!ebase) {
    LOG(ERROR) << "Failed to create event_base";
    return -1;
  }

  // Create a http server using that handler
  struct evhttp *server = evhttp_new(ebase);

  // Listen locally on port 5000
  struct evhttp_bound_socket *ev_listenr =
      evhttp_bind_socket_with_handle(server, "0.0.0.0", port);
  if (ev_listenr == nullptr) {
    LOG(ERROR) << "Could not bind to 127.0.0.1:5000";
  }
  evutil_socket_t listen_fd = evhttp_bound_socket_get_fd(ev_listenr);

  // Limit serving GET requests
  evhttp_set_allowed_methods(server, EVHTTP_REQ_GET);

  auto server_context = ServerContext{
      .host = host,
      .port = port,
      .file_path = path,
      .size_map = {},
      .fd_map = {},
      .fd_ttl = {},
  };

  evhttp_set_cb(server, "/heartbeat", heartbeat, 0);
  evhttp_set_cb(server, "/", list_videos_handler, &server_context);
  evhttp_set_cb(server, "/watch", player_handler, &server_context);
  // Set the callback for anything not recognized
  evhttp_set_gencb(server, content_handler, &server_context);

  // Start processing queries
  event_base_dispatch(ebase);

  // Free up stuff
  evhttp_free(server);
  event_base_free(ebase);
}
