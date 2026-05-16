#pragma once
#ifdef _WIN32

#include <string>
#include <vector>

namespace core {

// Downloads from url, following HTTPS redirects natively.
// Returns the response body, or empty vector on failure.
std::vector<char> winhttp_download(const std::string& url);

} // namespace core
#endif
