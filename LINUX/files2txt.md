# Print Files With Their Contents

This command prints every file in a directory along with its contents.

## Command

```bash
find /path/to/dir -maxdepth 1 -type f -exec sh -c 'for f do echo "$f:"; cat "$f"; echo; done' sh {} +
```

## What it does

- `find /path/to/dir` searches for files in the target directory
- `-maxdepth 1` limits the search to that directory only
- `-type f` selects regular files only
- `-exec sh -c '...'` runs a small shell script on the matched files
- `echo "$f:"` prints the file name
- `cat "$f"` prints the file content
- `echo` adds an empty line between files

## Example Output

```text
file1.txt:
Hello world

notes.md:
This is a note
```

## Recursive Version

To include files inside subdirectories too, remove `-maxdepth 1`:

```bash
find /path/to/dir -type f -exec sh -c 'for f do echo "$f:"; cat "$f"; echo; done' sh {} +
```

## Variants & Advanced Filters

### 3. Filter by Specific File Extension
Only prints files with a specific extension (e.g., only `.cpp` files).

```bash
find /path/to/dir -type f -name "*.cpp" -exec sh -c 'for f do echo "$f:"; cat "$f"; echo; done' sh {} +
```

### 4. Filter by Multiple Extensions
Only prints files matching specific extensions (e.g., C++ source and header files: `.cpp`, `.hpp`, `.h`).

```bash
find /path/to/dir -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) -exec sh -c 'for f do echo "$f:"; cat "$f"; echo; done' sh {} +
```

### 5. Exclude Specific Directories (e.g., `build` or `.git`)
Useful when you want to scan a project directory but skip build artifacts or version control files.

```bash
find /path/to/dir -path "*/build" -prune -o -path "*/.git" -prune -o -type f -exec sh -c 'for f do echo "$f:"; cat "$f"; echo; done' sh {} +
```
## Exporting the Output

If you want to save the console output directly to a file (e.g., `workspace_summary.txt`) instead of printing it to the terminal, append `> output.txt` to the end of any command:

```bash
find /path/to/dir -type f -name "*.txt" -exec sh -c 'for f do echo "$f:"; cat "$f"; echo; done' sh {} + > workspace_summary.txt
```


## Notes

- This works well for text files
*   **Binary Files:** These commands use `cat` to print contents. Running them on binary files (like `.png`, `.o`, or executables) will output garbled characters and might mess up your terminal formatting. Use the extension filters above to target only text/code files.
*   **Large Files:** Running this on massive datasets or directories with node_modules/build folders can generate huge outputs. Use the exclude pattern where necessary.
- File paths are printed before each file's content
