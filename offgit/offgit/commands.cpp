#include "commands.h"
#include "common.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>
#include <wininet.h>
#include <filesystem>

namespace fs = std::filesystem;

static void runGit(const std::string& cmd) {
    system(cmd.c_str());
}

void printHelp() {
    std::cout << "\n  OffGit v1.0 - Git without internet\n";
    std::cout << "  ─────────────────────────────────────────────────────\n";
    std::cout << "  offgit help                        Show this message\n";
    std::cout << "  offgit status                      Show network + cache status\n";
    std::cout << "  offgit list                        List all cached repos with size\n";
    std::cout << "  offgit search <name>               Search queue by name\n";
    std::cout << "  offgit history                     Show log of all offgit actions\n\n";
    std::cout << "  Queue:\n";
    std::cout << "  offgit queue add <url>             Add a repo to the queue\n";
    std::cout << "  offgit queue remove <url>          Remove a repo from the queue\n";
    std::cout << "  offgit queue list                  List all queued repos\n\n";
    std::cout << "  Cache (needs internet):\n";
    std::cout << "  offgit fetch-all                   Cache all queued repos\n";
    std::cout << "  offgit update <url>                Re-fetch a specific cached repo\n";
    std::cout << "  offgit sync                        Push queued commits when online\n\n";
    std::cout << "  Offline:\n";
    std::cout << "  offgit clone <url> <folder>        Clone from cache, no wifi needed\n";
    std::cout << "  offgit info <url>                  Show branches + size of cached repo\n";
    std::cout << "  offgit diff <url>                  Show changes since last fetch\n\n";
    std::cout << "  Manage:\n";
    std::cout << "  offgit remove <url>                Delete repo from cache\n";
    std::cout << "  offgit rename <url> <newname>      Rename a cached repo\n";
    std::cout << "  offgit pin <url>                   Pin repo (never auto-cleaned)\n";
    std::cout << "  offgit unpin <url>                 Unpin a repo\n";
    std::cout << "  offgit export <url> <path>         Copy cache to USB/folder\n";
    std::cout << "  offgit import <path>               Import repo from USB into cache\n";
    std::cout << "  offgit clean                       Delete all uncached queue entries\n";
    std::cout << "  offgit purge                       Wipe entire cache\n";
    std::cout << "  offgit doctor                      Check + fix broken cached repos\n\n";
}

void queueAdd(const std::string& url) {
    if (url.empty()) { std::cerr << "[OffGit] Usage: offgit queue add <url>\n"; return; }
    if (isBlocked(url)) {
        std::cout << "\n  [OffGit] BLOCKED: '" << url << "'\n";
        std::cout << "  This repo matches a known malware/RAT keyword.\n";
        std::cout << "  If this is a false positive, add an exception manually to:\n";
        std::cout << "  " << getBlocklistPath() << "\n\n";
        return;
    }
    auto queue = loadQueue();
    std::string slug = getSlug(url);
    for (auto& e : queue) {
        if (e.url == url) { std::cout << "[OffGit] '" << slug << "' is already queued.\n"; return; }
    }
    RepoEntry e; e.url = url; e.slug = slug; e.cached = false; e.pinned = false;
    queue.push_back(e);
    saveQueue(queue);
    logHistory("queue add " + url);
    std::cout << "[OffGit] Queued: " << slug << "\n";
}

void queueRemove(const std::string& url) {
    if (url.empty()) { std::cerr << "[OffGit] Usage: offgit queue remove <url>\n"; return; }
    auto queue = loadQueue();
    std::vector<RepoEntry> updated;
    for (auto& e : queue) if (e.url != url) updated.push_back(e);
    saveQueue(updated);
    logHistory("queue remove " + url);
    std::cout << "[OffGit] Removed: " << getSlug(url) << "\n";
}

void queueList() {
    auto queue = loadQueue();
    if (queue.empty()) { std::cout << "[OffGit] Queue is empty. Run 'offgit queue add <url>'\n"; return; }
    std::cout << "\n  Queued repos:\n";
    std::cout << "  ─────────────────────────────────\n";
    for (auto& e : queue) {
        std::cout << "  " << e.slug;
        if (e.cached) std::cout << "  [cached]";
        else          std::cout << "  [not cached]";
        if (e.pinned) std::cout << " [pinned]";
        std::cout << "\n";
    }
    std::cout << "\n";
}

void fetchAll() {
    auto queue = loadQueue();
    if (queue.empty()) { std::cout << "[OffGit] Nothing queued. Run 'offgit queue add <url>'\n"; return; }
    std::cout << "\n  Fetching " << queue.size() << " repo(s)...\n\n";
    for (auto& e : queue) {
        if (isBlocked(e.url)) {
            std::cout << "  BLOCKED " << e.slug << " (matches malware keyword, skipping)\n";
            continue;
        }
        std::string dest = getCacheDir() + "\\" + e.slug;
        std::cout << "  Caching " << e.slug << "...\n";
        if (fs::exists(dest)) runGit("git -C \"" + dest + "\" remote update --prune");
        else                  runGit("git clone --mirror " + e.url + " \"" + dest + "\"");
        e.cached = true;
        std::cout << "  OK " << e.slug << "\n";
        logHistory("cached " + e.url);
    }
    saveQueue(queue);
    std::cout << "\n  All repos cached. You're ready to go offline.\n\n";
}

void cloneRepo(const std::string& url, const std::string& folder) {
    if (url.empty()) { std::cerr << "[OffGit] Usage: offgit clone <url> <folder>\n"; return; }
    auto queue = loadQueue();
    RepoEntry* found = nullptr;
    for (auto& e : queue) if (e.url == url) { found = &e; break; }
    if (!found) {
        std::cout << "\n  [OffGit] Error: '" << url << "' is not queued.\n";
        std::cout << "  Add it while online with: offgit queue add " << url << "\n\n";
        return;
    }
    std::string cachePath = getCacheDir() + "\\" + found->slug;
    if (!fs::exists(cachePath)) {
        std::cout << "\n  [OffGit] Error: '" << found->slug << "' is queued but not cached yet.\n";
        std::cout << "  Run 'offgit fetch-all' while online first.\n\n";
        return;
    }
    std::string dest = folder.empty() ? found->slug : folder;
    std::cout << "\n  Cloning " << found->slug << " -> " << dest << "\n";
    runGit("git clone \"" + cachePath + "\" \"" + dest + "\"");
    logHistory("clone " + url + " -> " + dest);
    std::cout << "  Done. No wifi needed.\n\n";
}

void showStatus() {
    auto queue = loadQueue();
    int  cached = 0;
    for (auto& e : queue) if (e.cached) cached++;
    std::cout << "\n  OffGit Status\n";
    std::cout << "  ─────────────────────────────────\n";
    std::cout << "  Network  : " << (isOnline() ? "Online" : "Offline") << "\n";
    std::cout << "  Queued   : " << queue.size() << " repos\n";
    std::cout << "  Cached   : " << cached << " repos\n";
    std::string cd = getCacheDir();
    if (fs::exists(cd)) std::cout << "  Cache size: " << dirSize(cd) << "\n";
    std::cout << "\n";
}

void syncOnline() {
    if (!isOnline()) { std::cout << "[OffGit] You're offline. Cannot sync.\n"; return; }
    std::cout << "[OffGit] Syncing queued commits...\n";
    auto queue = loadQueue();
    for (auto& e : queue) {
        std::string dest = getCacheDir() + "\\" + e.slug;
        if (!fs::exists(dest)) continue;
        std::cout << "  Pushing " << e.slug << "...\n";
        runGit("git -C \"" + dest + "\" push --mirror");
    }
    logHistory("sync");
    std::cout << "[OffGit] Sync complete.\n";
}

void updateRepo(const std::string& url) {
    if (url.empty()) { std::cerr << "[OffGit] Usage: offgit update <url>\n"; return; }
    if (!isOnline()) { std::cout << "[OffGit] You're offline. Cannot update.\n"; return; }
    std::string slug = getSlug(url);
    std::string dest = getCacheDir() + "\\" + slug;
    if (!fs::exists(dest)) { std::cout << "[OffGit] '" << slug << "' is not cached.\n"; return; }
    std::cout << "  Updating " << slug << "...\n";
    runGit("git -C \"" + dest + "\" remote update --prune");
    logHistory("update " + url);
    std::cout << "  OK " << slug << "\n";
}

void removeRepo(const std::string& url) {
    if (url.empty()) { std::cerr << "[OffGit] Usage: offgit remove <url>\n"; return; }
    std::string slug = getSlug(url);
    std::string dest = getCacheDir() + "\\" + slug;
    if (fs::exists(dest)) {
        fs::remove_all(dest);
        std::cout << "[OffGit] Removed cache for: " << slug << "\n";
    }
    auto queue = loadQueue();
    std::vector<RepoEntry> updated;
    for (auto& e : queue) if (e.url != url) updated.push_back(e);
    saveQueue(updated);
    logHistory("remove " + url);
}

void listCached() {
    auto queue = loadQueue();
    std::cout << "\n  Cached repos:\n";
    std::cout << "  ─────────────────────────────────\n";
    bool any = false;
    for (auto& e : queue) {
        if (!e.cached) continue;
        std::string dest = getCacheDir() + "\\" + e.slug;
        std::string size = fs::exists(dest) ? dirSize(dest) : "?";
        std::cout << "  " << e.slug << "  " << size;
        if (e.pinned) std::cout << "  [pinned]";
        std::cout << "\n";
        any = true;
    }
    if (!any) std::cout << "  No cached repos.\n";
    std::cout << "\n";
}

void searchQueue(const std::string& name) {
    if (name.empty()) { std::cerr << "[OffGit] Usage: offgit search <name>\n"; return; }
    auto queue = loadQueue();
    std::cout << "\n  Search results for '" << name << "':\n";
    std::cout << "  ─────────────────────────────────\n";
    bool any = false;
    for (auto& e : queue) {
        if (e.slug.find(name) != std::string::npos || e.url.find(name) != std::string::npos) {
            std::cout << "  " << e.slug << "  " << (e.cached ? "[cached]" : "[not cached]") << "\n";
            any = true;
        }
    }
    if (!any) std::cout << "  No results.\n";
    std::cout << "\n";
}

void exportRepo(const std::string& url, const std::string& path) {
    if (url.empty() || path.empty()) { std::cerr << "[OffGit] Usage: offgit export <url> <path>\n"; return; }
    std::string slug = getSlug(url);
    std::string src = getCacheDir() + "\\" + slug;
    if (!fs::exists(src)) { std::cout << "[OffGit] '" << slug << "' is not cached.\n"; return; }
    std::string dest = path + "\\" + slug;
    fs::copy(src, dest, fs::copy_options::recursive);
    logHistory("export " + url + " -> " + path);
    std::cout << "[OffGit] Exported " << slug << " to " << dest << "\n";
}

void importRepo(const std::string& path) {
    if (path.empty()) { std::cerr << "[OffGit] Usage: offgit import <path>\n"; return; }
    if (!fs::exists(path)) { std::cout << "[OffGit] Path does not exist: " << path << "\n"; return; }
    std::string slug = fs::path(path).filename().string();
    std::string dest = getCacheDir() + "\\" + slug;
    fs::copy(path, dest, fs::copy_options::recursive);
    auto queue = loadQueue();
    bool exists = false;
    for (auto& e : queue) if (e.slug == slug) { e.cached = true; exists = true; }
    if (!exists) {
        RepoEntry e; e.slug = slug; e.url = "[imported]"; e.cached = true; e.pinned = false;
        queue.push_back(e);
    }
    saveQueue(queue);
    logHistory("import " + path);
    std::cout << "[OffGit] Imported " << slug << " into cache.\n";
}

void infoRepo(const std::string& url) {
    if (url.empty()) { std::cerr << "[OffGit] Usage: offgit info <url>\n"; return; }
    std::string slug = getSlug(url);
    std::string dest = getCacheDir() + "\\" + slug;
    if (!fs::exists(dest)) { std::cout << "[OffGit] '" << slug << "' is not cached.\n"; return; }
    std::cout << "\n  Info: " << slug << "\n";
    std::cout << "  ─────────────────────────────────\n";
    std::cout << "  Size: " << dirSize(dest) << "\n";
    std::cout << "  Branches:\n";
    runGit("git -C \"" + dest + "\" branch -a");
    std::cout << "\n";
}

void diffRepo(const std::string& url) {
    if (url.empty()) { std::cerr << "[OffGit] Usage: offgit diff <url>\n"; return; }
    std::string slug = getSlug(url);
    std::string dest = getCacheDir() + "\\" + slug;
    if (!fs::exists(dest)) { std::cout << "[OffGit] '" << slug << "' is not cached.\n"; return; }
    runGit("git -C \"" + dest + "\" log --oneline ORIG_HEAD..HEAD 2>nul || git -C \"" + dest + "\" log --oneline -10");
}

void cleanUncached() {
    auto queue = loadQueue();
    std::vector<RepoEntry> updated;
    int count = 0;
    for (auto& e : queue) {
        if (!e.cached && !e.pinned) { count++; continue; }
        updated.push_back(e);
    }
    saveQueue(updated);
    logHistory("clean");
    std::cout << "[OffGit] Removed " << count << " uncached repo(s) from queue.\n";
}

void purgeCache() {
    std::cout << "  Are you sure you want to wipe the entire cache? (yes/no): ";
    std::string confirm;
    std::getline(std::cin, confirm);
    if (confirm != "yes") { std::cout << "[OffGit] Cancelled.\n"; return; }
    fs::remove_all(getCacheDir());
    fs::create_directories(getCacheDir());
    auto queue = loadQueue();
    for (auto& e : queue) e.cached = false;
    saveQueue(queue);
    logHistory("purge");
    std::cout << "[OffGit] Cache wiped.\n";
}

void renameRepo(const std::string& url, const std::string& newname) {
    if (url.empty() || newname.empty()) { std::cerr << "[OffGit] Usage: offgit rename <url> <newname>\n"; return; }
    std::string slug = getSlug(url);
    std::string src = getCacheDir() + "\\" + slug;
    std::string dest = getCacheDir() + "\\" + newname;
    if (fs::exists(src)) fs::rename(src, dest);
    auto queue = loadQueue();
    for (auto& e : queue) if (e.url == url) e.slug = newname;
    saveQueue(queue);
    logHistory("rename " + url + " -> " + newname);
    std::cout << "[OffGit] Renamed " << slug << " -> " << newname << "\n";
}

void pinRepo(const std::string& url) {
    if (url.empty()) { std::cerr << "[OffGit] Usage: offgit pin <url>\n"; return; }
    auto queue = loadQueue();
    for (auto& e : queue) if (e.url == url) { e.pinned = true; }
    saveQueue(queue);
    logHistory("pin " + url);
    std::cout << "[OffGit] Pinned: " << getSlug(url) << "\n";
}

void unpinRepo(const std::string& url) {
    if (url.empty()) { std::cerr << "[OffGit] Usage: offgit unpin <url>\n"; return; }
    auto queue = loadQueue();
    for (auto& e : queue) if (e.url == url) { e.pinned = false; }
    saveQueue(queue);
    logHistory("unpin " + url);
    std::cout << "[OffGit] Unpinned: " << getSlug(url) << "\n";
}

void showHistory() {
    std::ifstream f(getHistoryPath());
    if (!f.is_open()) { std::cout << "[OffGit] No history yet.\n"; return; }
    std::cout << "\n  OffGit History\n";
    std::cout << "  ─────────────────────────────────\n";
    std::string line;
    while (std::getline(f, line)) std::cout << "  " << line << "\n";
    std::cout << "\n";
}

void runDoctor() {
    std::cout << "\n  Running doctor...\n";
    std::cout << "  ─────────────────────────────────\n";
    auto queue = loadQueue();
    bool allGood = true;
    for (auto& e : queue) {
        if (!e.cached) continue;
        std::string dest = getCacheDir() + "\\" + e.slug;
        if (!fs::exists(dest)) {
            std::cout << "  MISSING cache for: " << e.slug << " -- marking as not cached\n";
            e.cached = false;
            allGood = false;
            continue;
        }
        std::string check = "git -C \"" + dest + "\" fsck --quiet 2>nul";
        int result = system(check.c_str());
        if (result != 0) {
            std::cout << "  CORRUPT: " << e.slug << " -- try running 'offgit update <url>'\n";
            allGood = false;
        }
        else {
            std::cout << "  OK: " << e.slug << "\n";
        }
    }
    saveQueue(queue);
    if (allGood) std::cout << "  All repos healthy.\n";
    std::cout << "\n";
}