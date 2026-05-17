#include <iostream>
#include <string>
#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <vector>
#include <filesystem>
#include "common.h"
#include "commands.h"

int main(int argc, char* argv[]) {
    ensureDirs();

    if (argc < 2) {
        printHelp();
        return 0;
    }

    std::string cmd = argv[1];
    std::string arg1 = (argc >= 3) ? argv[2] : "";
    std::string arg2 = (argc >= 4) ? argv[3] : "";
    std::string arg3 = (argc >= 5) ? argv[4] : "";

    if (cmd == "help")                          printHelp();
    else if (cmd == "queue" && arg1 == "add")        queueAdd(arg2);
    else if (cmd == "queue" && arg1 == "remove")     queueRemove(arg2);
    else if (cmd == "queue" && arg1 == "list")       queueList();
    else if (cmd == "fetch-all")                     fetchAll();
    else if (cmd == "clone")                         cloneRepo(arg1, arg2);
    else if (cmd == "status")                        showStatus();
    else if (cmd == "sync")                          syncOnline();
    else if (cmd == "update")                        updateRepo(arg1);
    else if (cmd == "remove")                        removeRepo(arg1);
    else if (cmd == "list")                          listCached();
    else if (cmd == "search")                        searchQueue(arg1);
    else if (cmd == "export")                        exportRepo(arg1, arg2);
    else if (cmd == "import")                        importRepo(arg1);
    else if (cmd == "info")                          infoRepo(arg1);
    else if (cmd == "diff")                          diffRepo(arg1);
    else if (cmd == "clean")                         cleanUncached();
    else if (cmd == "purge")                         purgeCache();
    else if (cmd == "rename")                        renameRepo(arg1, arg2);
    else if (cmd == "pin")                           pinRepo(arg1);
    else if (cmd == "unpin")                         unpinRepo(arg1);
    else if (cmd == "history")                       showHistory();
    else if (cmd == "doctor")                        runDoctor();
    else {
        std::cerr << "[OffGit] Unknown command: " << cmd << "\n";
        std::cerr << "Run 'offgit help' to see all commands.\n";
        return 1;
    }

    return 0;
}
