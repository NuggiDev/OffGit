# OffGit

> Git that works without internet.

Offline-first Git repository manager for Windows. Queue repos while you have a connection, cache them locally, then clone and work completely offline — no wifi needed.

---

## How it works

```
1. Queue repos you want while online
2. Run fetch-all to mirror them locally
3. Go offline — clone and work as normal
4. Sync back when you have internet again
```

---

## Building

You need Visual Studio with the C++ workload installed.

1. Open Visual Studio
2. Create a new C++ Console App
3. Add all `.cpp` and `.h` files from the `offgit/` folder
4. Build in Release x64

To add `offgit` to your PATH so you can run it from anywhere, run `Add-Path.bat` as administrator after building.

---

## Usage

```
offgit queue add <url>         Add a repo to the queue
offgit queue remove <url>      Remove a repo from the queue
offgit queue list              List all queued repos

offgit fetch-all               Cache all queued repos (needs internet)
offgit update <url>            Re-fetch a specific cached repo
offgit sync                    Push queued commits when back online

offgit clone <url> <folder>    Clone from cache — no wifi needed
offgit info <url>              Show branches and size of a cached repo
offgit diff <url>              Show changes since last fetch

offgit list                    List all cached repos with size on disk
offgit search <name>           Search your queue by name
offgit status                  Show network status, queue, and cache info
offgit history                 Show log of all OffGit actions

offgit remove <url>            Delete a repo from cache
offgit rename <url> <newname>  Rename a cached repo
offgit pin <url>               Pin a repo so it never gets auto-cleaned
offgit unpin <url>             Unpin a repo
offgit export <url> <path>     Copy a cached repo to a USB or folder
offgit import <path>           Import a repo from USB into cache
offgit clean                   Remove all uncached entries from queue
offgit purge                   Wipe the entire cache
offgit doctor                  Check and fix broken cached repos
offgit help                    Show all commands
```

---

## Example

```
offgit queue add https://github.com/facebook/react.git
offgit queue add https://github.com/vuejs/vue.git
offgit fetch-all

-- turn off wifi --

offgit clone https://github.com/facebook/react.git my-react
offgit clone https://github.com/vuejs/vue.git my-vue
```

---

## Cache location

All cached repos are stored at:

```
%APPDATA%\OffGit\cache\
```

---

## Blocklist

OffGit has a built-in blocklist that blocks known malware and RAT repos from being queued or cached. You can add your own blocked keywords to:

```
%APPDATA%\OffGit\blocklist.txt
```

One keyword per line. Any repo URL containing that keyword will be blocked.

---

## Disclaimer

This software is provided "AS IS" without any warranty.

- Use at your own risk
- The blocklist is best-effort only and not perfect
- Always verify repos before cloning
- The author is not responsible for any damage or data loss

---

## License

MIT — free to use, modify, and distribute. See [LICENSE](LICENSE) for details.
