#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <windows.h>
#include <filesystem>

namespace fs = std::filesystem;

struct RepoEntry {
    std::string url;
    std::string slug;
    bool cached;
    bool pinned;
};

inline std::string getAppDataPath() {
    char path[MAX_PATH];
    GetEnvironmentVariableA("APPDATA", path, MAX_PATH);
    return std::string(path) + "\\OffGit";
}

inline std::string getQueuePath() {
    return getAppDataPath() + "\\queued-repos\\queue.json";
}

inline std::string getCacheDir() {
    return getAppDataPath() + "\\cache";
}

inline std::string getHistoryPath() {
    return getAppDataPath() + "\\history.log";
}

inline std::string getBlocklistPath() {
    return getAppDataPath() + "\\blocklist.txt";
}

inline bool isBlocked(const std::string& url) {
    std::string lower = url;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    std::vector<std::string> hardcoded = {
        "quasar", "asyncrat", "async-rat", "njrat", "nj-rat", "nanocore",
        "darkcomet", "dark-comet", "remcos", "netwire", "gh0st", "dcrat",
        "blackshades", "orcus", "thefatrat", "fatrat", "luminositylink",
        "warzonerats", "warzonerat", "revengerat", "revenge-rat", "lokibot",
        "loki-bot", "formbook", "emotet", "wannacry", "wanna-cry",
        "cobaltstr", "keylogger", "ransomware", "botnet",
        "malware-sample", "rat-builder", "ratbuilder", "trojan-builder",
        "stealc", "vidar", "lummastealer", "lumma-stealer", "blankgrabber",
        "blank-grabber", "hijackloader", "guloader", "smokeloader",
        "plugx", "blackremote", "netsupport-rat", "agent-tesla",
        "agentesla", "spynote", "androrat"
    };

    for (auto& kw : hardcoded) {
        if (lower.find(kw) != std::string::npos) return true;
    }

    std::ifstream f(getBlocklistPath());
    if (f.is_open()) {
        std::string line;
        while (std::getline(f, line)) {
            if (line.empty()) continue;
            std::string lline = line;
            std::transform(lline.begin(), lline.end(), lline.begin(), ::tolower);
            if (lower.find(lline) != std::string::npos) return true;
        }
    }

    return false;
}

inline std::string getSlug(const std::string& url) {
    std::string s = url;
    if (s.size() >= 4 && s.substr(s.size() - 4) == ".git")
        s = s.substr(0, s.size() - 4);
    size_t pos = s.rfind('/');
    if (pos != std::string::npos)
        s = s.substr(pos + 1);
    return s;
}

inline void ensureDirs() {
    fs::create_directories(getAppDataPath() + "\\queued-repos");
    fs::create_directories(getCacheDir());
    std::string qp = getQueuePath();
    if (!fs::exists(qp)) {
        std::ofstream f(qp);
        f << "[]\n";
    }
}

inline void logHistory(const std::string& entry) {
    std::ofstream f(getHistoryPath(), std::ios::app);
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[64];
    sprintf_s(buf, "%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    f << "[" << buf << "] " << entry << "\n";
}

inline std::vector<RepoEntry> loadQueue() {
    std::vector<RepoEntry> entries;
    std::ifstream f(getQueuePath());
    if (!f.is_open()) return entries;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty() || line == "[" || line == "]") continue;
        RepoEntry e;
        auto extract = [&](const std::string& key) {
            size_t pos = line.find("\"" + key + "\"");
            if (pos == std::string::npos) return std::string("");
            pos = line.find(':', pos);
            pos = line.find('"', pos);
            size_t end = line.find('"', pos + 1);
            return line.substr(pos + 1, end - pos - 1);
            };
        e.url = extract("url");
        e.slug = extract("slug");
        e.cached = line.find("\"cached\":true") != std::string::npos;
        e.pinned = line.find("\"pinned\":true") != std::string::npos;
        if (!e.url.empty()) entries.push_back(e);
    }
    return entries;
}

inline void saveQueue(const std::vector<RepoEntry>& entries) {
    std::ofstream f(getQueuePath());
    f << "[\n";
    for (size_t i = 0; i < entries.size(); i++) {
        const auto& e = entries[i];
        f << "  {\"url\":\"" << e.url << "\",\"slug\":\"" << e.slug
            << "\",\"cached\":" << (e.cached ? "true" : "false")
            << ",\"pinned\":" << (e.pinned ? "true" : "false") << "}";
        if (i + 1 < entries.size()) f << ",";
        f << "\n";
    }
    f << "]\n";
}

inline bool isOnline() {
    return system("ping -n 1 -w 1000 8.8.8.8 >nul 2>&1") == 0;
}

inline std::string dirSize(const std::string& path) {
    uintmax_t total = 0;
    for (auto& p : fs::recursive_directory_iterator(path))
        if (fs::is_regular_file(p)) total += fs::file_size(p);
    if (total < 1024)                   return std::to_string(total) + " B";
    if (total < 1024 * 1024)            return std::to_string(total / 1024) + " KB";
    if (total < 1024 * 1024 * 1024)     return std::to_string(total / (1024 * 1024)) + " MB";
    return std::to_string(total / (1024 * 1024 * 1024)) + " GB";
}