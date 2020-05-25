#!/usr/bin/python

import subprocess

class Args:
    num_frames=-1
    num_threads=1
    num_cubes=16
    overwrite=False

    def __init__(self, f=-1, t=1, c=16, o=False):
        self.num_frames=f
        self.num_threads=t
        self.num_cubes=c
        self.overwrite=o

    def build(self):
        a = [
            "./vulkan",
            "-num_cubes", str(self.num_cubes),
            "-num_threads", str(self.num_threads)
        ]
        if self.overwrite:
            a += ["-overwrite"]
        if self.num_frames>0:
            a += ["-num_frames", str(self.num_frames)]
        return a

subprocess.call(["make","-j","4"])
args = Args(50,1,250000,True)
for i in range(1,5):
    args.num_threads = i
    if i > 1:
        args.overwrite=False
    a=args.build()
    print(a)
    subprocess.call(a)
