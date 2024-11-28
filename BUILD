cc_binary(
  name = "http",
  srcs = ["http.cc"], 
  deps = [
    "@abseil-cpp//absl/log:log",
    "@abseil-cpp//absl/flags:flag",
    "@abseil-cpp//absl/flags:parse",
    "//handlers:content_handler",
    "//handlers:player_handler",
    "//handlers:heartbeat",
    "//handlers:list_videos_handler",
    "//:utils",
    "//:server_context",
  ],
  linkopts = [
    "-levent",
  ],
)

cc_library(
  name = "server_context",
  hdrs = ["server_context.h"],
  deps = [],
  visibility = [
    "//visibility:public",
  ]
)


cc_library(
  name = "utils",
  srcs = ["utils.cc"], 
  hdrs = ["utils.h"],
  deps = [
    "@abseil-cpp//absl/log:log",
    "@abseil-cpp//absl/cleanup:cleanup",
    "@abseil-cpp//absl/strings:str_format",
  ],
)

cc_library(
  name = "server",
  hdrs = ["server.h"],
  deps = [
    "@abseil-cpp//absl/status:status",
    "@abseil-cpp//absl/status:statusor",
  ],
)
