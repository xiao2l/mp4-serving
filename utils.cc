
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netdb.h>
#include <optional>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "absl/cleanup/cleanup.h"
#include "absl/log/log.h"
#include "absl/strings/str_format.h"

std::optional<std::string> get_host_ip() {
  struct ifaddrs *ifaddr;
  char host[NI_MAXHOST];

  if (getifaddrs(&ifaddr) == -1) {
    return std::nullopt;
  }
  absl::Cleanup cleanup = [&ifaddr]() { freeifaddrs(ifaddr); };

  /* Walk through linked list, maintaining head pointer so we
     can free list later. */
  for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL) {
      continue;
    }
    int family = ifa->ifa_addr->sa_family;
    if (family != AF_INET) {
      // Only retrieve AF_INET* interface address.
      continue;
    }

    std::string ifa_name(ifa->ifa_name);
    if (ifa_name != "eno1" && ifa_name != "enp1s0") {
      continue;
    }

    /* Display interface name and family (including symbolic
       form of the latter for the common families). */
    LOG(INFO) << absl::StrFormat("Found ifa_name:%s family:%s(%d)",
                                 ifa->ifa_name,
                                 (family == AF_PACKET)  ? "AF_PACKET"
                                 : (family == AF_INET)  ? "AF_INET"
                                 : (family == AF_INET6) ? "AF_INET6"
                                                        : "???",
                                 family);
    /* For an  display the address. */
    int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host,
                        NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
    if (s != 0) {
      printf("getnameinfo() failed: %s\n", gai_strerror(s));
      return std::nullopt;
    }
    return std::string(host);
  }
  return std::nullopt;
}
