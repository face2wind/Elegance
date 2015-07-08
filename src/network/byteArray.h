#ifndef BYTE_ARRAY_HPP
#define BYTE_ARRAY_HPP

#include <deque>
#include <string>

namespace face2wind {

  enum Endian {
    BYTE_BIG_ENDIAN = 0,
    BYTE_LITTLE_ENDIAN
  };

  template<class T>
  T TransformType(const void *const value)
  {
    T target = *(T*)value;
    return target;
  }

  class ByteArray
  {
  public:
    
    void setEndian(Endian e){_endian=e;}
    Endian getEndian(){return _endian;}
        
    unsigned int BytesAvailable(){return ( unsigned int)(_bytes.size());}

    signed char ReadSignedInt8();
    unsigned char ReadUnsignedInt8();

    short ReadSignedInt16();
    unsigned short ReadUnsignedInt16();

    int ReadSignedInt32();
    unsigned int ReadUnsignedInt32();
    float ReadFloat();

    long long ReadSignedInt64();
    unsigned long long ReadUnsignedInt64();
    double ReadDouble();

    std::string ReadAS3String();
    void *ReadObject(unsigned int size);

    void WriteSignedInt8(const signed char &value);
    void WriteUnsignedInt8(const unsigned char &value);

    void WriteSignedInt16(const short &value);
    void WriteUnsignedInt16(const unsigned short &value);

    void WriteSignedInt32(const int &value);
    void WriteUnsignedInt32(const unsigned int &value);
    void WriteFloat(const float &value);

    void WriteSignedInt64(const long long &value);
    void WriteUnsignedInt64(const unsigned long long &value);
    void WriteDouble(const double &value);

    void WriteString(const std::string &value);
    void WriteObject(const void *obj,unsigned int size);

    ByteArray &operator+(ByteArray other);
    ByteArray &operator=(ByteArray other);

    void ShowAllBytes();
    void ReadFromByteArray(ByteArray *other,int bytesLen = 0);
	void ReadFromData(unsigned char *data, int bytesLen);

    void clear(){
      _bytes.clear();
    }
    
    ByteArray()
      :_endian(BYTE_LITTLE_ENDIAN),_readBuffSize(5),showDebugMsg(false){}
    ~ByteArray(){}

  private:
    std::deque<unsigned char> _bytes;
    Endian _endian;    
    int _readBuffSize;
  public:
    bool showDebugMsg; 
  };

}

#endif //BYTE_ARRAY_HPP
