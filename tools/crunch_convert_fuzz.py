#!/usr/bin/env python3
"""Runs `jcrunchtool convert-fuzz` on each .ods, .fods, or .csv file in a list, each in a child process, and sums up the sweep for crashes and hangs of the readers of
OpenDocument spreadsheets and of delimited text (the same sweep as crunch_xls_fuzz.py makes for the old binary format).

    python3 tools/crunch_convert_fuzz.py build/jcrunchtool keys.txt out-dir [jobs] [copies=200] [seed=1]

keys.txt has one path to a file on each line (paths may have spaces). Each runs under `systemd-run --user --scope -p MemoryMax=4G` with 120 seconds. out-dir/fuzz.tsv has a line for
each file; the summary is printed: how many copies were made, how many opened, how many files crashed or took more than 120 seconds.
"""
import importlib.util
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))


def main():
    if len(sys.argv) < 4:
        print(__doc__)
        return 2
    spec = importlib.util.spec_from_file_location("crunch_xls_fuzz", os.path.join(HERE, "crunch_xls_fuzz.py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)

    def run_one(tool, path, copies, seed):
        command = ["systemd-run", "--user", "--scope", "-p", "MemoryMax=4G", "-p", "MemorySwapMax=0", "-q", "timeout", "-k", "5", str(module.LIMIT_SECONDS), tool, "convert-fuzz", path,
                   str(copies), str(seed), "100"]
        env = dict(os.environ, JP_NO_TELEMETRY="1", QT_QPA_PLATFORM="offscreen")
        with tempfile.TemporaryFile() as errors:
            child = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=errors, env=env)
            output = child.stdout.read().decode("utf-8", "replace")
            _, status, usage = os.wait4(child.pid, 0)
            errors.seek(0)
            message = errors.read(300).decode("utf-8", "replace").strip().replace("\n", " | ")
        code = os.waitstatus_to_exitcode(status)
        if code < 0:
            code = 128 - code
        return code, output.strip(), message, usage.ru_maxrss / 1024.0

    module.run_one = run_one
    sys.argv[0] = "crunch_xls_fuzz.py"
    return module.main()


if __name__ == "__main__":
    sys.exit(main())
