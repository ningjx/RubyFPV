#include "session.h"
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

namespace {
struct Reply {
   uint32_t accepted;
   SessionRequest active;
};

socklen_t address(sockaddr_un& addr, const char* name)
{
   size_t length = strlen(name);
   if (!length || length >= sizeof(addr.sun_path)) return 0;
   addr.sun_family = AF_UNIX;
   memcpy(addr.sun_path + 1, name, length);
   return offsetof(sockaddr_un, sun_path) + 1 + length;
}

bool same_user(int fd)
{
   struct ucred peer{};
   socklen_t len = sizeof(peer);
   return getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &peer, &len) == 0 && peer.uid == geteuid();
}
}

int open_session_listener(const char* endpoint)
{
   sockaddr_un addr{};
   socklen_t size = address(addr, endpoint);
   if (!size) return -1;
   int fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
   if (fd < 0) return -1;
   if (bind(fd, reinterpret_cast<sockaddr*>(&addr), size) == 0 && listen(fd, 4) == 0) return fd;
   close(fd);
   return -1;
}

static bool exchange(const SessionRequest& request, Reply& reply, const char* endpoint)
{
   sockaddr_un addr{};
   socklen_t size = address(addr, endpoint);
   if (!size) return false;
   int fd = socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
   if (fd < 0) return false;
   timeval timeout{12, 0};
   setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
   setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
   bool ok = connect(fd, reinterpret_cast<sockaddr*>(&addr), size) == 0 && same_user(fd)
      && send(fd, &request, sizeof(request), MSG_NOSIGNAL) == sizeof(request)
      && recv(fd, &reply, sizeof(reply), MSG_TRUNC) == sizeof(reply);
   close(fd);
   return ok && reply.accepted == 1;
}

bool request_session(const SessionRequest& request, const char* endpoint)
{
   Reply reply{};
   return exchange(request, reply, endpoint);
}

bool read_session(SessionRequest& snapshot, const char* endpoint)
{
   SessionRequest query;
   query.mode = SessionMode::Inspect;
   Reply reply{};
   if (!exchange(query, reply, endpoint)) return false;
   snapshot = reply.active;
   return true;
}

void serve_session_request(int listener, bool (*apply)(const SessionRequest&), const SessionRequest& state)
{
   int fd = accept4(listener, nullptr, nullptr, SOCK_CLOEXEC);
   if (fd < 0) return;
   timeval timeout{0, 200000};
   setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
   SessionRequest request;
   bool accepted = same_user(fd)
      && recv(fd, &request, sizeof(request), MSG_TRUNC) == sizeof(request)
      && (request.mode == SessionMode::Inspect || apply(request));
   Reply reply{accepted, state};
   send(fd, &reply, sizeof(reply), MSG_NOSIGNAL);
   close(fd);
}
