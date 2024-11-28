#include "handlers/content_handler.h"

#include <fcntl.h>
#include <sys/stat.h>

#include <regex>
#include <string>

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>

#include "absl/log/log.h"

#include "server_context.h"

void content_handler(struct evhttp_request *request, void *context) {
  auto *server_context = (ServerContext *)context;
  const std::string& path = server_context->file_path;

  struct evkeyvalq headers;
  auto uri = std::string(evhttp_request_get_uri(request));
  static const std::regex pattern("/v/(.*)");
  std::smatch matches;
  if (!std::regex_match(uri, matches, pattern)) {
    evhttp_send_error(request, HTTP_NOTFOUND, "Not Found");
    return;
  }
  // Match found
  const std::string& filename = matches[1].str();
  std::string fullpath = path + filename;
  // TODO: fix hte c_str
  if (evhttp_parse_query(uri.c_str(), &headers)== -1) {
    LOG(ERROR) << "Parse query failed";
  }
  struct evkeyvalq *kv = evhttp_request_get_input_headers(request);

  // lookup the 'q' GET parameter
  // q = evhttp_find_header(&headers, "q");
  const char *range = evhttp_find_header(kv, "Range");
  int64_t start = 0, content_length = 1 << 18;
  if (range) {
    static const std::regex range_regex("bytes=(.*)-(.*)");
    std::smatch range_matches;
    std::string range_value(range);
    if (!std::regex_match(range_value, range_matches, range_regex)) {
      // LOG(ERROR) << "cannot match range";
    } else {
      char *e;
      start = strtoull(range_matches[1].str().c_str(), &e, 10);
      std::string end_str = range_matches[2].str();
      if (!end_str.empty()) {
        char *ee;
        content_length =
            strtoull(range_matches[2].str().c_str(), &ee, 10) - start;
      }
    }
  }

  int64_t filesize = 0;
  std::map<std::string, size_t> *size_map = &(server_context->size_map);
  auto iter = size_map->find(filename);
  if (iter == size_map->end()) {
    // Get file size
    struct stat s;
    if (auto ret = stat(fullpath.c_str(), &s); ret == 0) {
      LOG(INFO) << "size of " << fullpath << " is " << s.st_size;
    }
    (*size_map)[filename] = s.st_size;
    filesize = s.st_size;
  } else {
    filesize = iter->second;
  }
  if (content_length + start > filesize) {
    content_length = filesize - start;
  }

  struct evbuffer *buffer = evbuffer_new();
  int filefd;
  std::map<std::string, int> *fd_map = &(server_context->fd_map);
  auto fd_iter = fd_map->find(filename);
  if (fd_iter == fd_map->end()) {
    filefd = open(fullpath.c_str(), O_RDONLY);
    (*fd_map)[filename] = filefd;
  } else {
    filefd = fd_iter->second;
  }

  struct evbuffer_file_segment *file_seg =
      evbuffer_file_segment_new(filefd, start, content_length, /*flags=*/0);
  if (file_seg == nullptr) {
    LOG(ERROR) << "File to get the file segment for " << filename << ", start["
               << start << "]";
    return;
  }

  int read_result = evbuffer_add_file_segment(buffer, file_seg, 0, -1);
  content_length = evbuffer_get_length(buffer);

  // Add a HTTP header, an application/json for the content type here
  evhttp_add_header(evhttp_request_get_output_headers(request),
                    "Content-Length", std::to_string(content_length).c_str());
  evhttp_add_header(evhttp_request_get_output_headers(request), "Content-Type",
                    "text/plain");
  std::string content_range = "bytes " + std::to_string(start) + "-" +
                              std::to_string(content_length - 1 + start) + "/" +
                              std::to_string(filesize);
  evhttp_add_header(evhttp_request_get_output_headers(request), "Content-Range",
                    content_range.c_str());
  // Tell we're done and data should be sent back
  evhttp_send_reply(request, 206, "Partial Content", buffer);

  // Free up stuff
  evhttp_clear_headers(&headers);

  evbuffer_free(buffer);
  evbuffer_file_segment_free(file_seg);

  return;
}
