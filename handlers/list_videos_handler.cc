#include <filesystem>

#include <event2/http.h>
#include <event2/keyvalq_struct.h>

#include "absl/strings/match.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"

#include "handlers/list_videos_handler.h"
#include "server_context.h"

namespace {}

void list_videos_handler(struct evhttp_request *request, void *context) {
  auto *server_context = (ServerContext *)context;
  const std::string &path = server_context->file_path;
  const int port = server_context->port;
  struct evbuffer *buffer = evbuffer_new();

  evbuffer_add_printf(
      buffer, R"(
<!DOCTYPE html>
<html lang="en">
<head></head>
<body>
<ul>%s</ul>
</body>
</html>)",
      [path, port, host = server_context->host]() -> std::string {
        std::vector<std::string> lines;
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
          const std::string &path = entry.path();
          if (!absl::EndsWith(path, ".mp4")) {
            // Filter out the files that are not MP4.
            continue;
          }
          const auto &filename =
              std::filesystem::path(path).filename().string();
          std::string line =
              absl::StrFormat("<li><a href=\"http://%s:%d/watch?v=%s\">%s</a>",
                              host, port, filename, filename);
          lines.push_back(std::move(line));
        }
        return absl::StrJoin(lines, "\n");
      }()
                                                         .c_str());

  evhttp_add_header(evhttp_request_get_output_headers(request), "Content-Type",
                    "text/html; charset=utf-8");

  evhttp_send_reply(request, HTTP_OK, "OK", buffer);
  evbuffer_free(buffer);
}
