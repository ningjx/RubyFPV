#pragma once
#include <stdint.h>

// Private, same-build Unix socket protocol between ruby_central and ruby_controller.
enum class SessionMode : uint32_t { Stopped, Paired, Search, Inspect };
struct SessionRequest {
   SessionMode mode = SessionMode::Stopped;
   uint32_t vehicle = 0;
   int frequency = 0, firmware = 0;
   int airRate = -1, ecc = -1, lbt = -1, mcstr = -1;
};

constexpr char groundSessionEndpoint[] = "ruby-ground-session-v1";
bool request_session(const SessionRequest& request, const char* endpoint = groundSessionEndpoint);
bool read_session(SessionRequest& snapshot, const char* endpoint = groundSessionEndpoint);
int open_session_listener(const char* endpoint = groundSessionEndpoint);
void serve_session_request(int listener, bool (*apply)(const SessionRequest&), const SessionRequest& state);
const SessionRequest& current_session();
bool apply_session(const SessionRequest& request);
bool resume_saved_session();
