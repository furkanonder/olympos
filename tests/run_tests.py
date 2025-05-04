import argparse
import os
import sys
from collections import defaultdict

# Add parent directory to path
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from test_framework import OlymposTestFramework
from test_printf import register_printf_tests
from test_serial import register_serial_tests


def list_tests(framework):
    test_files = defaultdict(list)

    for test in framework.tests:
        file_name, test_case = test["name"].split("_", 1)
        test_files[file_name].append(test_case)

    for file_name, test_cases in test_files.items():
        print(f"\ntest_{file_name}:")
        for test_case in test_cases:
            print(f"  test_{file_name}_{test_case}")


def filter_tests(framework, test_patterns):
    filtered = []

    for pattern in test_patterns:
        if not pattern.startswith("test_"):
            print(f"Warning: Test names should start with 'test_'. Ignoring '{pattern}'")
            continue
        # Remove 'test_' prefix
        name = pattern[5:]
        # File-level pattern (e.g., test_printf)
        if "_" not in name:
            filtered.extend(t for t in framework.tests if t["name"].startswith(f"{name}_"))
        # Specific test pattern
        else:
            filtered.extend(t for t in framework.tests if t["name"] == name)

    return filtered


def main():
    parser = argparse.ArgumentParser(description="Olympos Test Runner")
    parser.add_argument("tests", nargs="*", help="Tests to run")
    parser.add_argument("--list", action="store_true", help="List all available tests")
    args = parser.parse_args()

    framework = OlymposTestFramework()

    # Register all tests
    register_serial_tests(framework)
    register_printf_tests(framework)

    if args.list:
        list_tests(framework)
        sys.exit(0)
    if args.tests:
        framework.tests = filter_tests(framework, args.tests)
        if not framework.tests:
            print(f"No tests found matching: {args.tests}")
            sys.exit(1)

    if not framework.tests:
        print("No tests to run")
        sys.exit(1)

    success = framework.run_all_tests()
    framework.cleanup()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
