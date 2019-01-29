#!/usr/bin/env python
#coding: utf-8

import sys
f = open(sys.argv[1])

header = """
#define X )*2+1
#define _ )*2
#define s ((((((((0
"""

footer = """
#undef s
#undef _
#undef X
"""

print(header)

print("char font[4096] = {")

for line in f.readlines():
    line = line.strip()
    if line.startswith("char"):
        print("// " + line)
    elif line.startswith(".") or line.startswith("*"):
        out = "s "
        for c in line:
            if c == ".":
                out += "_ "
            else:
                out += "X "
        out += ","
        print(out)

print("};")

print(footer)