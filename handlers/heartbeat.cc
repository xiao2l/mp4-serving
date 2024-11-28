#include "handlers/heartbeat.h"

#include <event2/event.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/util.h>
#include <event2/visibility.h>
#include <evhttp.h>

void heartbeat(struct evhttp_request *request, void *privParams) {
  struct evbuffer *buffer;
  struct evkeyvalq headers;
  const char *q;
  // Parse the query for later lookups
  evhttp_parse_query(evhttp_request_get_uri(request), &headers);

  // lookup the 'q' GET parameter
  q = evhttp_find_header(&headers, "q");

  // Create an answer buffer where the data to send back to the browser will be
  // appened
  buffer = evbuffer_new();
  evbuffer_add(buffer, "coucou !", 8);
  evbuffer_add_printf(buffer, "%s", q);

  // Add a HTTP header, an application/json for the content type here
  evhttp_add_header(evhttp_request_get_output_headers(request), "Content-Type",
                    "text/plain");

  // Tell we're done and data should be sent back
  evhttp_send_reply(request, HTTP_OK, "OK", buffer);

  // Free up stuff
  evhttp_clear_headers(&headers);

  evbuffer_free(buffer);

  return;
}
