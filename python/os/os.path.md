# `os.path` — Cross-Platform File Path Manipulation

![Python](https://img.shields.io/badge/python-3.6%2B-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)
![Module](https://img.shields.io/badge/module-os.path-green.svg)
![License](https://img.shields.io/badge/license-MIT-blue.svg)

> A practical, example-driven reference for Python's built-in `os.path` module.

**TL;DR** — `os.path` is a submodule of Python's standard `os` module. It provides a
cross-platform toolkit for **building, splitting, inspecting, and querying file paths**
without hardcoding separators (`\` on Windows, `/` on POSIX). If your code touches the
filesystem, this module is your first stop.

---

## Table of Contents

- [Introduction](#introduction--what-is-ospath)
- [Quick Start](#quick-start)
- [Path Manipulation Functions](#path-manipulation-functions)
- [Path Inspection Functions](#path-inspection-functions)
- [Path Metadata Functions](#path-metadata-functions)
- [Common Real-World Recipes](#common-real-world-recipes)
- [Cheatsheet](#cheatsheet)
- [Cross-Platform Notes & Gotchas](#cross-platform-notes--gotchas)
- [`os.path` vs `pathlib`](#ospath-vs-pathlib--when-to-use-which)
- [License & Contributions](#license--contributions)

---

## Introduction — What is `os.path`?

`os.path` is a submodule of Python's built-in `os` module. It exposes a set of
functions for working with file paths in a way that is **compatible across all
operating systems** — Windows, macOS, and Linux — without you having to worry about
path separators (`\` vs `/`), drive letters, or mount points.

Consider this anti-pattern:

```python
# DON'T do this — breaks on Linux/macOS
path = "C:\\Users\\LOQ\\Documents\\file.txt"
```

Instead, use `os.path.join`:

```python
import os
path = os.path.join("C:", "Users", "LOQ", "Documents", "file.txt")
```

The second form works on any OS because `os.path` delegates to the correct
platform-specific implementation (`posixpath` on Unix, `ntpath` on Windows) at
import time. You write the same code; Python picks the right separator for the host
operating system. This is the single most important reason `os.path` exists:
**write once, run anywhere filesystems are involved.**

Throughout this README you'll see runnable scripts that demonstrate realistic
usage. Every snippet is self-contained — copy, paste, and run.

---

## Quick Start

```python
import os

# Build a path to a config file under the user's home directory
config_path = os.path.join(os.path.expanduser("~"), ".myapp", "config.json")

# Inspect it
print("Full path:    ", config_path)
print("Directory:    ", os.path.dirname(config_path))
print("Filename:     ", os.path.basename(config_path))
print("Extension:    ", os.path.splitext(config_path)[1])
print("Exists?       ", os.path.exists(config_path))
print("Absolute?     ", os.path.isabs(config_path))

# Create the parent folder so the config can be written safely
os.makedirs(os.path.dirname(config_path), exist_ok=True)
print("Parent ready: ", os.path.isdir(os.path.dirname(config_path)))
```

That's 80% of what most developers ever need. The rest of this README expands on
each function with realistic examples.

---

## Path Manipulation Functions

These functions **build or decompose** path strings. They never touch the
filesystem — they're pure string operations tuned to the host OS's rules.

### `os.path.join(*path_components)`

Joins one or more path components using the platform-specific separator. Any
component that starts with a separator resets the path (just like POSIX `cd`).

```python
import os

# Build a log file path for a multi-platform CLI tool
log_dir = os.path.join(os.path.expanduser("~"), ".myapp", "logs")
log_file = os.path.join(log_dir, "app.log")
print(log_file)
# macOS/Linux: /home/alice/.myapp/logs/app.log
# Windows:     C:\Users\alice\.myapp\logs\app.log

# Beware: an absolute component discards everything before it
weird = os.path.join("/var/log", "/etc/hosts")
print(weird)  # /etc/hosts   (the /var/log prefix is gone!)
```

### `os.path.split(path)`

Splits a path into a `(head, tail)` pair, where `Tail` is the last component and
`Head` is everything leading up to it. The opposite of `os.path.join`.

```python
import os

archive = "/data/backups/2024/photos.zip"
head, tail = os.path.split(archive)
print("Head:", head)   # /data/backups/2024
print("Tail:", tail)   # photos.zip

# Useful when iterating over a directory and processing each file
for entry in os.listdir("/var/log"):
    full = os.path.join("/var/log", entry)
    if os.path.isfile(full):
        parent, name = os.path.split(full)
        print(f"Found log file: {name} in {parent}")
```

### `os.path.splitext(path)`

Splits a path into `(root, ext)`, where `ext` includes the leading dot and only
the **last** extension is captured.

```python
import os

files = ["report.tar.gz", "script.py", "README", "data.JSON"]
for f in files:
    root, ext = os.path.splitext(f)
    print(f"{f:20} → root={root!r}, ext={ext!r}")
# report.tar.gz       → root='report.tar', ext='.gz'
# script.py           → root='script',     ext='.py'
# README              → root='README',     ext=''
# data.JSON           → root='data',       ext='.JSON'

# Practical use: filter only .csv files when scanning a folder
csv_files = [
    f for f in os.listdir(".")
    if os.path.splitext(f)[1].lower() == ".csv"
]
```

### `os.path.basename(path)` and `os.path.dirname(path)`

Convenience wrappers around `os.path.split`. `basename` returns the tail;
`dirname` returns the head.

```python
import os

target = os.path.join(os.getcwd(), "src", "main.py")

print("Filename only:", os.path.basename(target))  # main.py
print("Folder only:  ", os.path.dirname(target))   # .../src

# Walk up two parent directories
one_up = os.path.dirname(target)
two_up = os.path.dirname(one_up)
print("Two levels up:", two_up)
```

### `os.path.abspath(path)`

Returns a normalized, absolute version of `path`. Resolves relative paths
against the current working directory. Does **not** resolve symlinks — use
`os.path.realpath` for that.

```python
import os

# Convert a relative path from CLI args into an absolute one
user_input = "../data/input.csv"
absolute = os.path.abspath(user_input)
print("Resolved:", absolute)
# e.g. /home/alice/data/input.csv

# Useful for logging — always print absolute paths so users can find the file
print(f"[INFO] Reading config from {os.path.abspath('config.toml')}")
```

### `os.path.normpath(path)`

Collapses redundant separators and up-level references (`..`, `.`). Useful
after concatenating user-supplied strings.

```python
import os

messy = "/home/alice/./projects/../projects/myapp//src/main.py"
clean = os.path.normpath(messy)
print(clean)
# /home/alice/projects/myapp/src/main.py

# Security use case: prevent directory traversal in a file-serving app
def safe_path(base_dir, user_path):
    full = os.path.normpath(os.path.join(base_dir, user_path))
    if not full.startswith(os.path.abspath(base_dir)):
        raise PermissionError("Attempted path traversal!")
    return full
```

### `os.path.relpath(path, start=os.curdir)`

Returns `path` expressed as a relative path starting from `start`. Invaluable
when generating portable configs or comparing paths across machines.

```python
import os

target = "/home/alice/projects/myapp/src/main.py"
start  = "/home/alice/projects/myapp"
print(os.path.relpath(target, start))  # src/main.py

# Build a relative symlink from anywhere in the filesystem
src = os.path.abspath("config/local.json")
dst = os.path.abspath("bin/local.json")
print("Symlink target should be:", os.path.relpath(src, os.path.dirname(dst)))
```

### `os.path.expanduser(path)`

Replaces a leading `~` with the user's home directory. On Windows this resolves
to `C:\Users\<username>` (or `%USERPROFILE%`); on POSIX, `$HOME`.

```python
import os

# Store per-user application data in the user's home
app_data = os.path.join(os.path.expanduser("~"), ".myapp", "state.db")
os.makedirs(os.path.dirname(app_data), exist_ok=True)
print(f"App state lives at: {app_data}")

# expanduser also handles ~username on POSIX
print(os.path.expanduser("~root"))  # /root (on Linux)
```

### `os.path.commonpath(paths)` and `os.path.commonprefix(paths)`

`commonpath` returns the longest common sub-path of a list of paths.
`commonprefix` does a **character-level** comparison and can produce invalid
paths — prefer `commonpath` unless you know you need the raw prefix.

```python
import os

files = [
    "/home/alice/projects/myapp/src/a.py",
    "/home/alice/projects/myapp/src/b.py",
    "/home/alice/projects/myapp/tests/test_a.py",
]
shared = os.path.commonpath(files)
print("Common root:", shared)
# /home/alice/projects/myapp

# Compute each file's path relative to the shared root
for f in files:
    print(os.path.relpath(f, shared))
# src/a.py
# src/b.py
# tests/test_a.py
```

---

## Path Inspection Functions

These functions **query the filesystem** to tell you what a path points to.
They all return booleans (except `samefile`).

### `os.path.exists(path)`

Returns `True` if `path` exists as **anything** — file, directory, symlink
(even a broken one on some platforms), socket, device, etc.

```python
import os

config_candidates = [
    "./config.yaml",
    "./config.yml",
    "~/.config/myapp/config.yaml",
    "/etc/myapp/config.yaml",
]
for c in config_candidates:
    expanded = os.path.expanduser(c)
    if os.path.exists(expanded):
        print(f"Found config: {expanded}")
        break
else:
    print("No config file found — falling back to defaults")
```

### `os.path.isfile(path)` and `os.path.isdir(path)`

Strict checks: `isfile` is `True` only for regular files; `isdir` is `True`
only for directories. Both return `False` for broken symlinks, sockets, etc.

```python
import os

# Recursively count files by extension under a project root
def count_by_ext(root):
    counts = {}
    for dirpath, _, filenames in os.walk(root):
        for name in filenames:
            full = os.path.join(dirpath, name)
            if not os.path.isfile(full):  # skip symlinks, broken links, etc.
                continue
            ext = os.path.splitext(name)[1].lower() or "(no ext)"
            counts[ext] = counts.get(ext, 0) + 1
    return counts

print(count_by_ext("."))
# {'.py': 42, '.md': 8, '.json': 3, '(no ext)': 1}
```

### `os.path.isabs(path)`

Returns `True` if `path` is absolute. On Windows, both `C:\...` and `\\server\share\...`
count as absolute.

```python
import os

paths = ["/var/log", "data/input.csv", "C:\\Users\\alice", "~/notes.md"]
for p in paths:
    print(f"{p:25} absolute={os.path.isabs(p)}")
# /var/log                 absolute=True
# data/input.csv           absolute=False
# C:\Users\alice           absolute=True   (on Windows)
# ~/notes.md               absolute=False  (~ is not expanded automatically
```

### `os.path.islink(path)`

Returns `True` if `path` is a symbolic link. Returns `False` for regular files
and directories. On Windows, this works for symlinks created with `mklink`.

```python
import os

# Find all symlinks in a directory tree (useful for auditing)
symlinks = []
for dirpath, dirnames, filenames in os.walk("/usr/local"):
    for name in filenames + dirnames:
        full = os.path.join(dirpath, name)
        if os.path.islink(full):
            target = os.readlink(full)
            symlinks.append((full, target))

for link, target in symlinks[:5]:
    print(f"{link} → {target}")
```

### `os.path.samefile(path1, path2)`

Returns `True` if both paths refer to the **same file** (compares device + inode
on POSIX; same volume + file ID on Windows). Resolves symlinks.

```python
import os

# Detect when a user passes the same file twice
def dedupe_paths(paths):
    seen = []
    unique = []
    for p in paths:
        if not os.path.exists(p):
            continue
        if any(os.path.samefile(p, s) for s in seen):
            continue
        seen.append(p)
        unique.append(p)
    return unique

print(dedupe_paths(["./data.csv", "data.csv", "/tmp/link_to_data.csv"]))
# ['./data.csv']  — the other two point to the same inode
```

### `os.path.ismount(path)`

Returns `True` if `path` is a mount point (drive root on Windows, `/` or any
mounted volume on POSIX).

```python
import os

# Walk up the directory tree until we hit a filesystem boundary
def find_mount_point(path):
    path = os.path.abspath(path)
    while not os.path.ismount(path):
        path = os.path.dirname(path)
    return path

print(find_mount_point("/mnt/usb/photos/2024"))
# /mnt/usb
```

---

## Path Metadata Functions

These functions query the filesystem for **information about** a path: size,
modification time, etc.

### `os.path.getsize(path)`

Returns file size in bytes. Raises `OSError` if the file doesn't exist or is
inaccessible.

```python
import os

def human_size(num_bytes):
    for unit in ["B", "KB", "MB", "GB", "TB"]:
        if num_bytes < 1024:
            return f"{num_bytes:.1f} {unit}"
        num_bytes /= 1024
    return f"{num_bytes:.1f} PB"

# Scan a folder and report the 5 largest files
folder = os.path.expanduser("~/Downloads")
files = []
for name in os.listdir(folder):
    full = os.path.join(folder, name)
    if os.path.isfile(full):
        files.append((os.path.getsize(full), full))

for size, path in sorted(files, reverse=True)[:5]:
    print(f"{human_size(size):>10}  {path}")
#   1.4 GB   /home/alice/Downloads/ubuntu-24.04.iso
# 412.3 MB   /home/alice/Downloads/dataset.zip
# ...
```

### `os.path.getmtime`, `os.path.getatime`, `os.path.getctime`

Return timestamps (Unix epoch floats) for **modification**, **access**, and
**creation/metadata-change** time, respectively. Wrap with
`datetime.fromtimestamp` to make them human-readable.

```python
import os
from datetime import datetime

def file_info(path):
    if not os.path.exists(path):
        return f"{path} does not exist"
    return (
        f"{path}\n"
        f"  size:  {os.path.getsize(path):,} bytes\n"
        f"  mtime: {datetime.fromtimestamp(os.path.getmtime(path))}\n"
        f"  atime: {datetime.fromtimestamp(os.path.getatime(path))}\n"
        f"  ctime: {datetime.fromtimestamp(os.path.getctime(path))}\n"
    )

# Find files modified in the last 24 hours
import time
cutoff = time.time() - 24 * 3600
recent = []
for dirpath, _, filenames in os.walk("."):
    for name in filenames:
        full = os.path.join(dirpath, name)
        if os.path.getmtime(full) > cutoff:
            recent.append(full)

print(f"{len(recent)} files modified in the last 24 hours:")
for f in recent[:10]:
    print(" -", f)
```

> **Note**: `os.getcwd()` lives in `os`, not `os.path`. Same for `os.listdir`,
> `os.walk`, `os.makedirs`, `os.stat`. `os.path` is strictly for path **string**
> manipulation and quick metadata queries — directory operations themselves
> live in the parent `os` module.

---

## Common Real-World Recipes

### Recipe 1 — Safely build a path under the user's home

```python
import os

def app_data_file(filename):
    """Return an absolute path to a file under ~/.myapp/, creating the folder."""
    base = os.path.join(os.path.expanduser("~"), ".myapp")
    os.makedirs(base, exist_ok=True)
    return os.path.join(base, filename)

state_db = app_data_file("state.db")
print(f"State DB at: {state_db}")
# State DB at: /home/alice/.myapp/state.db
```

### Recipe 2 — Find every `.py` file in a project

```python
import os

def collect_python_files(root):
    """Yield absolute paths to every .py file under `root`."""
    for dirpath, _, filenames in os.walk(root):
        # Skip hidden directories like .git, .venv, __pycache__
        if any(part.startswith(".") or part == "__pycache__"
               for part in dirpath.split(os.sep)):
            continue
        for name in filenames:
            if os.path.splitext(name)[1] == ".py":
                yield os.path.abspath(os.path.join(dirpath, name))

for py_file in collect_python_files("."):
    print(py_file)
```

### Recipe 3 — Compute the relative path between two directories

```python
import os

def relative_symlink_target(src, dst):
    """Compute what the symlink at `dst` should point to so it resolves to `src`."""
    src_abs = os.path.abspath(src)
    dst_dir = os.path.dirname(os.path.abspath(dst))
    return os.path.relpath(src_abs, dst_dir)

# Create a portable symlink that works no matter where the project is mounted
src = "./config/local.json"
dst = "./bin/local.json"
print("ln -s", relative_symlink_target(src, dst), dst)
# ln -s ../config/local.json bin/local.json
```

### Recipe 4 — Normalize and validate user-supplied paths

```python
import os

def safe_resolve(base_dir, user_path, must_exist=False):
    """
    Resolve `user_path` against `base_dir`, refusing path traversal escapes.
    Returns the absolute, normalized path on success.
    """
    base_abs = os.path.abspath(base_dir)
    full = os.path.normpath(os.path.join(base_abs, user_path))
    if not (full == base_abs or full.startswith(base_abs + os.sep)):
        raise PermissionError(f"Path '{user_path}' escapes base directory")
    if must_exist and not os.path.exists(full):
        raise FileNotFoundError(full)
    return full

# File-serving app: only serve files under /var/www
print(safe_resolve("/var/www", "images/logo.png"))
# /var/www/images/logo.png
print(safe_resolve("/var/www", "../../etc/passwd"))
# PermissionError: Path '../../etc/passwd' escapes base directory
```

---

## Cheatsheet

| Function | Description | Example Input | Example Output |
|---|---|---|---|
| `os.path.join(a, b, c)` | Join path components cross-platform | `join("/var", "log", "app.log")` | `/var/log/app.log` |
| `os.path.split(p)` | Split into `(head, tail)` | `split("/a/b/c.txt")` | `('/a/b', 'c.txt')` |
| `os.path.splitext(p)` | Split into `(root, ext)` | `splitext("a.tar.gz")` | `('a.tar', '.gz')` |
| `os.path.basename(p)` | Last path component | `basename("/a/b/c.txt")` | `'c.txt'` |
| `os.path.dirname(p)` | Everything except the last component | `dirname("/a/b/c.txt")` | `'/a/b'` |
| `os.path.abspath(p)` | Absolute, normalized path | `abspath("../x")` | `'/home/alice/x'` |
| `os.path.normpath(p)` | Collapse `.`/`..` and `//` | `normpath("/a/./b/../c")` | `'/a/c'` |
| `os.path.relpath(p, s)` | Path relative to `s` | `relpath("/a/b/c", "/a")` | `'b/c'` |
| `os.path.expanduser(p)` | Expand `~` to home dir | `expanduser("~/x")` | `'/home/alice/x'` |
| `os.path.commonpath(ps)` | Longest shared sub-path | `commonpath(['/a/b/c','/a/b/d'])` | `'/a/b'` |
| `os.path.exists(p)` | Does anything exist at `p`? | `exists("/etc/hosts")` | `True` |
| `os.path.isfile(p)` | Is it a regular file? | `isfile("/etc/hosts")` | `True` |
| `os.path.isdir(p)` | Is it a directory? | `isdir("/etc")` | `True` |
| `os.path.isabs(p)` | Is the path absolute? | `isabs("/etc")` | `True` |
| `os.path.islink(p)` | Is it a symlink? | `islink("/usr/local/bin/python")` | `True` |
| `os.path.ismount(p)` | Is it a mount point? | `ismount("/mnt/usb")` | `True` |
| `os.path.samefile(a, b)` | Do `a` and `b` point to the same inode? | `samefile("a", "link_to_a")` | `True` |
| `os.path.getsize(p)` | File size in bytes | `getsize("/etc/hosts")` | `238` |
| `os.path.getmtime(p)` | Last modification time (epoch float) | `getmtime("/etc/hosts")` | `1715000000.0` |
| `os.path.getatime(p)` | Last access time | `getatime("/etc/hosts")` | `1715100000.0` |
| `os.path.getctime(p)` | Creation / metadata-change time | `getctime("/etc/hosts")` | `1714800000.0` |

---

## Cross-Platform Notes & Gotchas

`os.path` exists precisely because path handling differs between operating
systems. Keep these rules in mind:

1. **Separator differs.** POSIX uses `/`; Windows uses `\`. Always use
   `os.path.join` or `os.sep` instead of hardcoding. Note that **Windows also
   accepts `/`** in most contexts, but the reverse is not true — backslashes
   on POSIX are valid filename characters, not separators.

2. **Drive letters and UNC paths.** On Windows, paths like `C:\Users\...` and
   `\\server\share\...` are absolute. `os.path.isabs` correctly recognizes both.
   On POSIX there are no drive letters — absolute paths always start with `/`.

3. **Case sensitivity.** macOS and Linux filesystems (ext4, APFS default) are
   **case-sensitive**; Windows NTFS is **case-insensitive**. Never rely on
   case to differentiate files — `README.md` and `readme.md` may collide on
   Windows but be two distinct files on Linux.

4. **Trailing slashes.** `os.path.join("/var/log/", "app.log")` produces
   `/var/log/app.log` correctly, but `os.path.split("/var/log/")` returns
   `('/var/log', '')`. If your code is sensitive to trailing slashes, run it
   through `os.path.normpath` first.

5. **Symlinks.** `os.path.exists` returns `False` for a **broken** symlink,
   but `os.path.islink` returns `True`. If you need to inspect a symlink
   itself, use `os.path.islink` before `os.path.exists`. To resolve to the
   real target, use `os.path.realpath`.

6. **Non-ASCII paths.** Python 3 handles Unicode filenames natively, but
   filesystem encoding varies by platform (UTF-8 on Linux/macOS, UTF-16 on
   Windows). If you pass bytes paths, decode them with
   `os.fsdecode` / encode with `os.fsencode`.

7. **Network and removable media.** Calls like `os.path.getsize` and
   `os.path.exists` will block if the path is on a slow or unmounted network
   share. Wrap them in `try/except OSError` for robustness.

8. **Path length limits.** Windows has a historical 260-character `MAX_PATH`
   limit (relaxed in recent versions but still encountered). Long paths may
   raise `FileNotFoundError` even when the file exists. Use the `\\?\` prefix
   or `os.path.abspath` to sidestep this in extreme cases.

---

## `os.path` vs `pathlib` — When to Use Which

Python 3.4+ introduced `pathlib`, an object-oriented alternative to `os.path`.
For new code, **`pathlib` is the recommended approach** — it's more readable,
chainable, and less error-prone. However, `os.path` remains essential for
legacy codebases, quick scripts, and situations where you're working with raw
path strings (e.g., from environment variables or config files).

Side-by-side equivalent operations:

| Operation | `os.path` | `pathlib` |
|---|---|---|
| Join paths | `os.path.join(a, b)` | `Path(a) / b` |
| Get filename | `os.path.basename(p)` | `Path(p).name` |
| Get parent dir | `os.path.dirname(p)` | `Path(p).parent` |
| Get extension | `os.path.splitext(p)[1]` | `Path(p).suffix` |
| Check exists | `os.path.exists(p)` | `Path(p).exists()` |
| Get size | `os.path.getsize(p)` | `Path(p).stat().st_size` |
| Expand `~` | `os.path.expanduser("~")` | `Path.home()` |

```python
# os.path style
import os
config = os.path.join(os.path.expanduser("~"), ".myapp", "config.json")
if os.path.exists(config):
    size = os.path.getsize(config)

# pathlib equivalent
from pathlib import Path
config = Path.home() / ".myapp" / "config.json"
if config.exists():
    size = config.stat().st_size
```

**Recommendation**: Learn `os.path` first — it appears in countless tutorials,
libraries, and existing codebases. Reach for `pathlib` when starting a new
project where you control the code style.

---

## License & Contributions

This README is released under the **MIT License** — feel free to copy, modify,
and distribute.

For the authoritative reference, always consult the official Python docs:
- [`os.path` documentation](https://docs.python.org/3/library/os.path.html)
- [`pathlib` documentation](https://docs.python.org/3/library/pathlib.html)
- [`os` module documentation](https://docs.python.org/3/library/os.html)

**Contributions welcome.** If you spot an error, have a clearer example, or
want to add a platform-specific gotcha, please open an issue or pull request.
