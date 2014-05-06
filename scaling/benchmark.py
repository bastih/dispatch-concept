from jinja2 import Environment, FileSystemLoader
from datetime import datetime
from collections import namedtuple
from numpy import std
from itertools import product
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
            raise CompileException()
        compile_times.append((end-start).total_seconds())
    return compile_times

def out_time_call(call, times):
    compile_times = []
    for i in range(times):
        ret = float(subprocess.check_output(call)) / (1000 * 1000)
        compile_times.append(ret)
    return compile_times

def average(compile_times):
    return sum(compile_times) / len(compile_times)

def memory_consumption(call):
    ret = subprocess.call(["/usr/bin/time", "-v", "-o", "timeoutput"] + call)
    if(ret != 0):
        raise CompileException()
    for line in open("timeoutput").readlines():
        if line.strip().startswith("Maximum resident set size (kbytes):"):
            return int(line[len("Maximum resident set size (kbytes): "):].strip())

def massif_consumption(call):
    subprocess.call("valgrind --quiet --tool=massif --pages-as-heap=yes --massif-out-file=massif.out".split(" ") + call, stdout=open("/dev/null", "w"))
    return max(int(line[len("mem_heap_B="):]) for line in open("massif.out").readlines() if line.startswith("mem_heap_B="))

loader = FileSystemLoader('')
env = Environment(loader=loader)

Compiler = namedtuple("Compiler", ["name", "path"])
Flags = namedtuple("Flags", ["name", "flags"])
Mode = namedtuple("Mode", ["name", "flags"])
RUNS = 5

def run(template_n, mode, compiler, flags, child_classes, workload):
    template = env.get_template(template_n)
    with open("benchmark.cpp", "w") as f:
        f.writelines(template.render(nums=range(child_classes)))
    compile_cmd = [compiler.path] + flags.flags + mode.flags + workload.flags + [ "-std=c++11", "-ftemplate-depth-1500", "-o",  "bench", "benchmark.cpp" ]
    compile_times = time_call(compile_cmd, RUNS)
    subprocess.call(["/usr/bin/strip", "bench"])
    binary_size_stripped = os.path.getsize("bench")
    run_times = out_time_call(["./bench"], RUNS)
    compile_memory = memory_consumption(compile_cmd)
    runtime_memory = massif_consumption(["./bench"])
    print ",".join(str(s) for s in [template_n, mode.name, workload.name, compiler.name, flags.name, child_classes,
                    binary_size_stripped, average(compile_times), std(compile_times), compile_memory,
                    average(run_times), std(run_times), runtime_memory])

def main():
    print ",".join(["template", "mode", "workload", "compiler", "flags", "num_cls", "binary_size", "compile_time_avg", "compile_time_std", "compile_time_memory", "run_time_avg", "run_time_std", "run_time_memory"])
    for template, mode, compiler, child_classes, flags, extra in product(['bench_new.cpp'],
                                                                         [Mode._make(("dispatch", ["-DWITH_DISPATCH"])), Mode._make(("no_dispatch", []))],
                                                                         [Compiler._make(("g++", "g++")), Compiler._make(("clang++", "/home/hillig/clangilize/build/bin/clang++"))],
                                                                         [1, 2, 4, 8, 50,], #100, 200],
                                                                         #[Flags._make(("debug", ["-O0"])), Flags._make(("release", ["-O3", "-flto"]))],
                                                                         [Flags._make(("aggressive_release", ["-O3", "-march=native", "-fomit-frame-pointer", "-s"]))],
                                                                         [Flags._make(("significant work", ["-DWITH_SIGNIFICANT_WORK"])), Flags._make(("single virtual call", []))][:1]):
        try:
            run(template, mode, compiler, flags, child_classes, extra)
        except CompileException:
            pass

if __name__ == "__main__":
    main()
