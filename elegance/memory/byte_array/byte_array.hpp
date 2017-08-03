#pragma once

#include <string>

namespace face2wind {

enum class Endian
{
  FW_BIG_ENDIAN = 0,
  FW_LITTLE_ENDIAN
};

class ByteArray
{
 public:
  ByteArray() : cur_endian_(Endian::FW_LITTLE_ENDIAN) {}
  virtual ~ByteArray() {}

  void SetEndian(Endian edian){cur_endian_ = edian;}
  Endian GetEndian(){return cur_endian_;}

  virtual unsigned int BytesAvailable() = 0;

  virtual signed char ReadInt8() = 0;
  virtual unsigned char ReadUint8() = 0;

  virtual short ReadInt16() = 0;
  virtual unsigned short ReadUint16() = 0;

  virtual int ReadInt32() = 0;
  virtual unsigned int ReadUint32() = 0;
  virtual float ReadFloat() = 0;

  virtual long long ReadInt64() = 0;
  virtual unsigned long long ReadUint64() = 0;
  virtual double ReadDouble() = 0;

  virtual std::string ReadString() = 0;
  virtual void *ReadObject(unsigned int size) = 0;

  virtual void WriteInt8(const signed char &value) = 0;
  virtual void WriteUint8(const unsigned char &value) = 0;

  virtual void WriteInt16(const short &value) = 0;
  virtual void WriteUint16(const unsigned short &value) = 0;

  virtual void WriteInt32(const int &value) = 0;
  virtual void WriteUint32(const unsigned int &value) = 0;
  virtual void WriteFloat(const float &value) = 0;

  virtual void WriteInt64(const long long &value) = 0;
  virtual void WriteUint64(const unsigned long long &value) = 0;
  virtual void WriteDouble(const double &value) = 0;

  virtual void WriteString(const std::string &value) = 0;
  virtual void WriteObject(const void *obj, int bytesLen) = 0;

  //virtual ByteArray &operator+(ByteArray other) = 0;
  //virtual ByteArray &operator=(ByteArray other) = 0;

  virtual void ReadFromByteArray(ByteArray *other,int bytesLen = 0) = 0;
  virtual void ShowAllBytes() = 0;

  virtual void Clear() = 0;

 private:
  Endian cur_endian_;
  
};

}
