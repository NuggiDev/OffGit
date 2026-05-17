#pragma once
#include <string>

void printHelp();
void queueAdd(const std::string& url);
void queueRemove(const std::string& url);
void queueList();
void fetchAll();
void cloneRepo(const std::string& url, const std::string& folder);
void showStatus();
void syncOnline();
void updateRepo(const std::string& url);
void removeRepo(const std::string& url);
void listCached();
void searchQueue(const std::string& name);
void exportRepo(const std::string& url, const std::string& path);
void importRepo(const std::string& path);
void infoRepo(const std::string& url);
void diffRepo(const std::string& url);
void cleanUncached();
void purgeCache();
void renameRepo(const std::string& url, const std::string& newname);
void pinRepo(const std::string& url);
void unpinRepo(const std::string& url);
void showHistory();
void runDoctor();
