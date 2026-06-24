# Contributing

Thanks for your interest! Contributions are welcome — bug fixes, portability
improvements, new features, or better documentation.

## Setup

```bash
# Clone the repository
git clone https://github.com/LaJuMiRa/dispatcher.git
cd dispatcher

# Build
gcc -o dispatcher dispatcher.c -lrt   # Linux
clang -o dispatcher dispatcher.c       # macOS
```

## Guidelines

- **Keep it standard.** The code targets C99 and POSIX. Avoid compiler-specific
  extensions or platform-only APIs unless they are guarded by feature-test macros.
- **Single translation unit.** For now the entire implementation lives in
  `dispatcher.c`. Larger additions (e.g. a dedicated logging module) may warrant
  splitting — open an issue first to discuss.
- **EINTR safety.** All `mq_send` / `mq_receive` calls must retry on `EINTR`.
  Do not remove the retry loops. If you do so, please explain why and how and which purpose it has.
- **No silent errors.** Every syscall return value must be checked. Use
  `perror` / `fprintf(stderr, ...)` for failures and exit with `EXIT_FAILURE`.
- **Match the existing style.** 4-space indentation, braces on the same line for
  control flow, one blank line between top-level definitions.

## Pull requests

1. Fork the repository and create a branch (`feature/...` or `fix/...`).
2. Make your change. For non-trivial changes, add a short explanation in the PR
   description (what, why, how to test).
3. For user-visible changes, add an entry to [`CHANGELOG.md`](CHANGELOG.md) under
   an `[Unreleased]` section.
4. Run the binary manually and verify the output looks correct before opening the PR.

## Reporting bugs

Open an issue with:
- The command you ran (flags and values).
- The full terminal output.
- Your OS and compiler version (`gcc --version` / `clang --version`).

By contributing, you agree that your contributions are licensed under the
project's [GNU GPL v3](LICENSE).
