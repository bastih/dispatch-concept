from jinja2 import Environment, FileSystemLoader
from datetime import datetime
from collections import namedtuple
from numpy import std
import subprocess
import os

class CompileException(Exception):
    pass

def time_call(call, times):
    compile_times = []
    for i in range(times):
        start = datetime.now()
        ret = subprocess.call(call)
        end = datetime.now()
        if(ret != 0):
            print time
            raise CompileException()
        compile_times.append((end-start).total_seconds())
    return compile_times

def average(compile_times):
    return sum(compile_times) / len(compile_times)

def memory_consumption(call):
    ret = subprocess.call(["/usr/bin/time", "-v", "-o", "timeoutput"] + call)
    if(ret != 0):
        print "memory"
        raise CompileException()
    for line in open("timeoutput").readlines():
        if line.strip().startswith("Maximum resident set size (kbytes):"):
            return int(line[len("Maximum resident set size (kbytes): "):].strip())

loader = FileSystemLoader('')
env = Environment(loader=loader)
template = env.get_template('bench.cpp')

Compiler = namedtuple("Compiler", ["name", "path"])
Flags = namedtuple("Flags", ["name", "flags"])

RUNS = 1

def run(compiler, flags, child_classes):
    with open("benchmark.cpp", "w") as f:
        f.writelines(template.render(nums=range(child_classes)))
    compile_cmd = [compiler.path] + flags.flags + [ "-std=c++11", "-o", "bench", "benchmark.cpp" ]
    compile_times = time_call(compile_cmd, RUNS)
    run_times = time_call(["./bench"], RUNS)
    binary_size = os.path.getsize("bench")
    compile_memory = memory_consumption(compile_cmd)
    runtime_memory = memory_consumption(["./bench"])
    print ",".join(str(s) for s in [compiler.name, flags.name, child_classes,
                    binary_size, average(compile_times), std(compile_times), compile_memory,
                    average(run_times), std(run_times), runtime_memory])

def main():
    print ",".join(["compiler", "flags", "num_cls", "binary_size", "compile_time_avg", "compile_time_std", "compile_time_memory", "run_time_avg", "run_time_std", "run_time_memory"])
    for compiler in [Compiler._make(("g++", "g++")), Compiler._make(("clang++", "/home/vagrant/clang34/bin/clang++"))]:
        for child_classes in [1,2,4,8]: #+ range(10, 100, 10):
            for flags in [Flags._make(("debug", ["-O0", "-ggdb"])),
                          Flags._make(("release", ["-O3"])),
                          Flags._make(("release with debug", ["-O3", "-ggdb"])),
                          Flags._make(("release with flto", ["-O3", "-flto"])),
                          Flags._make(("release with size optimization", ["-Os", "-finline-functions", "-frename-registers", "-march=native", "-fomit-frame-pointer", "-s"]))
            ]:

                try:
                    run(compiler, flags, child_classes)
                except CompileException:
                    pass

if __name__ == "__main__":
    main()
