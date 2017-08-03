#pragma once

#include <deque>

#include "byte_array.hpp"

namespace face2wind {

template<class T>
T TransformType(const void *const value)
{
  T target = *(T*)value;
  return target;
}

class QueueByteArray : public ByteArray
{
 public:
  unsigned int BytesAvailable(){return ( unsigned int)(bytes_queue_.size());}

  signed char ReadInt8();
  unsigned char ReadUint8();

  short ReadInt16();
  unsigned short ReadUint16();

  int ReadInt32();
  unsigned int ReadUint32();
  float ReadFloat();

  long long ReadInt64();
  unsigned long long ReadUint64();
  double ReadDouble();

  std::string ReadString();
  void *ReadObject(unsigned int size);

  void WriteInt8(const signed char &value);
  void WriteUint8(const unsigned char &value);

  void WriteInt16(const short &value);
  void WriteUint16(const unsigned short &value);

  void WriteInt32(const int &value);
  void WriteUint32(const unsigned int &value);
  void WriteFloat(const float &value);

  void WriteInt64(const long long &value);
  void WriteUint64(const unsigned long long &value);
  void WriteDouble(const double &value);

  void WriteString(const std::string &value);
  void WriteObject(const void *obj, int bytesLen);

  QueueByteArray &operator+(QueueByteArray other);
  QueueByteArray &operator=(QueueByteArray other);

  void ReadFromByteArray(ByteArray *other,int bytesLen = 0);
  void ShowAllBytes();

  void Clear(){
    bytes_queue_.clear();
  }
    
  QueueByteArray() : show_debug_msg_(false) {}
  ~QueueByteArray(){}

 private:
  std::deque<unsigned char> bytes_queue_;
  
 public:
  bool show_debug_msg_; 
};

}


