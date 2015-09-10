#include "byte_array.hpp"

#include <iostream>

#include <string>
#include <sys/types.h>
#include <string.h> //strerror()
#include <stdio.h>
#include <errno.h>

namespace face2wind {

  signed char ByteArray::ReadSignedInt8()
  {
    if(0 == _bytes.size())
      return 0;
    unsigned char tmp = _bytes[0];
    _bytes.pop_front();
    if(showDebugMsg)
      std::cout<<"ReadSignedInt8() = "<<int(tmp)<<"\n";
    return tmp;
  }
  
  unsigned char ByteArray::ReadUnsignedInt8()
  {
    if(0 == _bytes.size())
      return 0;
    unsigned char tmp = _bytes[0];
    _bytes.pop_front();
    if(showDebugMsg)
      std::cout<<"ReadUnsignedInt8() = "<<int(tmp)<<"\n";
    return tmp;
  }

  short ByteArray::ReadSignedInt16()
  {
    unsigned short value = ReadUnsignedInt16();
    short realValue = TransformType<short>(&value);
    if(showDebugMsg)
      std::cout<<"ReadSignedInt16() = "<<realValue<<"\n";
    return realValue;
  }

  unsigned short ByteArray::ReadUnsignedInt16()
  {
    unsigned short value = 0;
    unsigned char left = ReadUnsignedInt8();
    unsigned char right = ReadUnsignedInt8();
    if(BYTE_BIG_ENDIAN == _endian){
      value = (left<<8) + right;
    }else{
      value = left + (right<<8);
    }
    if(showDebugMsg)
      std::cout<<"ReadUnsignedInt16() = "<<value<<"\n";
    return value;
  }

  int ByteArray::ReadSignedInt32()
  {
    unsigned int value = ReadUnsignedInt32();
    long realValue = TransformType<long>(&value);
    if(showDebugMsg)
      std::cout<<"ReadSignedInt32() = "<<realValue<<"\n";
    return realValue;
  }

  unsigned int ByteArray::ReadUnsignedInt32()
  {
    unsigned int value = 0;
    unsigned int left = ReadUnsignedInt16();
    unsigned int right = ReadUnsignedInt16();
    if(BYTE_BIG_ENDIAN == _endian){
      value = (left<<16) + right;
    }else{
      value = left + (right<<16);
      //std::cout<<"======== left:"<<left<<", right<<16:"<<(right<<16)<<"====\n";
    }
    if(showDebugMsg)
      std::cout<<"ReadUnsignedInt32() = "<<value<<"\n";
    return value;
  }

  float ByteArray::ReadFloat()
  {
    unsigned long value = ReadUnsignedInt32();
    return TransformType<float>(&value);
  }

  double ByteArray::ReadDouble()
  {
    unsigned long long value = ReadUnsignedInt64();
    double realValue = TransformType<double>(&value);
    return realValue;
  }

  long long ByteArray::ReadSignedInt64()
  {
    unsigned long long value = ReadUnsignedInt64();
    long long realValue = TransformType<long long>(&value);
    return realValue;
  }

  unsigned long long ByteArray::ReadUnsignedInt64()
  {
    unsigned long long value = 0;
    long rawLeft = ReadUnsignedInt32();
    unsigned long left = TransformType<unsigned long>(&rawLeft);
    long rawRight = ReadUnsignedInt32();
    unsigned long right = TransformType<unsigned long>(&rawRight);
    if(BYTE_BIG_ENDIAN == _endian){
      value = left*4294967296 + right;
    }else{
      value = left + right*4294967296;
    }
    return value;
  }

  std::string ByteArray::ReadAS3String()
  {
    std::string str;
    if(2 > _bytes.size()) // not enough length bytes
      return str;
    unsigned int len = 0;;
    if(BYTE_BIG_ENDIAN == _endian){
      len = int(_bytes[0])<<8;
      len += int(_bytes[1]);
    }
    else if(BYTE_LITTLE_ENDIAN == _endian){
      len = int(_bytes[1])<<8;
      len += int(_bytes[0]);
    }
    if( (2+len) > _bytes.size() ) // not enough length string
      return str;

    _bytes.pop_front();
    _bytes.pop_front();
    while(len){
      --len;
      str += _bytes[0];
      _bytes.pop_front();
    }
    return str;
  }

  void *ByteArray::ReadObject(unsigned int size)
  {
    if(size > _bytes.size())
      return NULL;
    if(0 == size) // 0 means use all bytes
      size = _bytes.size();
    
    char *data = new char[size];
    for(unsigned int i = 0; i < size; ++i){
      data[i] = _bytes[0];
      _bytes.pop_front();
    }
    return static_cast<void *>(data);
  }

  void ByteArray::WriteSignedInt8(const signed char &value)
  {
    unsigned char realValue = TransformType<unsigned char>(&value);
    //std::cout<<"write char("<<int(realValue)<<std::endl;
    _bytes.push_back(realValue);
  }

  void ByteArray::WriteUnsignedInt8(const unsigned char &value)
  {
    _bytes.push_back(value);
  }

  void ByteArray::WriteSignedInt16(const short &value)
  {
    if(showDebugMsg)
      std::cout<<"WriteSignedInt16("<<value<<")\n";
    unsigned short realValue = TransformType<unsigned short>(&value);
    WriteUnsignedInt16(realValue);
  }

  void ByteArray::WriteUnsignedInt16(const unsigned short &value)
  {
    if(showDebugMsg)
      std::cout<<"WriteUnsignedInt16("<<value<<")\n";
    if(BYTE_BIG_ENDIAN == _endian){
      WriteUnsignedInt8(value>>8);
      WriteUnsignedInt8(value%256);
    }
    else if(BYTE_LITTLE_ENDIAN == _endian){
      WriteUnsignedInt8(value%256);
      WriteUnsignedInt8(value>>8);
    }
  }
  
  void ByteArray::WriteSignedInt32(const int &value)
  {
    if(showDebugMsg)
      std::cout<<"WriteSignedInt32("<<value<<")\n";
    unsigned int realValue = TransformType<unsigned int>(&value);
    WriteUnsignedInt32(realValue);
  }

  void ByteArray::WriteUnsignedInt32(const unsigned int &value)
  {
    if(showDebugMsg)
      std::cout<<"WriteUnsignedInt32("<<value<<")\n";
    if(BYTE_BIG_ENDIAN == _endian){
      WriteUnsignedInt16(value>>16);
      WriteUnsignedInt16(value%65536);
    }else if(BYTE_LITTLE_ENDIAN == _endian){
      WriteUnsignedInt16(value%65536);
      WriteUnsignedInt16(value>>16);
    }
  }

  void ByteArray::WriteFloat(const float &value)
  {
    unsigned long realValue = TransformType<unsigned long>(&value);
    WriteUnsignedInt32(realValue);
  }

  void ByteArray::WriteSignedInt64(const long long &value)
  {
    unsigned long long realValue = TransformType<unsigned long long>(&value);
    WriteUnsignedInt64(realValue);
  } 

  void ByteArray::WriteUnsignedInt64(const unsigned long long &value)
  {
    unsigned long long int32 = 0x100000000;
    //std::cout<<"write double("<<value<<","<<realValue<<")\n";
    if(BYTE_BIG_ENDIAN == _endian){
      WriteUnsignedInt32(value/int32);
      WriteUnsignedInt32(value-(value/int32)*int32);
    }else if(BYTE_LITTLE_ENDIAN == _endian){
      WriteUnsignedInt32(value-(value/int32)*int32);
      WriteUnsignedInt32(value/int32);
    }
  } 

  void ByteArray::WriteDouble(const double &value)
  {
    unsigned long long realValue = TransformType<unsigned long long>(&value);
    WriteUnsignedInt64(realValue);
  }   
  
  void ByteArray::WriteString(const std::string &value)
  {
    ////std::cout<<"str = |"<<value<<"| size: "<<value.size()<<std::endl;
    int len = value.size();
    if(BYTE_BIG_ENDIAN == _endian){
      _bytes.push_back(len>>8);
      _bytes.push_back(len%256);
    }else if(BYTE_LITTLE_ENDIAN == _endian){
      _bytes.push_back(len%256);
      _bytes.push_back(len>>8);
    }
    for(int i = 0; i < len; i++){
      _bytes.push_back(value[i]);
    }
  }

  ByteArray &ByteArray::operator+(ByteArray other)
  {
    ReadFromByteArray(&other,other.BytesAvailable());
    return *this;
  }

  ByteArray &ByteArray::operator=(ByteArray other)
  {
    clear();
    ReadFromByteArray(&other,other.BytesAvailable());
    return *this;
  }

  void ByteArray::ReadFromByteArray(ByteArray *other, int bytesLen)
  {
    if(0 == bytesLen || other->BytesAvailable() < (unsigned int)bytesLen)
      bytesLen = other->BytesAvailable();
    for(int i = 0; i < bytesLen ; i++){
      _bytes.push_back(other->_bytes[0]);
      other->_bytes.pop_front();
    }
  }

  void ByteArray::ReadFromData(unsigned char *data, int bytesLen)
  {
    if(0 > bytesLen)
      return;
    for(int i = 0; i < bytesLen ; i++){
      _bytes.push_back(*data);
      data = data + 1;
    }
  }

  void ByteArray::ShowAllBytes()
  {
    int allSize = BytesAvailable();
    for(int i = 0; i < allSize; i++){
      std::cout<<"bytes["<<i<<"] = "<<char(_bytes[i])<<"("<<int(_bytes[i])<<")"<<std::endl;
    }
  }

  void ByteArray::WriteObject(const void *obj,unsigned int size)
  {
    const char *tmp = static_cast<const char *>(obj);
    for(unsigned int i = 0 ; i < size ; ++i)
      _bytes.push_back(tmp[i]);
  }

}
