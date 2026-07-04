# `tree` Command

The `tree` command is a highly useful command-line utility that displays the structure of directories and files in a hierarchical tree format. It is an excellent tool for documenting projects and quickly visualizing directory layouts.

---

## 1. Installation

The `tree` command is not installed by default on many operating systems. Use the following commands to install it:

### Linux (Ubuntu/Debian)
```bash
sudo apt install tree
```

### Windows
Windows comes with a simple built-in `tree` command, but for the full-featured version (similar to Linux), you can install it via [Scoop](https://scoop.sh/) or [Chocolatey](https://chocolatey.org/):
```bash
choco install tree
```

---

## 2. Basic Usage

To view the structure of the directory you are currently in, simply type:
```bash
tree
```

To view the structure of a specific path:
```bash
tree /path/to/folder
```

---

## 3. Most Useful Switches (Options)

You can customize the output of `tree` by using various flags:

### Control and Display
*   **`-L level`**: Limit the depth of the tree (e.g., show only 2 levels down).
    ```bash
    tree -L 2
    ```
*   **`-d`**: List **directories only** (hides files).
    ```bash
    tree -d
    ```
*   **`-a`**: List all files, including **hidden files** (those starting with a dot, e.g., `.git` or `.env`).
    ```bash
    tree -a
    ```

### Filtering Files
*   **`-P pattern`**: List only those files that match the wildcard pattern.
    ```bash
    tree -P "*.py"   # Shows only Python files
    ```
*   **`-I pattern`**: **Ignore** (exclude) specific folders or files from the output (useful for hiding heavy or unnecessary directories).
    ```bash
    tree -I "node_modules|.git|__pycache__"
    ```

### File Information
*   **`-h`**: Print the size of each file in **human-readable** format (KB, MB, GB).
    ```bash
    tree -h
    ```
*   **`-f`**: Print the **full path** prefix for each file.
    ```bash
    tree -f
    ```
*   **`-s`**: Print the size of each file in bytes.
    ```bash
    tree -s
    ```
*   **`-D`**: Print the date of the **last modification** for each file.
    ```bash
    tree -D
    ```

---

## 4. Real-World Examples

**Example 1: Mapping a coding project (without clutter)**
Visualize the project structure up to 3 levels deep, show hidden files, but exclude `node_modules` and `.git` folders:
```bash
tree -a -L 3 -I "node_modules|.git"
```

**Example 2: Finding folder sizes in the current directory**
Show only directories and their sizes:
```bash
tree -d -h
```

**Example 3: Saving output to a file for documentation**
Instead of printing to the terminal, save the tree structure to a file named `structure.txt`:
```bash
tree -a -I "node_modules" > structure.txt
# Alternatively, you can use the -o flag:
tree -a -I "node_modules" -o structure.txt
```

---

## 5. Web and Structured Output

The `tree` command can also generate machine-readable or web-ready formats:

*   **JSON Output:**
    ```bash
    tree -J > output.json
    ```

*   **HTML Output:**
    Generates a web file with clickable links for the directories:
    ```bash
    tree -H . > index.html
    ```
