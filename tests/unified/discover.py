#!/usr/bin/env python3

import argparse
import pathlib
import sys

ARCHS = {
    "mips32": {
        "name": "MIPS 32",
    },
    "riscv32": {
        "name": "RISC-V 32"
    },
}

TESTS_ROOT = pathlib.Path("src/tests")

class TestCase:
    all_ = []

    @staticmethod
    def make(arch, name):
        res = TestCase(arch, name)
        TestCase.all_.append(res)
        print(f"Found test {name} for {arch}", file=sys.stderr)
        return res

    def get_all():
        return TestCase.all_

    def __init__(self, arch, name):
        self.arch = arch
        self.name = name
        self.msim_conf = None
        self.bootloader_asm = None
        self.bootloader_lds = None
        self.guest_expected = None
        self.host_expected = None

    def get_arch(self):
        return self.arch

    def get_name(self):
        return self.name

    def get_target_filename(self):
        return self.name.replace("/", "__").replace("-", "_") + f"__{self.arch}"

    def get_msim_conf(self):
        return self.msim_conf

    def get_bootloader_asm(self):
        return self.bootloader_asm

    def get_bootloader_ldscript(self):
        return self.bootloader_lds

    def get_guest_expected(self):
        return self.guest_expected

    def get_host_expected(self):
        return self.host_expected

    def set_msim_conf(self, path):
        self.msim_conf = path

    def set_bootloader(self, asm, lds):
        self.bootloader_asm = asm
        self.bootloader_lds = lds

    def set_guest_expected(self, path):
        self.guest_expected = path

    def set_host_expected(self, path):
        self.host_expected = path


def discover_test_dirs():
    for base_path in TESTS_ROOT.glob("*/*"):
        yield {
            'path': base_path,
            'name': f"{base_path.parent.name}/{base_path.name}"
        }

def find_nearest_file(start, filename, fallback_path=None):
    # TODO: anchor this better
    iters = 5
    it = start.resolve()
    while True:
        iters = iters - 1
        if iters == 0:
            if fallback_path is not None:
                return fallback_path
            else:
                raise Exception(f"No {filename} found in {start} or above.")
        actual_path = it.joinpath(filename)
        if actual_path.exists():
            return actual_path
        it = it.parent

def find_one_of(start, *args):
    for f in args:
        if f is None:
            return None
        path = start.joinpath(f)
        if path.exists():
            return path
    raise Exception(f"None of {args} exists in {start}")

def discover_expected_outputs(test, base_path, arch):
    test.set_guest_expected(find_one_of(
            base_path,
            f"guest.{arch}.expected",
            "guest.expected"
    ))
    test.set_host_expected(find_one_of(
            base_path,
            f"host.{arch}.expected",
            "host.expected"
    ))


def print_makefile(tests, output):
    build_phony = " ".join(["build_" + t.get_target_filename() for t in tests])
    print("# Generated file. Do not edit, do not commit", file=output)
    print(f"\n.PHONY: all clean {build_phony}\n\n", file=output)
    print(f"all: {build_phony}\n", file=output)
    print("include toolchain.mk\n\n", file=output)
    print(f"-include local.mk\n", file=output)

    cleanable_files = []

    for test in tests:
        target_dir = test.get_target_filename()
        top_target = "build_" + target_dir
        image_dir = f"images/{target_dir}"
        make_arch = test.get_arch().upper()

        subtargets = []
        def subtarget(target, deps, command, is_versioned=True):
            def fix_dep_path(path):
                if isinstance(path, str):
                    if path.startswith("./"):
                        return f"{image_dir}/{path[2:]}"
                return str(path)
            target_path = f"{image_dir}/{target}"
            subtargets.append({
                    "target": target_path,
                    "deps": " ".join([fix_dep_path(i) for i in deps]),
                    "command": command,
            })
            if not is_versioned:
                cleanable_files.append(target_path)

        subtarget(
                "msim.conf",
                [test.get_msim_conf()],
                "cat < $< > $@"
        )
        subtarget(
                "guest.expected",
                [test.get_guest_expected()],
                "cat < $< > $@"
        )
        subtarget(
                "host.expected",
                [test.get_host_expected()],
                "cat < $< > $@"
        )
        subtarget(
                "boot.o",
                [test.get_bootloader_asm()],
                f"$({make_arch}_AS) $({make_arch}_ASFLAGS) -c -o $@ $<",
                False
        )
        ldscript = test.get_bootloader_ldscript()
        subtarget(
                "boot.raw",
                ["./boot.o", ldscript],
                f"$({make_arch}_LD) $({make_arch}_LDFLAGS) -T {ldscript} -o $@ $<",
                False
        )
        subtarget(
                "boot.bin",
                ["./boot.raw"],
                f"$({make_arch}_OBJCOPY) -O binary $< $@"
        )

        subtarget_deps = " ".join([i["target"] for i in subtargets])
        print(f"{top_target}: {subtarget_deps}\n", file=output)
        print(f"{image_dir}:\n\tmkdir -p {image_dir}\n", file=output)
        for i in subtargets:
            print(f"{i['target']}: {i['deps']} | {image_dir}", file=output)
            print(f"\t{i['command']}\n", file=output)

        print(file=output)

    print("\nclean:\n\trm -f " + " ".join(cleanable_files), file=output)


def print_bats(tests, output):
    build_phony = " ".join(["build_" + t.get_target_filename() for t in tests])
    print("#!/usr/bin/env bats\n", file=output)
    print("# Generated file. Do not edit but commit\n", file=output)
    print("load \"../system/common\"\n\n", file=output)

    for test in tests:
        target_dir = test.get_target_filename()
        image_dir = f"images/{target_dir}"
        test_arch = ARCHS[test.get_arch()]['name']
        test_name = test.get_name()

        print(f"@test \"{test_arch}: {test_name}\" {{", file=output)
        print(f"    msim_run_code \"{image_dir}\"", file=output)
        print("}\n", file=output)

def main():
    args = argparse.ArgumentParser(description='Discover MSIM system tests')
    args.add_argument('--makefile',
        default=None,
        dest='makefile',
        help='Where to place the makefile code for rebuilding the tests.'
    )
    args.add_argument('--bats',
        default=None,
        dest='bats',
        help='Where to place the BATS launcher for the tests.'
    )
    config = args.parse_args()

    for base in discover_test_dirs():
        if base['path'].joinpath("kernel.c").exists():
            # TODO
            continue
        for arch in ARCHS.keys():
            boot_file = base['path'].joinpath(f"boot.{arch}.S")
            if not boot_file.exists():
                continue
            test = TestCase.make(arch, base['name'])
            test.set_bootloader(
                    boot_file,
                    find_nearest_file(base['path'], f"boot.{arch}.lds")
            )
            test.set_msim_conf(find_nearest_file(
                    base['path'],
                    f"msim.{arch}.conf",
                    TESTS_ROOT.joinpath(f"msim.boot.{arch}.conf")
            ))
            test.set_guest_expected(find_one_of(
                    base['path'],
                    f"guest.{arch}.expected",
                    "guest.expected"
            ))
            test.set_host_expected(find_one_of(
                    base['path'],
                    f"host.{arch}.expected",
                    "host.expected"
            ))

    if config.makefile:
        with open(config.makefile, "wt") as f:
            print_makefile(TestCase.get_all(), f)
    if config.bats:
        with open(config.bats, "wt") as f:
            print_bats(TestCase.get_all(), f)

if __name__ == '__main__':
    main()
