#include <iostream>
#include <elegance/elegance.hpp>

namespace test_byte_array {

	struct TestObj
	{
		char aa;
		int bb;
		double cc;
	};

	int TestByteArrayTmp(face2wind::ByteArray &by1, face2wind::ByteArray &by2)
	{
		by1.WriteInt8(1);
		by1.WriteUint8(2);
		by1.WriteInt16(3);
		by1.WriteUint16(4);
		by1.WriteInt32(5);
		by1.WriteUint32(6);
		by1.WriteFloat(7.5);
		by1.WriteInt64(8);
		by1.WriteUint64(9);
		by1.WriteDouble(10.5);
		by1.WriteString("I love you!");

		TestObj tt;
		tt.aa = 11;
		tt.bb = 22;
		tt.cc = 33.99;
		by1.WriteObject((void*)&tt, sizeof(TestObj));

		std::cout << "by1.ReadInt8() = " << int(by1.ReadInt8()) << std::endl;
		std::cout << "by1.ReadUint8() = " << int(by1.ReadUint8()) << std::endl;
		std::cout << "by1.ReadInt16() = " << by1.ReadInt16() << std::endl;
		std::cout << "by1.ReadUint16() = " << by1.ReadUint16() << std::endl;
		std::cout << "by1.ReadInt32() = " << by1.ReadInt32() << std::endl;
		std::cout << "by1.ReadUint32() = " << by1.ReadUint32() << std::endl;
		std::cout << "by1.ReadFloat() = " << by1.ReadFloat() << std::endl;
		std::cout << "by1.ReadInt64() = " << by1.ReadInt64() << std::endl;
		std::cout << "by1.ReadUint64() = " << by1.ReadUint64() << std::endl;
		std::cout << "by1.ReadDouble() = " << by1.ReadDouble() << std::endl;
		std::cout << "by1.ReadString() = " << by1.ReadString() << std::endl;

		by2 = by1;

		TestObj *r_tt = (TestObj *)by2.ReadObject(sizeof(TestObj));
		if (nullptr != r_tt)
		{
			std::cout << "by1.ReadObject() = { aa = " << int(r_tt->aa) << ", bb = " << r_tt->bb << ", cc = " << r_tt->cc << " }" << std::endl;
			delete r_tt;
		}

		return 0;
	}

}

using namespace test_byte_array;

int TestByteArray()
{
	face2wind::StaticByteArray by1, by2;
	std::cout << "Test face2wind::StaticByteArray ============ Begin" << std::endl;
	TestByteArrayTmp(by1, by2);
	std::cout << "Test face2wind::StaticByteArray ============ End\n" << std::endl;

	face2wind::QueueByteArray by3, by4;
	std::cout << "Test face2wind::QueueByteArray ============ Begin" << std::endl;
	TestByteArrayTmp(by3, by4);
	std::cout << "Test face2wind::QueueByteArray ============ Begin" << std::endl;

	face2wind::Timer::Sleep(10000);

	return 0;
}
