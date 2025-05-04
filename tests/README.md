# Olympos Test Framework

This directory contains the integration test framework for Olympos. The framework uses 
`QEMU` to run tests in an isolated environment and captures serial output for verification.

### From the test directory:
```bash
cd test
make test                                   # Run all tests
make test_<file_name>                       # Run all tests from a file (e.g., test_printf)
make test_<file>_<test_name>                # Run specific test (e.g., test_printf_basic)
make list                                   # List all available tests
make help                                   # Show help
```

### From the project root:
```bash
python tests/run_tests.py                                   # Run all tests
python tests/run_tests.py <file_name>                       # Run specific test (e.g., test_printf_basic)
python tests/run_tests.py test_<file_name>                  # Run all tests from a file (e.g., test_printf)
python tests/run_tests.py --list                            # List all tests
python tests/run_tests.py --help                            # Show help
```
