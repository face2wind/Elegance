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
                print("GetCppRealType : subtype_str can not empty when type is array")
                exit(1)
            real_type_str = "std::vector<" + self.GetCppRealType(subtype_str, "") + ">"
        return real_type_str

    def GetSerializeCode(self, type_str, subtype_str, attr_name):
        code_str = ""
        if type_str == "int8":
            code_str += ("  collector.WriteInt8(" + attr_name + ");\n")
        elif type_str == "uint8":
            code_str += ("  collector.WriteUint8(" + attr_name + ");\n")
        elif type_str == "int16":
            code_str += ("  collector.WriteInt16(" + attr_name + ");\n")
        elif type_str == "uint16":
            code_str += ("  collector.WriteUint16(" + attr_name + ");\n")
        elif type_str == "int32":
            code_str += ("  collector.WriteInt32(" + attr_name + ");\n")
        elif type_str == "uint32":
            code_str += ("  collector.WriteUint32(" + attr_name + ");\n")
        elif type_str == "int64":
            code_str += ("  collector.WriteInt64(" + attr_name + ");\n")
        elif type_str == "uint64":
            code_str += ("  collector.WriteUint64(" + attr_name + ");\n")
        elif type_str == "string":
            code_str += ("  collector.WriteString(" + attr_name + ");\n")
        elif type_str == "array":
            if subtype_str == "":
                print("GetSerializeCode : subtype_str can not empty when type is array")
                exit(1)
            code_str += ("  collector.WriteUint16((unsigned short)" + attr_name + ".size());\n")
            code_str += "  for (auto array_item : " + attr_name + ")\n  {\n  "
            sub_serialize_code = self.GetSerializeCode(subtype_str, "", "array_item")
            if sub_serialize_code == "":
                sub_serialize_code = "  array_item.Serialize(collector);\n"
            code_str += sub_serialize_code
            code_str += "  }\n"
        return code_str

    def GetUnserializeCode(self, type_str, subtype_str, attr_name):
        code_str = ""
        if type_str == "int8":
            code_str += ("  " + attr_name + " = collector.ReadInt8();\n")
        elif type_str == "uint8":
            code_str += ("  " + attr_name + " = collector.ReadUint8();\n")
        elif type_str == "int16":
            code_str += ("  " + attr_name + " = collector.ReadInt16();\n")
        elif type_str == "uint16":
            code_str += ("  " + attr_name + " = collector.ReadUint16();\n")
        elif type_str == "int32":
            code_str += ("  " + attr_name + " = collector.ReadInt32();\n")
        elif type_str == "uint32":
            code_str += ("  " + attr_name + " = collector.ReadUint32();\n")
        elif type_str == "int64":
            code_str += ("  " + attr_name + " = collector.ReadInt64();\n")
        elif type_str == "uint64":
            code_str += ("  " + attr_name + " = collector.ReadUint64();\n")
        elif type_str == "string":
            code_str += ("  " + attr_name + " = collector.ReadString();\n")
        elif type_str == "array":
            if subtype_str == "":
                print("GetSerializeCode : subtype_str can not empty when type is array")
                exit(1)
            code_str += ("  {\n    int array_size = collector.ReadUint16();\n    " + self.GetCppRealType(subtype_str, "") + " tmp_attr_value;\n")
            code_str += "    for (int index = 0; index < array_size; ++ index)\n    {\n    "
            sub_serialize_code = self.GetSerializeCode(subtype_str, "", "tmp_attr_value")
            if sub_serialize_code == "":
                sub_serialize_code = "  tmp_attr_value.Unserialize(collector);\n"
            code_str += sub_serialize_code
            code_str += ("      " + attr_name + ".push_back(tmp_attr_value);\n")
            code_str += "    }\n  }\n"
        return code_str

    def DoCreate(self):
        protocols = self.xml_root.getElementsByTagName("protocol")
        
        hpp_file_str = "#pragma once\n\n#include <string>\n#include <vector>\n#include <memory/serialize/serialize_base.hpp>\n\nusing face2wind::SerializeBase;\nusing face2wind::SerializeDescribe;\nusing face2wind::ByteArray;\n\nnamespace Protocol {\n\n"
        cpp_file_header_str = "#include \"" + self.file_name + ".hpp\"\n\nnamespace Protocol {\n\n"
        describe_hpp_str = ""
        describe_cpp_str = ""
        cpp_file_str = ""
        
        for protocol in protocols:
            class_name = protocol.getAttribute("name")

            hpp_file_str += ("class " + class_name + " : public SerializeBase\n{\npublic:\n")

            cpp_serialize_code = ""
            cpp_unserialize_code = ""
            
            attrs = protocol.getElementsByTagName("attr")
            for attr in attrs:
                type_name = attr.getAttribute("type")
                attr_name = attr.getAttribute("name")
                subtype_name = ""
                real_type_name = ""
                if (attr.hasAttribute("subtype")):
                    subtype_name = attr.getAttribute("subtype")
                real_type_name = self.GetCppRealType(type_name, subtype_name)
                hpp_file_str += ("  " + real_type_name + " " + attr_name + ";\n")
                cpp_serialize_code += self.GetSerializeCode(type_name, subtype_name, attr_name)
                cpp_unserialize_code += self.GetUnserializeCode(type_name, subtype_name, attr_name)

            hpp_file_str += "\n  virtual void Serialize(ByteArray &collector) const;\n"
            hpp_file_str += "  virtual void Unserialize(ByteArray &collector);\n"
            hpp_file_str += "  virtual const std::string GetTypeName() const { return \"" + class_name + "\"; }\n"
            hpp_file_str += "};\n\n"

            describe_class_name = "__" + class_name + "Describe__";
            describe_hpp_str += ("class " + describe_class_name + " : public SerializeDescribe\n{\npublic:\n  " + describe_class_name + "() { name_to_object_map_[\"" + class_name + "\"] = this; }\n  virtual ~" + describe_class_name + "() {}\n")
            describe_hpp_str += "\nprotected:\n  virtual SerializeBase * CreateSerialize() const { return new " + class_name + "(); }\n};\n\n"
            describe_cpp_str += (describe_class_name + " " + "for_describe_register_to_" + describe_class_name.lower() + ";\n")
            cpp_file_str += ("void " + class_name + "::Serialize(ByteArray &collector) const\n")
            cpp_file_str += ("{\n" + cpp_serialize_code + "}\n\n")
            cpp_file_str += ("void " + class_name + "::Unserialize(ByteArray &collector)\n")
            cpp_file_str += ("{\n" + cpp_unserialize_code + "}\n\n")
            
        cpp_file_str += "}\n\n"
        describe_hpp_str += "\n\n"
        describe_cpp_str += "\n\n"

        hpp_file = open(self.output_path + "/" + self.file_name + ".hpp", "w")
        hpp_file.write(hpp_file_str + describe_hpp_str + "}\n\n")
        hpp_file.close()

        cpp_file = open(self.output_path + "/" + self.file_name + ".cpp", "w")
        cpp_file.write(cpp_file_header_str + describe_cpp_str + cpp_file_str)
        cpp_file.close()
