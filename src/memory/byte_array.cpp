#include "byte_array.hpp"

#include <iostream>

#include <string>
#include <sys/types.h>
#include <string.h> //strerror()
#include <stdio.h>
#include <errno.h>

namespace face2wind {

signed char ByteArray::ReadInt8()
{
  if(0 == bytes_queue_.size())
    return 0;
  unsigned char tmp = bytes_queue_[0];
  bytes_queue_.pop_front();
  if(show_debug_msg_)
    std::cout<<"ReadInt8() = "<<int(tmp)<<"\n";
  return tmp;
}

unsigned char ByteArray::ReadUint8()
{
  if(0 == bytes_queue_.size())
    return 0;
  unsigned char tmp = bytes_queue_[0];
  bytes_queue_.pop_front();
  if(show_debug_msg_)
    std::cout<<"ReadUint8() = "<<int(tmp)<<"\n";
  return tmp;
}

short ByteArray::ReadInt16()
{
  unsigned short value = ReadUint16();
  short realValue = TransformType<short>(&value);
  if(show_debug_msg_)
    std::cout<<"ReadInt16() = "<<realValue<<"\n";
  return realValue;
}

unsigned short ByteArray::ReadUint16()
{
  unsigned short value = 0;
  unsigned char left = ReadUint8();
  unsigned char right = ReadUint8();
  if(Endian::BIG_ENDIAN == cur_endian_){
    value = (left<<8) + right;
  }else{
    value = left + (right<<8);
  }
  if(show_debug_msg_)
    std::cout<<"ReadUint16() = "<<value<<"\n";
  return value;
}

int ByteArray::ReadInt32()
{
  unsigned int value = ReadUint32();
  long realValue = TransformType<long>(&value);
  if(show_debug_msg_)
    std::cout<<"ReadInt32() = "<<realValue<<"\n";
  return realValue;
}

unsigned int ByteArray::ReadUint32()
{
  unsigned int value = 0;
  unsigned int left = ReadUint16();
  unsigned int right = ReadUint16();
  if(Endian::BIG_ENDIAN == cur_endian_){
    value = (left<<16) + right;
  }else{
    value = left + (right<<16);
  }
  if(show_debug_msg_)
    std::cout<<"ReadUint32() = "<<value<<"\n";
  return value;
}

float ByteArray::ReadFloat()
{
  unsigned long value = ReadUint32();
  return TransformType<float>(&value);
}

double ByteArray::ReadDouble()
{
  unsigned long long value = ReadUint64();
  double realValue = TransformType<double>(&value);
  return realValue;
}

long long ByteArray::ReadInt64()
{
  unsigned long long value = ReadUint64();
  long long realValue = TransformType<long long>(&value);
  return realValue;
}

unsigned long long ByteArray::ReadUint64()
{
  unsigned long long value = 0;
  unsigned int rawLeft = ReadUint32();
  unsigned int rawRight = ReadUint32();
  if(Endian::BIG_ENDIAN == cur_endian_){
    value = ((unsigned long long)rawLeft << 32) + rawRight;
  }else{
    value = rawLeft + ((unsigned long long)rawRight << 32);
  }
  return value;
}

std::string ByteArray::ReadString()
{
  std::string str;
  if(2 > bytes_queue_.size()) // not enough length bytes
    return str;
  unsigned int len = 0;;
  if(Endian::BIG_ENDIAN == cur_endian_){
    len = int(bytes_queue_[0])<<8;
    len += int(bytes_queue_[1]);
  }
  else if(Endian::LITTLE_ENDIAN == cur_endian_){
    len = int(bytes_queue_[1])<<8;
    len += int(bytes_queue_[0]);
  }
  if( (2+len) > bytes_queue_.size() ) // not enough length string
    return str;

  bytes_queue_.pop_front();
  bytes_queue_.pop_front();
  while(len){
    --len;
    str += bytes_queue_[0];
    bytes_queue_.pop_front();
  }
  return str;
}

void *ByteArray::ReadObject(unsigned int size)
{
  if(size > bytes_queue_.size())
    return nullptr;
  if(0 == size) // 0 means use all bytes
    size = (unsigned int)bytes_queue_.size();
    
  char *data = new char[size];
  for(unsigned int i = 0; i < size; ++i){
    data[i] = bytes_queue_[0];
    bytes_queue_.pop_front();
  }
  return static_cast<void *>(data);
}

void ByteArray::WriteInt8(const signed char &value)
{
  unsigned char realValue = TransformType<unsigned char>(&value);
  if(show_debug_msg_)
    std::cout<<"WriteInt8("<<int(realValue)<<std::endl;
  bytes_queue_.push_back(realValue);
}

void ByteArray::WriteUint8(const unsigned char &value)
{
  bytes_queue_.push_back(value);
}

void ByteArray::WriteInt16(const short &value)
{
  if(show_debug_msg_)
    std::cout<<"WriteInt16("<<value<<")\n";
  unsigned short realValue = TransformType<unsigned short>(&value);
  WriteUint16(realValue);
}

void ByteArray::WriteUint16(const unsigned short &value)
{
  if(show_debug_msg_)
    std::cout<<"WriteUint16("<<value<<")\n";
  if(Endian::BIG_ENDIAN == cur_endian_){
    WriteUint8(value>>8);
    WriteUint8(value%256);
  }
  else if(Endian::LITTLE_ENDIAN == cur_endian_){
    WriteUint8(value%256);
    WriteUint8(value>>8);
  }
}
  
void ByteArray::WriteInt32(const int &value)
{
  if(show_debug_msg_)
    std::cout<<"WriteInt32("<<value<<")\n";
  unsigned int realValue = TransformType<unsigned int>(&value);
  WriteUint32(realValue);
}

void ByteArray::WriteUint32(const unsigned int &value)
{
  if(show_debug_msg_)
    std::cout<<"WriteUint32("<<value<<")\n";
  if(Endian::BIG_ENDIAN == cur_endian_){
    WriteUint16(value >> 16);
    WriteUint16((value << 16) >> 16);
  }else if(Endian::LITTLE_ENDIAN == cur_endian_){
    WriteUint16((value << 16) >> 16);
    WriteUint16(value >> 16);
  }
}

void ByteArray::WriteFloat(const float &value)
{
  unsigned long realValue = TransformType<unsigned long>(&value);
  WriteUint32(realValue);
}

void ByteArray::WriteInt64(const long long &value)
{
  unsigned long long realValue = TransformType<unsigned long long>(&value);
  WriteUint64(realValue);
} 

void ByteArray::WriteUint64(const unsigned long long &value)
{
  if(Endian::BIG_ENDIAN == cur_endian_){
    WriteUint32(value >> 32);
    WriteUint32((value << 32) >> 32);
  }else if(Endian::LITTLE_ENDIAN == cur_endian_){
    WriteUint32((value << 32) >> 32);
    WriteUint32(value >> 32);
  }
} 

void ByteArray::WriteDouble(const double &value)
{
  unsigned long long realValue = TransformType<unsigned long long>(&value);
  WriteUint64(realValue);
}   
  
void ByteArray::WriteString(const std::string &value)
{
  if (value.size() > USHRT_MAX)
    return;

  if(show_debug_msg_)
    std::cout<<"str = |"<<value<<"| size: "<<value.size()<<std::endl;

  unsigned short len = (unsigned short)value.size();
  this->WriteUint16(len);
  for(int i = 0; i < len; i++)
    bytes_queue_.push_back(value[i]);
}

void ByteArray::WriteObject(const void *obj, int bytesLen)
{
  if (bytesLen <= 0)
    return;

  const unsigned char *data = (const unsigned char *)obj;
  for (int i = 0; i < bytesLen ; i++)
  {
    bytes_queue_.push_back(*data);
    ++ data;
  }
}

ByteArray &ByteArray::operator+(ByteArray other)
{
  ReadFromByteArray(&other,other.BytesAvailable());
  return *this;
}

ByteArray &ByteArray::operator=(ByteArray other)
{
  Clear();
  ReadFromByteArray(&other,other.BytesAvailable());
  return *this;
}

void ByteArray::ReadFromByteArray(ByteArray *other, int bytesLen)
{
  if(0 == bytesLen || other->BytesAvailable() < (unsigned int)bytesLen)
    bytesLen = other->BytesAvailable();
  
  for(int i = 0; i < bytesLen ; i++)
  {
    bytes_queue_.push_back(other->bytes_queue_[0]);
    other->bytes_queue_.pop_front();
  }
}

void ByteArray::ShowAllBytes()
{
  int allSize = BytesAvailable();
  for(int i = 0; i < allSize; i++)
    std::cout<<"bytes["<<i<<"] = "<<char(bytes_queue_[i])<<"("<<int(bytes_queue_[i])<<")"<<std::endl;
}

}
