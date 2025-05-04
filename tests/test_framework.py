import os
import shutil
import subprocess
from typing import Any, Dict


class OlymposTestFramework:
    def __init__(self):
        self.tests = []
        self.results = []
        # Determine if we're in the test directory or root
        self.in_test_dir = os.path.basename(os.getcwd()) == "test"
        self.root_dir = ".." if self.in_test_dir else "."

    def get_path(self, path):
        """Get the correct path based on the current directory"""
        if self.in_test_dir:
            return os.path.join(self.root_dir, path)
        return path

    def register_test(self, name: str, test_code: str, expected_output: str):
        """Register a test case"""
        self.tests.append({"name": name, "code": test_code, "expected": expected_output})

    def backup_kernel(self):
        """Backup original kernel"""
        kernel_path = self.get_path("kernel/init/kernel.c")
        backup_path = f"{kernel_path}.bak"
        if os.path.exists(backup_path):
            os.remove(backup_path)
        shutil.copy2(kernel_path, backup_path)

    def restore_kernel(self):
        """Restore original kernel"""
        kernel_path = self.get_path("kernel/init/kernel.c")
        backup_path = f"{kernel_path}.bak"
        if os.path.exists(backup_path):
            shutil.move(backup_path, kernel_path)

    def create_test_kernel(self, test_code: str) -> bool:
        """Create test kernel with given code"""
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

            build_cmd = f"cd {self.root_dir} && ./build-test.sh"
            if os.system(build_cmd) != 0:
                print("Build failed!")
                return False

            iso_cmd = f"cd {self.root_dir} && ./iso.sh"
            if os.system(iso_cmd) != 0:
                print("ISO creation failed!")
                return False

            return True
        except Exception as e:
            print(f"Error creating test kernel: {e}")
            return False

    def setup_test_iso(self):
        grub_config = """
        set timeout=0
        set default=0
        menuentry "olympos-test" {
            multiboot /boot/olympos.kernel
            boot
        }
        """
        isodir_path = self.get_path("isodir")
        isodir_test_path = self.get_path("isodir-test")

        if os.path.exists(isodir_test_path):
            shutil.rmtree(isodir_test_path)
        shutil.copytree(isodir_path, isodir_test_path)

        grub_cfg_path = os.path.join(isodir_test_path, "boot/grub/grub.cfg")
        with open(grub_cfg_path, "w") as f:
            f.write(grub_config)

        iso_cmd = f"cd {self.root_dir} && grub-mkrescue -o olympos-test.iso isodir-test"
        os.system(iso_cmd)

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
        print(f"\nRunning test: {test['name']}")
        print("-" * 40)

        self.backup_kernel()
        try:
            if not self.create_test_kernel(test["code"]):
                return False

            self.setup_test_iso()
            return_code, output = self.run_qemu()
            expected_exit_code = 1
            success = return_code == expected_exit_code and test["expected"] in output

            print(f"Output: {output[:500]}...")
            if success:
                print(f"✓ Test {test['name']} PASSED")
            else:
                print(f"✗ Test {test['name']} FAILED")
                if test["expected"] not in output:
                    print(f"  Expected output '{test['expected']}' not found")

            self.results.append({"name": test["name"], "passed": success, "output": output, "returncode": return_code})
            return success
        finally:
            self.restore_kernel()

    def run_all_tests(self):
        print("=== Running Olympos Tests ===\n")

        for test in self.tests:
            self.run_test(test)

        print("\n=== Test Summary ===")
        passed = sum(1 for r in self.results if r["passed"])
        total = len(self.results)

        print(f"Passed: {passed}/{total}")
        for result in self.results:
            status = "✓ PASS" if result["passed"] else "✗ FAIL"
            print(f"{status}: {result['name']}")

        return passed == total

    def cleanup(self):
        files_to_remove = [self.get_path("olympos-test.iso"), self.get_path("build-test.sh")]
        dirs_to_remove = [self.get_path("isodir-test")]

        for f in files_to_remove:
            if os.path.exists(f):
                os.remove(f)
        for d in dirs_to_remove:
            if os.path.exists(d):
                shutil.rmtree(d)
