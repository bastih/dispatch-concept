from jinja2 import Environment, FileSystemLoader
from datetime import datetime
import subprocess
import os

def time_call(call, times):
    compile_times = []
    for i in range(times):
        start = datetime.now()
        ret = subprocess.call(call)
        end = datetime.now()
        assert(ret == 0)
        compile_times.append((end-start).total_seconds())
    return compile_times

def average(compile_times):
    return sum(compile_times) / len(compile_times)

def main():
    loader = FileSystemLoader('')
    env = Environment(loader=loader)
    template = env.get_template('bench.cpp')
    print "num_cls", "binary_size", "compile_time_avg", "run_time_avg"
    for child_classes in [1,2,4,8] + range(10, 100, 10):
        with open("benchmark.cpp", "w") as f:
            f.writelines(template.render(nums=range(child_classes)))

        compile_times = time_call(["/home/vagrant/clang34/bin/clang++", "-std=c++11", "-O3", "-o", "bench", "benchmark.cpp"], 5)

        run_times = time_call(["./bench"], 5)


        binary_size = os.path.getsize("bench")
        print child_classes, binary_size, average(compile_times), average(run_times)


if __name__ == "__main__":
    main()
