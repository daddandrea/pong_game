#pragma once
#ifdef _WIN32

#include <string>
#include <vector>

namespace core {

// Sends GET to github.com/<path> with redirects disabled.
// Returns the Location header value, or empty string on failure.
std::string winhttp_get_redirect_location(const std::string& path);

// Downloads from url, following HTTPS redirects natively.
// Returns the response body, or empty vector on failure.
std::vector<char> winhttp_download(const std::string& url);

} // namespace core
#endif
