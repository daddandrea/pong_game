#include "WinHttpFetch.hpp"

#include <array>
#include <windows.h>
#include <winhttp.h>

namespace core {

std::string winhttp_get_redirect_location(const std::string& path) {
    HINTERNET hSession = WinHttpOpen(L"pong-updater/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return {};

    HINTERNET hConnect = WinHttpConnect(hSession, L"github.com", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return {}; }

    std::wstring wpath(path.begin(), path.end());
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", wpath.c_str(),
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return {}; }

    DWORD policy = WINHTTP_OPTION_REDIRECT_POLICY_NEVER;
    WinHttpSetOption(hRequest, WINHTTP_OPTION_REDIRECT_POLICY, &policy, sizeof(policy));

    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(hRequest, nullptr)) {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return {};
    }

    std::string location;
    DWORD size = 0;
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_LOCATION, WINHTTP_HEADER_NAME_BY_INDEX,
        nullptr, &size, WINHTTP_NO_HEADER_INDEX);
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER && size > 0) {
        std::wstring buf(size / sizeof(wchar_t), L'\0');
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_LOCATION, WINHTTP_HEADER_NAME_BY_INDEX,
            buf.data(), &size, WINHTTP_NO_HEADER_INDEX);
        buf.resize(size / sizeof(wchar_t));
        while (!buf.empty() && buf.back() == L'\0') buf.pop_back();
        location.assign(buf.begin(), buf.end());
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return location;
}

std::vector<char> winhttp_download(const std::string& url) {
    const std::string prefix = "https://";
    const auto host_start = url.starts_with(prefix) ? prefix.size() : 0;
    const auto path_start = url.find('/', host_start);
    const std::string host_str = url.substr(host_start, path_start - host_start);
    const std::string path_str = url.substr(path_start);

    std::wstring whost(host_str.begin(), host_str.end());
    std::wstring wpath(path_str.begin(), path_str.end());

    HINTERNET hSession = WinHttpOpen(L"pong-updater/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return {};

    HINTERNET hConnect = WinHttpConnect(hSession, whost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return {}; }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", wpath.c_str(),
        nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return {}; }

    // Default policy follows HTTPS->HTTPS redirects — handles the github.com -> CDN redirect
    // without re-encoding the URL (unlike httplib which broke signed CDN URLs)
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
        !WinHttpReceiveResponse(hRequest, nullptr)) {
        WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
        return {};
    }

    std::vector<char> body;
    DWORD bytesRead = 0;

    std::array<char, 8192> buf;
    while (WinHttpReadData(hRequest, buf.data(), buf.size(), &bytesRead) && bytesRead > 0) {
        body.insert(body.end(), buf.begin(), buf.begin() + bytesRead);
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return body;
}

} // namespace core
