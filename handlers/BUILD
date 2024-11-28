cc_library(
  name = "heartbeat",
  srcs = ["heartbeat.cc"], 
  hdrs = ["heartbeat.h"],
  deps = [],
  linkopts = [
    "-levent",
  ],
  visibility = [
    "//visibility:public",
  ]
)

cc_library(
  name = "list_videos_handler",
  srcs = ["list_videos_handler.cc"], 
  hdrs = ["list_videos_handler.h"],
  deps = [
    "@abseil-cpp//absl/strings:strings",
    "@abseil-cpp//absl/strings:str_format",
    "//:server_context",
  ],
  linkopts = [
    "-levent",
  ],
  visibility = [
    "//visibility:public",
  ]
)

cc_library(
  name = "content_handler",
  srcs = ["content_handler.cc"], 
  hdrs = ["content_handler.h"],
  deps = [
    "@abseil-cpp//absl/log:log",
    "@abseil-cpp//absl/strings:strings",
    "@abseil-cpp//absl/strings:str_format",
    "//:server_context",
  ],
  linkopts = [
    "-levent",
  ],
  visibility = [
    "//visibility:public",
  ]
)

cc_library(
  name = "player_handler",
  srcs = ["player_handler.cc"], 
  hdrs = ["player_handler.h"],
  deps = [
    "//:server_context",
  ],
  linkopts = [
    "-levent",
  ],
  visibility = [
    "//visibility:public",
  ]
)
