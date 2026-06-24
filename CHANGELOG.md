# Changelog

All notable changes to this project are documented here. The format is based on
[Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres
to [Semantic Versioning](https://semver.org/).

## [1.0.0] – 2026-04-18

Initial release.

### Added
- Dispatcher parent process that forks a configurable number of worker child processes.
- Command-line argument parsing via `getopt` (`-w`, `-t`, `-s`).
- POSIX message queue `/snek` for distributing tasks from dispatcher to workers.
- POSIX message queue `/comp` for collecting per-worker completion statistics.
- Random task effort generation (1–10 seconds) using `rand()`.
- Graceful worker termination via sentinel message (`value1 == 0`).
- Per-worker statistics reporting: tasks processed, total effort, PID.
- Timestamped logging (`[HH:MM:SS]`) via variadic `log_msg()`.
- Proper queue cleanup (`mq_unlink`) on dispatcher exit.
- EINTR-safe send/receive loops for all message queue operations.

[1.0.0]: https://github.com/laurenzrauscher/dispatcher/releases/tag/v1.0.0
