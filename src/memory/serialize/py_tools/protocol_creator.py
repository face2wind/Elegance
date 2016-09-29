#!/usr/bin/env python3

import sys
import getopt

version="0.0.1"

#print "script name :", sys.argv[0]
#for i in range(1, len(sys.argv)):
#    print "argument ", i, " = ", sys.argv[i]

def usage():
    print("./protocol_creator.py")
    print("  -i input_protocol_file")
    print("  -o out_put_file")
    print("  -h show usage")

def showVersion():
    print("protocol_creator.py")
    print("  version : ", version)

opts, args = getopt.getopt(sys.argv[1:], "hi:o:", ["version","file="])
input_file=""
output_path=""

for op, value in opts:
    if op == "-i":
        input_file = value
    elif op == "-o":
        output_path = value
    elif op == "-h":
        usage()
        sys.exit()
    elif op == "--version":
        showVersion()
        sys.exit()

print("input (", input_file, ")")
print("output (", output_path, ")")
