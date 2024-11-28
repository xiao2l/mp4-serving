#include "handlers/player_handler.h"

#include <string>

#include <event2/buffer.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>

#include "server_context.h"

void player_handler(struct evhttp_request *request, void *privParams) {
  auto *server_context = (ServerContext *)privParams;
  struct evkeyvalq headers;

  // Parse the query for later lookups
  evhttp_parse_query(evhttp_request_get_uri(request), &headers);

  const char *filename = evhttp_find_header(&headers, "v");
  const char *host = server_context->host.c_str();
  const int port = server_context->port;

  struct evbuffer *buffer = evbuffer_new();
  evbuffer_add_printf(buffer, R"(
<!DOCTYPE html>
<html lang="en">
<head>
<title>%s</title>
<script src="https://cdn.jsdelivr.net/npm/clappr@latest/dist/clappr.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/clappr-chromecast-plugin@latest/dist/clappr-chromecast-plugin.min.js"></script>
</head>
<body>
  <div id="player"></div>
  <script>
	var player = new Clappr.Player({
        source: 'http://%s:%d/v/%s',
        plugins: [ChromecastPlugin],
        parentId: '#player',
        chromecast: {
          appId: '9DFB77C0',
          contentType: 'video/mp4',
          media: {
            type: ChromecastPlugin.Movie,
            title: '',
            subtitle: ''
          },
        }
      });
  </script>
</body>
</html>)",
                      filename, host, port, filename);
  // Add a HTTP header, an application/json for the content type here
  evhttp_add_header(evhttp_request_get_output_headers(request), "Content-Type",
                    "text/html; charset=utf-8");

  // Tell we're done and data should be sent back
  evhttp_send_reply(request, HTTP_OK, "OK", buffer);

  evbuffer_free(buffer);
  return;
}
