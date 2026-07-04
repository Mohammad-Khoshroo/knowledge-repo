# Complete Guide to Working with Git Submodules on GitHub

> A step-by-step tutorial in Persian on working with Git submodules — from scratch to handling common errors and understanding alternatives.

---

## Table of Contents

1. [Introduction: What is a submodule?](#1-introduction-what-is-a-submodule)
2. [Prerequisites](#2-prerequisites)
3. [Core Concepts and Terminology](#3-core-concepts-and-terminology)
4. [Adding a Submodule](#4-adding-a-submodule)
5. [Cloning a Repository That Contains Submodules](#5-cloning-a-repository-that-contains-submodules)
6. [Checking Submodule Status](#6-checking-submodule-status)
7. [Editing, Committing, and Pushing Inside a Submodule](#7-editing-committing-and-pushing-inside-a-submodule)
8. [Updating a Submodule from the Parent Repository](#8-updating-a-submodule-from-the-parent-repository)
9. [Working with Branches and Tags in a Submodule](#9-working-with-branches-and-tags-in-a-submodule)
10. [Removing a Submodule](#10-removing-a-submodule)
11. [Moving and Changing a Submodule Path](#11-moving-and-changing-a-submodule-path)
12. [Common Errors and Troubleshooting](#12-common-errors-and-troubleshooting)
13. [Alternatives: Comparing with Git Subtree](#13-alternatives-comparing-with-git-subtree)
14. [Summary and Best Practices](#14-summary-and-best-practices)

---

## 1. Introduction: What is a submodule?

When working on a project, you may sometimes need to use code from another repository inside your own project. For example, imagine you are building a website and want to use a ready-made CSS library or a Bootstrap template that lives in a separate repository. One option is to simply copy its files into your project. However, that approach has two major downsides: first, when the library repository gets updated, you have to manually copy everything again; second, the Git history of the two repositories gets mixed together, making it harder to track changes.

**Git Submodule** was created to solve exactly this problem. In simple terms, a submodule is a Git repository embedded inside another Git repository, while still keeping its own independent history. In other words, the parent repository only stores a **reference** to a specific commit of the child repository, not the actual files themselves. This means you can pin your project to a specific version of a library and upgrade it whenever you want, without merging their histories together.

### When should you use a Submodule?

- When you use a shared library or module across multiple projects and want to manage its version independently in each project.
- When you use code from another team or organization and do not want to fork or copy it.
- When you have split a large project into several smaller repositories but still want a **super-project** that ties them together.

### When should you **not** use a Submodule?

- When your team has only a shallow understanding of Git, because submodules have a learning curve and mistakes can be confusing.
- When the child code changes constantly and you need live synchronization (in that case, a monorepo may be a better fit).
- When you only want to copy a few files from another repository and modify them inside your own project (in that case, simple copying may be easier).

---

## 2. Prerequisites

Before getting started, make sure you have the following:

1. **Git installed** — version 2.18 or later is recommended (newer versions have better HTTPS and SSH support). To check your version:

   ```bash
   git --version
   ```

2. A **GitHub account** and access to the repositories you want to add as submodules (public or private).
3. **Basic familiarity with Git**: you know what `clone`, `commit`, `push`, and `branch` do. If not, first read [this official guide](https://docs.github.com/en/get-started/getting-started-with-git).
4. **SSH or HTTPS configured for GitHub**: if the repository is private, setting up SSH keys is usually better so you are not prompted for your password every time.

> 💡 **Tip:** If you are new to Git, I strongly recommend understanding the concepts of `commit` and `working directory` before continuing, because submodules are built on top of those ideas.

---

## 3. Core Concepts and Terminology

To make the rest of this guide easier to follow, let’s review a few key terms:

| Term | Meaning |
|---|---|
| **Super-project (parent/main repository)** | The repository that contains the submodule. For example, your main project. |
| **Submodule (child repository)** | A repository that is included inside the super-project as a nested dependency. |
| **`.gitmodules`** | A file in the root of the super-project that stores information about all submodules (their URLs and local paths). This file must be committed. |
| **Detached HEAD** | A state where the submodule is checked out at a specific commit rather than being on a branch. This is the default behavior for submodules. |
| **Pinning** | Locking the submodule to a specific commit so the super-project always sees that exact version until you explicitly update it. |

### Internal Structure

When you add a submodule, Git creates two things:

1. A text file called `.gitmodules` at the root of the project.
2. A folder for the submodule (for example, `libs/my-lib/`) that contains a hidden `.git` file — not a directory — pointing to `.git/modules/...` inside the super-project.

In addition, the super-project stores a full copy of the submodule’s Git data inside `.git/modules/`. This means the submodule is a complete and independent repository; it just happens to live physically inside the super-project folder.

---

## 4. Adding a Submodule

Suppose you have a main project called `my-app` and want to add a library named `awesome-lib` from GitHub as a submodule under the `libs/` directory.

### Step 1: Go to your project folder

```bash
cd path/to/my-app
```

### Step 2: Add the submodule

```bash
git submodule add https://github.com/user/awesome-lib.git libs/awesome-lib
```

This command does several things at once:

1. Clones the `awesome-lib` repository into `libs/awesome-lib`.
2. Creates the `.gitmodules` file (or updates it if it already exists).
3. Adds the relevant entries to the staging area automatically.

### Step 3: Check the status

```bash
git status
```

The output will look something like this:

```text
Changes to be committed:
  (use "git rm --cached <file>..." to unstage)
        new file:   .gitmodules
        new file:   libs/awesome-lib
```

Notice that `libs/awesome-lib` is tracked as a **tree entry** (a reference to a commit), not as the files inside it. That is exactly the main advantage of submodules: Git keeps its history separate.

### Step 4: Commit

```bash
git commit -m "feat: add awesome-lib as submodule"
```

### Step 5: Push

```bash
git push origin main
```

That’s it. Your project now contains a submodule. Anyone who clones this repository later will need to perform one extra step to fetch the submodule contents as well, which we will cover in the next section.

> ⚠️ **Warning:** If you add the submodule using an HTTPS URL, all team members can clone it (assuming the repository is public). If you use SSH (`git@github.com:user/awesome-lib.git`), all team members must have valid SSH keys configured. In organizational environments, HTTPS is often simpler.

---

## 5. Cloning a Repository That Contains Submodules

When someone else — or even you on another machine — clones the repository, the **submodule directories remain empty by default**. This is one of the first common points of confusion.

### Method 1: Clone with `--recurse-submodules` (recommended)

```bash
git clone --recurse-submodules https://github.com/user/my-app.git
```

This command clones the super-project and then automatically initializes and updates all submodules.

### Method 2: Clone normally, then initialize/update

If you forgot to use `--recurse-submodules`, or you already cloned the repository earlier:

```bash
git clone https://github.com/user/my-app.git
cd my-app

# Step 1: read .gitmodules and create the local config
git submodule init

# Step 2: fetch the contents of each submodule
git submodule update
```

### Shortcut: do both at once

```bash
git submodule update --init --recursive
```

The `--recursive` flag is useful when submodules themselves contain submodules (nested dependencies).

> 💡 **Practical tip:** Many teams add a `setup.sh` script to the repository that runs this command so new team members do not have to remember it.

---

## 6. Checking Submodule Status

To see which commit each submodule points to and whether anything has changed:

```bash
git submodule status
```

Example output:

```text
+e3b9c2a4f5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0 libs/awesome-lib (v1.2.0)
 1a2b3c4d5e6f7a8b9c0d1e2f3a4b5c6d7e8f9a0b libs/another-lib (heads/main)
```

- A leading `+` means the submodule is checked out at a different commit than the one expected by the super-project.
- A leading `-` means the submodule has not been initialized yet.
- A leading `U` means there is a conflict.

### Viewing changes inside a submodule

```bash
cd libs/awesome-lib
git status
git log --oneline -5
```

You can work inside a submodule exactly like in any normal Git repository.

---

## 7. Editing, Committing, and Pushing Inside a Submodule

One of the most important things to remember: **a submodule is a complete Git repository**. So if you want to commit changes inside it, you must first commit inside the submodule, and then tell the super-project to point to that new commit.

### Step 1: Make changes inside the submodule

```bash
cd libs/awesome-lib

# edit a file
echo "patch" >> some-file.txt

git add some-file.txt
git commit -m "fix: small patch in awesome-lib"
```

### Step 2: Push the changes to the submodule repository

```bash
git push origin main
```

> ⚠️ **Important note:** If you do not have push access to the submodule repository (for example, it is a third-party repository), you must first fork it and update the submodule URL to your fork. Otherwise, pushing will fail with a permission error.

### Step 3: Return to the super-project and update the pointer

```bash
cd ../..
git status
```

The output will show that `libs/awesome-lib` has changed:

```text
modified:   libs/awesome-lib (new commits)
```

Now commit that change in the super-project as well:

```bash
git add libs/awesome-lib
git commit -m "chore: bump awesome-lib to latest commit"
git push origin main
```

### The order is very important

If you commit inside the submodule but do not update the super-project, the next person who clones the repository will still see the old version. Always follow this order:

1. Commit inside the submodule
2. Push the submodule
3. Commit in the super-project to record the new pointer
4. Push the super-project

---

## 8. Updating a Submodule from the Parent Repository

When the submodule repository (for example, `awesome-lib`) gets updated on GitHub, your super-project does **not** automatically pick that up. You need to update it manually.

### Method 1: Update all submodules at once

```bash
git submodule update --remote
```

This command fetches the latest commit from the default branch of each submodule (usually `main`) and checks it out in the working directory. However, the change is not yet recorded in the super-project, so you still need to commit it:

```bash
git add libs/awesome-lib
git commit -m "chore: update awesome-lib to latest upstream"
git push origin main
```

### Method 2: Update only one specific submodule

```bash
git submodule update --remote libs/awesome-lib
```

### Method 3: Track a specific branch instead of the default

If you want the submodule to follow a different branch, such as `develop`:

```bash
git config -f .gitmodules submodule.libs/awesome-lib.branch develop
git submodule update --remote
```

This changes `.gitmodules`, so you must commit that file as well if you want the rest of the team to use the same branch.

> 💡 **Tip:** After `update --remote`, the submodule is usually in a **detached HEAD** state. If you want to stay on a branch and make changes, you should manually check out a branch first, as explained in the next section.

---

## 9. Working with Branches and Tags in a Submodule

### Checking out a specific branch inside the submodule

```bash
cd libs/awesome-lib
git checkout main
```

Now you can edit and commit normally because you are on a branch rather than in detached HEAD.

### Pinning a submodule to a specific tag

Suppose `awesome-lib` has released version `v2.0.0`, and you want your project to stay exactly on that version:

```bash
cd libs/awesome-lib
git fetch --tags
git checkout v2.0.0
cd ../..
git add libs/awesome-lib
git commit -m "chore: pin awesome-lib to v2.0.0"
```

From now on, anyone who clones the super-project will get exactly version `v2.0.0` of `awesome-lib` — even if newer releases exist — until you explicitly upgrade it.

### Pinning to a specific commit hash

```bash
cd libs/awesome-lib
git checkout 1a2b3c4d5e6f7a8b9c0d
cd ../..
git add libs/awesome-lib
git commit -m "chore: pin awesome-lib to specific commit"
```

This is useful when a newer version contains a bug and you want to roll back to the last stable revision.

---

## 10. Removing a Submodule

Removing a submodule in Git is a multi-step process, and unfortunately there is no simple one-command `git submodule remove` solution in older versions. Here is the full process:

### Step 1: Deinitialize the submodule

```bash
git submodule deinit -f libs/awesome-lib
```

This removes the local configuration entries from `.git/config`.

### Step 2: Remove it from the tree

```bash
git rm -f libs/awesome-lib
```

### Step 3: Delete the leftover Git data in `.git/modules`

```bash
rm -rf .git/modules/libs/awesome-lib
```

> ⚠️ Make sure you do this step. Otherwise, if you later add a submodule with the same name, Git may reuse the old metadata and cause strange errors.

### Step 4: Commit the removal

```bash
git commit -m "chore: remove awesome-lib submodule"
```

### Step 5: If `.gitmodules` is now empty

If this was the only submodule, the `.gitmodules` file may now be empty. In that case, it is better to remove it too:

```bash
git rm .gitmodules
git commit -m "chore: remove .gitmodules"
```

---

## 11. Moving and Changing a Submodule Path

### Changing the remote URL of a submodule

Sometimes a repository is moved — for example, from one GitHub organization to another. To update its URL:

```bash
git submodule set-url libs/awesome-lib https://github.com/new-org/awesome-lib.git
```

This command is available in Git 2.18+. In older versions, you would edit `.gitmodules` manually and then run:

```bash
git submodule sync
git submodule update --init --recursive
```

### Moving a submodule to a new path

Git does not have a straightforward built-in command for physically moving a submodule. The recommended approach is:

1. Remove the submodule as described in [Section 10](#10-removing-a-submodule).
2. Add it again at the new path:

   ```bash
   git submodule add https://github.com/user/awesome-lib.git new/path/awesome-lib
   ```

3. Commit the change.

---

## 12. Common Errors and Troubleshooting

### Problem 1: `fatal: 'libs/awesome-lib' already exists in the index`

**Cause:** The submodule was previously added but not removed correctly.

**Solution:**

```bash
git rm --cached libs/awesome-lib
rm -rf .git/modules/libs/awesome-lib
```

Then run `git submodule add` again.

### Problem 2: The submodule is in **detached HEAD** state

When you run `git submodule update`, Git checks out a specific commit instead of a branch. If you commit while in this state, your commits may be hard to find later.

**Solution:** Switch back to a branch first, then cherry-pick any commits you made in detached mode if necessary:

```bash
cd libs/awesome-lib
git log --oneline -3   # find the lost commit if there is one
git checkout main
git cherry-pick <commit-hash>   # if you committed while detached
```

### Problem 3: `error: Your local changes to the following files would be overwritten by checkout`

**Cause:** You have uncommitted changes inside the submodule, and Git wants to move it to a different commit.

**Solution:** Either commit or stash your changes first:

```bash
cd libs/awesome-lib
git stash
# or
git add . && git commit -m "wip"
```

Then run the update command again.

### Problem 4: After pulling the super-project, the submodule folder is empty

**Cause:** You forgot to run `git submodule update` after pulling.

**Solution:**

```bash
git submodule update --init --recursive
```

To avoid this problem in the future, you can enable the following setting so Git automatically recurses into submodules after each pull:

```bash
git config --global submodule.recurse true
```

### Problem 5: Conflict in a submodule during merge

Sometimes when merging two branches in the super-project, Git cannot decide which submodule commit to keep.

**Solution:**

```bash
# 1. Resolve the conflict in the super-project by choosing one side
git checkout --theirs libs/awesome-lib   # or --ours
git add libs/awesome-lib

# 2. Then inside the submodule, check out the actual desired commit
cd libs/awesome-lib
git checkout <commit-hash>
cd ../..
git add libs/awesome-lib
git commit
```

---

## 13. Alternatives: Comparing with Git Subtree

Submodule is not the only way to use code from another repository inside your project. A common alternative is `Git Subtree`.

### Git Subtree

`subtree` copies the code of another repository directly into your project, while optionally preserving its history. Its main advantage is that users of your project do not need any special commands — a normal clone is enough.

**Adding a subtree:**

```bash
git subtree add --prefix=libs/awesome-lib https://github.com/user/awesome-lib.git main --squash
```

**Updating a subtree:**

```bash
git subtree pull --prefix=libs/awesome-lib https://github.com/user/awesome-lib.git main --squash
```

### Quick comparison

| Criterion | Submodule | Subtree |
|---|---|---|
| **Transparency for end users** | Poor — they need to know about `--recurse-submodules` | Excellent — normal clone is enough |
| **Repository size** | Small — only stores a pointer | Larger — code is copied into the repo |
| **Independent history** | Yes — completely separate | Can be preserved or squashed |
| **Easy upstream updates** | Yes, with `update --remote` | Possible, but may cause conflicts |
| **Good for modifying child code** | Excellent | Harder (pushing changes back is more complex) |
| **Best suited for** | Shared libraries, independent modules | One-time imports with local customization |

### When should you choose each one?

- **Choose Submodule if:** you want to preserve independent history, want easy upgrades from upstream, and your team is comfortable with Git.
- **Choose Subtree if:** you want a simpler user experience, expect to heavily modify the child code locally, or want to keep a public repository easy for end users to clone.

Other alternatives such as a **monorepo** (everything in one repository), **Git X-Modules**, or tools like **Google’s Repo** can be useful for larger organizational projects, but are usually unnecessary for medium-sized projects.

---

## 14. Summary and Best Practices

### Flashcard summary

- **Add:** `git submodule add <url> <path>`
- **Clone with submodules:** `git clone --recurse-submodules <url>`
- **Initialize after a normal clone:** `git submodule update --init --recursive`
- **Update from upstream:** `git submodule update --remote`
- **Status:** `git submodule status`
- **Remove:** `git submodule deinit -f <path>` + `git rm -f <path>` + `rm -rf .git/modules/<path>`

### Best Practices

1. **Always commit `.gitmodules`.** This file is essential. Without it, team members will not know where to fetch the submodule from.

2. **Pin to a specific tag or commit, not a branch.** This makes your builds reproducible. If you want the latest version, update manually on a schedule (for example, monthly), test it, and then commit the new pointer.

3. **Create a setup script.** For example, a `scripts/setup.sh` file with:

   ```bash
   #!/bin/bash
   git submodule update --init --recursive
   ```

   This lets new team members run one command and have everything ready.

4. **In CI/CD, always enable submodule checkout.** For example, in GitHub Actions:

   ```yaml
   steps:
     - uses: actions/checkout@v4
       with:
         submodules: recursive
   ```

5. **Never commit inside a submodule while in detached HEAD.** Always run `git checkout main` (or the appropriate branch) first.

6. **Respect the push order:** first the submodule, then the super-project. If you do it the other way around, builds may fail because the submodule commit referenced by the super-project is not yet publicly available.

7. **Write clear documentation.** In your project README, explicitly state that the project uses submodules and include the required setup command.

8. **Train the team.** Submodules can be confusing for beginners. A short internal tutorial can prevent hours of debugging later.

### When should you reconsider using Submodule?

If you notice these signs, it may be time to rethink your approach:

- Team members constantly complain about submodules and their builds keep breaking.
- The number of submodules has grown beyond roughly 5–10 and dependencies are becoming too complex.
- In practice, the submodules have many local changes that are never sent upstream — in that case, copying or subtree may be better.
- The project has reached a point where a monorepo offers better management benefits.

---

### Further Resources

- [Official Git documentation: Submodules](https://git-scm.com/book/en/v2/Git-Tools-Submodules)
- [GitHub Docs: Working with submodules](https://docs.github.com/en/repositories/working-with-files/managing-files/working-with-submodules)
- [Atlassian Tutorial: Git Submodules](https://www.atlassian.com/git/tutorials/git-subtree)

---

> This guide was written for beginners. If any part is unclear or you have a specific scenario that is not covered, open an issue in the repository so it can be improved.
