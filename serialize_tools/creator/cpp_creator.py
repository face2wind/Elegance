#!/usr/bin/env python3

import os
import sys
import getopt
import xml.dom.minidom


class CppCreator(object):
    def __init__(self, file_name, xml_root, output_path):
        if not os.path.exists(output_path):
            print ("CppCreator create error")
            exit(1)
            
        self.xml_root = xml_root
        self.output_path = output_path
        self.file_name = file_name

    def GetCppRealType(self, type_str, subtype_str):
        real_type_str = type_str
        if type_str == "int8":
            real_type_str = "char"
        elif type_str == "uint8":
            real_type_str = "unsigned char"
        elif type_str == "int16":
            real_type_str = "short"
        elif type_str == "uint16":
            real_type_str = "unsigned short"
        elif type_str == "int32":
            real_type_str = "int"
        elif type_str == "uint32":
            real_type_str = "unsigned int"
        elif type_str == "int64":
            real_type_str = "long long"
        elif type_str == "uint64":
            real_type_str = "unsigned long long"
        elif type_str == "string":
            real_type_str = "std::string"
        elif type_str == "array":
            if subtype_str == "":
                print("subtype_str can not empty when type is array")
                exit(1)
            real_type_str = "std::vector<" + subtype_str + ">"
        return real_type_str
        

    def DoCreate(self):
        protocols = self.xml_root.getElementsByTagName("protocol")
        
        hpp_file_str = "#pragma once\n\n#include <string>\n#include <vector>\n#include <memory/serialize/serialize_base.hpp>\n\n"
        cpp_file_str = "#include \"" + self.file_name + ".hpp\"\n\n"
        
        for protocol in protocols:
            class_name = protocol.getAttribute("name")

            hpp_file_str += ("struct " + class_name + " : public SerializeBase\n{\n")
            
            attrs = protocol.getElementsByTagName("attr")
            for attr in attrs:
                subtype_str = ""
                if (attr.hasAttribute("subtype")):
                    subtype_str = self.GetCppRealType(attr.getAttribute("subtype"), "")
                attr_str = "  " + self.GetCppRealType(attr.getAttribute("type"), subtype_str) + " " + attr.getAttribute("name") + ";\n"
                hpp_file_str += attr_str
            hpp_file_str += "\nprotected:\n"
            hpp_file_str += "  virtual SerializeBase * Clone();\n"
            hpp_file_str += "  virtual const std::string & GetClassName() const;\n"
            hpp_file_str += "  virtual void Serialize(ByteArray &collector) const;\n"
            hpp_file_str += "  virtual void Unserialize(ByteArray &collector);\n"
            hpp_file_str += "};\n\n"

        hpp_file = open(self.output_path + "/" + self.file_name + ".hpp", "w")
        hpp_file.write(hpp_file_str)
        hpp_file.close()

        cpp_file = open(self.output_path + "/" + self.file_name + ".cpp", "w")
        cpp_file.write(cpp_file_str)
        cpp_file.close()
