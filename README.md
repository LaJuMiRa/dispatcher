<div align="center">

# Dispatcher

**A POSIX multi-process task dispatcher written in C — fork workers, distribute tasks via message queues, collect results.**

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](LICENSE) [![Language](https://img.shields.io/badge/language-C-blue.svg)](#requirements) [![Platform](https://img.shields.io/badge/platform-Linux%20%C2%B7%20macOS-lightgrey.svg)](#requirements) [![PRs welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)

</div>

---

## Table of Contents

- [Overview](#overview)
- [How it works](#how-it-works)
- [Requirements](#requirements)
- [Build](#build)
- [Usage](#usage)
- [Example output](#example-output)
- [Project structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

Dispatcher is a small systems-programming demo that shows how a parent process can fan work out to a configurable pool of child processes using POSIX message queues for IPC. The parent acts as the dispatcher: it creates a bounded task queue, forks *N* workers, enqueues *T* tasks, sends one termination sentinel per worker, and then waits for every worker to report its statistics before cleaning up.

---

## How it works

```
Dispatcher (parent)
│
├─ opens  /snek  (O_WRONLY)   — task queue
├─ opens  /comp  (O_RDONLY)   — completion queue
│
├─ fork() × num_workers
│     └─ Worker #i
│           ├─ opens /snek (O_RDONLY)
│           ├─ loops: mq_receive → sleep(effort)
│           ├─ exits on sentinel (value1 == 0)
│           └─ mq_send stats → /comp
│
├─ enqueues num_tasks messages  {task_id, effort}
├─ enqueues num_workers sentinels  {0, 0}
├─ drains /comp — logs per-worker stats
└─ wait() × num_workers → cleanup
```

Two POSIX message queues are used:

| Queue   | Direction              | Message type  | Purpose                              |
|---------|------------------------|---------------|--------------------------------------|
| `/snek` | dispatcher → workers   | `struct message` (`value1`, `value2`) | Task ID + effort (seconds). `value1 == 0` is the termination sentinel. |
| `/comp` | workers → dispatcher   | `comp_msg`    | Worker ID, PID, tasks processed, total effort. |

Each task carries a random *effort* value (1–10 seconds). Workers simulate work by calling `sleep(effort)`.

---

## Requirements

| Dependency | Notes |
|------------|-------|
| C99 compiler (`gcc` / `clang`) | |
| POSIX message queues (`<mqueue.h>`) | Kernel support required; standard on Linux. On macOS, install via Homebrew (`brew install mqueue` or use a Linux VM). |
| `make` (optional) | Any C build system works. |

---

## Build

```bash
# gcc
gcc -o dispatcher dispatcher.c -lrt

# clang
clang -o dispatcher dispatcher.c
```

---

## Usage

```
./dispatcher -w <num_workers> -t <num_tasks> -s <queue_size>
```

| Flag | Description |
|------|-------------|
| `-w` | Number of worker processes to fork |
| `-t` | Number of tasks to distribute |
| `-s` | Maximum messages the task queue can hold simultaneously |

**Example**

```bash
./dispatcher -w 3 -t 10 -s 5
```

Starts 3 worker processes, distributes 10 tasks with a queue that holds at most 5 messages at a time.

---

## Example output

```
[14:22:01] | Dispatcher | Starting 3 workers for 10 tasks with queue size 5
[14:22:01] | Worker  #1 | Started worker PID 12345
[14:22:01] | Worker  #2 | Started worker PID 12346
[14:22:01] | Worker  #3 | Started worker PID 12347
[14:22:01] | Dispatcher | Distributing tasks
[14:22:01] | Dispatcher | Queuing task #1 with effort 4
[14:22:01] | Dispatcher | Queuing task #2 with effort 7
...
[14:22:01] | Dispatcher | Sending termination task
[14:22:01] | Dispatcher | Waiting for workers to terminate
[14:22:01] | Worker  #1 | Received task with effort 4
[14:22:05] | Worker  #1 | Received termination task
...
[14:22:09] | Dispatcher | Worker #1 processed 4 tasks in 22 seconds
[14:22:09] | Dispatcher | Worker #1 with PID 12345 exited with status 0
```

---

## Project structure

```
dispatcher/
├── dispatcher.c      # entire implementation — single translation unit
├── CHANGELOG.md      # version history
├── CONTRIBUTING.md   # contribution guidelines
├── LICENSE           # GNU GPL v3
└── README.md
```

---

## Contributing

Bug reports, improvements, and suggestions are welcome. Please read [`CONTRIBUTING.md`](CONTRIBUTING.md) before opening a pull request.

---

## License

[GNU GPL v3](LICENSE) © 2026 Laurenz Rauscher

This program is free software: you can redistribute it and/or modify it under the terms of the **GNU General Public License v3.0**. It is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY. See the [LICENSE](LICENSE) file for the full text.
