# Folder structure

- `src/`     - application source (.c files)
- `include/` - application headers (.h files)
- `tests/`   - CUnit test suite (test_*.c, test_helpers.*, test_suites.h)
- `data/`    - runtime-generated user data (.dat files) - gitignored,
               created automatically by `make` via the `data-dir` target

## Build

    make            # builds ./financeapp
    make run        # builds and runs it
    make unit-test   # builds and runs the CUnit suite (needs libcunit1-dev)
    make benchmark  # builds and runs the standalone performance/capacity report
    make clean      # removes binaries and data/*.dat
