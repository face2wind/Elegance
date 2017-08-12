#!/usr/bin/env python3

import os
import sys
import getopt
import xml.dom.minidom

from serialize_creator import cpp_creator

#if not 'cpp_creator' in sys.modules:
#    b = __import__('cpp_creator')
#else:
#    eval('import cpp_creator')
#    b = eval('reload(cpp_creator)')

                    
version="0.0.1"

#print "script name :", sys.argv[0]
#for i in range(1, len(sys.argv)):
#    print "argument ", i, " = ", sys.argv[i]

def usage():
    print("./protocol_creator.py")
    print("  -i input_protocol_file")
    print("  -o out_put_path")
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

if ("" == input_file or "" == output_path):
    print("input or output is empty")
    #sys.exit()

print("input (", input_file, ")")
print("output (", output_path, ")")

dom = xml.dom.minidom.parse(input_file)
root = dom.documentElement

#print(root.nodeName)
#print(root.nodeValue)
#print(root.nodeType)
#print(root.ELEMENT_NODE)

file_name = os.path.basename(input_file)[0:-4]
creator = cpp_creator.CppCreator(file_name, root, output_path)
creator.DoCreate()
