#ifndef _SOCKET_DATA_HPP_
#define _SOCKET_DATA_HPP_

#include <string>
#include <boost/asio.hpp>

namespace face2wind
{
	using boost::asio::ip::tcp;

	class SocketData
	{
	public:
		SocketData(boost::asio::io_service& io_service) : m_socket(io_service), m_buffer(NULL) {}
		~SocketData() { if (NULL != m_buffer) delete [] m_buffer; }

		tcp::socket &GetSocket() { return m_socket; }
		char *GetBuffer() { return m_buffer; }
		int GetBufferSize() { return m_buffer_size; }

		bool ChangeBufferSize(int size)
		{
			if (0 >= size)
				return false;

			m_buffer_size = size;
			if (NULL != m_buffer) delete [] m_buffer;
			m_buffer = new char(size);

			return true;
		}

	protected:
		tcp::socket m_socket;
		char *m_buffer;
		int m_buffer_size;
	};

	typedef boost::shared_ptr<SocketData> SocketPtr;

	static const int MESSAGE_HEADER_LENGTH = 2;
	typedef unsigned short MessageHeader;
}

#endif