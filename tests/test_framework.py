import os
import shutil
import subprocess
from collections import Counter
from typing import Any, Dict


class OlymposTestFramework:
    def __init__(self):
        self.tests = []
        self.results = []
        self.verbose = False
        # Determine if we're in the test directory or root
        self.in_test_dir = os.path.basename(os.getcwd()) == "test"
        self.root_dir = ".." if self.in_test_dir else "."

    def set_verbose(self, verbose):
        self.verbose = verbose

    def get_path(self, path):
        if self.in_test_dir:
            return os.path.join(self.root_dir, path)
        return path

    def register_test(self, name: str, test_code: str, expected_output: str):
        self.tests.append({"name": name, "code": test_code, "expected": expected_output})

    def backup_kernel(self):
        kernel_path = self.get_path("kernel/init/kernel.c")
        backup_path = f"{kernel_path}.bak"
        if os.path.exists(backup_path):
            os.remove(backup_path)
        shutil.copy2(kernel_path, backup_path)

    def restore_kernel(self):
        kernel_path = self.get_path("kernel/init/kernel.c")
        backup_path = f"{kernel_path}.bak"
        if os.path.exists(backup_path):
            shutil.move(backup_path, kernel_path)

    def create_test_kernel(self, test_code: str) -> bool:
        try:
            kernel_path = self.get_path("kernel/init/kernel.c")
            with open(kernel_path, "w") as f:
                f.write(test_code)

            build_script = self.get_path("build-test.sh")
            with open(build_script, "w") as f:
                f.write("#!/bin/sh\n")
                f.write("# Temporary build script for tests\n")
                f.write(". ./config.sh\n")
                f.write(". ./headers.sh\n\n")
                f.write("# Add TEST define to CFLAGS\n")
                f.write('export CFLAGS="$CFLAGS -DTEST"\n')
                f.write('export CPPFLAGS="$CPPFLAGS -DTEST"\n\n')
                f.write("# Build the projects\n")
                f.write("for PROJECT in $PROJECTS; do\n")
                f.write('  (cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)\n')
                f.write("done\n")
            os.chmod(build_script, 0o755)

            if self.verbose:
                print("Building kernel...")
                build_cmd = f"cd {self.root_dir} && ./build-test.sh"
            else:
                build_cmd = f"cd {self.root_dir} && ./build-test.sh > /dev/null 2>&1"
            if os.system(build_cmd) != 0:
                return False

            return True
        except Exception as e:
            print(f"Error creating test kernel: {e}")
            return False

    def create_test_iso(self) -> bool:
        try:
            # Create directory structure
            isodir_test_path = self.get_path("isodir-test")
            boot_path = os.path.join(isodir_test_path, "boot")
            grub_path = os.path.join(boot_path, "grub")
            os.makedirs(grub_path, exist_ok=True)

            # Copy kernel from sysroot
            sysroot_path = self.get_path("sysroot")
            kernel_source = os.path.join(sysroot_path, "boot", "olympos.kernel")
            kernel_dest = os.path.join(boot_path, "olympos.kernel")
            shutil.copy2(kernel_source, kernel_dest)

            grub_config = """
            set timeout=0
            set default=0
            menuentry "olympos-test" {
                multiboot /boot/olympos.kernel
                boot
            }
            """
            grub_cfg_path = os.path.join(grub_path, "grub.cfg")
            with open(grub_cfg_path, "w") as f:
                f.write(grub_config)

            if self.verbose:
                print("Creating test ISO...")
                iso_cmd = f"cd {self.root_dir} && grub-mkrescue -o olympos-test.iso isodir-test"
            else:
                iso_cmd = f"cd {self.root_dir} && grub-mkrescue -o olympos-test.iso isodir-test > /dev/null 2>&1"
            return os.system(iso_cmd) == 0
        except Exception as e:
            print(f"Error creating test ISO: {e}")
            return False

    def run_qemu(self) -> tuple[int, str]:
        iso_path = self.get_path("olympos-test.iso")
        qemu_cmd = [
            "qemu-system-i386",
            "-cdrom",
            iso_path,
            "-serial",
            "stdio",
            "-display",
            "none",
            "-device",
            "isa-debug-exit,iobase=0xf4,iosize=0x04",
            "-no-reboot",
        ]
        result = subprocess.run(qemu_cmd, capture_output=True)
        output = result.stdout.decode("utf-8", errors="ignore")
        return result.returncode, output

    def run_test(self, test: Dict[str, Any]) -> bool:
        test_name = test["name"]
        self.backup_kernel()
        output = ""
        success = False

        try:
            if not self.create_test_kernel(test["code"]):
                self.results.append({"name": test_name, "passed": False, "output": "Failed to build test kernel"})
                return False
            if not self.create_test_iso():
                self.results.append({"name": test_name, "passed": False, "output": "Failed to create test ISO"})
                return False
            return_code, output = self.run_qemu()
            success = return_code == 1 and test["expected"] in output

            if self.verbose:
                print(f"Test Output:\n{output}")
                if test["expected"] not in output:
                    print(f"Expected output:\n'{test['expected']}'")
        except Exception as e:
            print(f"run_test failed - {e}")
        finally:
            self.restore_kernel()

        self.results.append({"name": test_name, "passed": success, "output": output})
        return success

    def run_all_tests(self):
        total_test_count = len(self.tests)
        print(f"=== Running {total_test_count} Tests ===")

        for i, test in enumerate(self.tests, 1):
            result = self.run_test(test)
            result_message = "PASSED" if result else "FAILED"
            print(f"[{i}/{total_test_count}] {test['name']}: {result_message}")

        print("\n=== Test Summary ===")
        status_counter = Counter(r["passed"] for r in self.results)
        passed, failed = status_counter[True], status_counter[False]
        print(f"Passed: {passed}/{total_test_count} ({passed/total_test_count*100:.1f}%)")

        if failed:
            print("\nFailed Tests:")
            for result in self.results:
                if not result["passed"]:
                    print(f"  - {result['name']}")

        return passed == total_test_count

    def cleanup(self):
        files_to_remove = [self.get_path("olympos-test.iso"), self.get_path("build-test.sh")]
        dirs_to_remove = [self.get_path("isodir-test"), self.get_path("sysroot")]

        for f in files_to_remove:
            if os.path.exists(f):
                os.remove(f)
        for d in dirs_to_remove:
            if os.path.exists(d):
                shutil.rmtree(d)

        if self.verbose:
            print("Cleaning projects...")
            clean_cmd = (
                f"cd {self.root_dir}"
                f"&& . ./config.sh && for PROJECT in $PROJECTS; do (cd $PROJECT && $MAKE clean); done"
            )
        else:
            clean_cmd = (
                f"cd {self.root_dir} && . ./config.sh "
                f"&& for PROJECT in $PROJECTS; do (cd $PROJECT && $MAKE clean); done > /dev/null 2>&1"
            )

        os.system(clean_cmd)
