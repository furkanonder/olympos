.PHONY: all test clean help list

all: test

test:
	@echo "Running all Olympos tests..."
	@cd .. && python tests/run_tests.py

test_%:
	@echo "Running test: test_$*"
	@cd .. && python tests/run_tests.py test_$*

list:
	@echo "Listing all available tests..."
	@cd .. && python tests/run_tests.py --list

help:
	@echo "Available targets:"
	@echo "  test                           - Run all tests"
	@echo "  test_<file>                    - Run all tests from a file (e.g., test_printf)"
	@echo "  test_<file>_<test_name>        - Run specific test (e.g., test_printf_basic)"
	@echo "  list                           - List all available tests"
	@echo "  help                           - Show this help message"
