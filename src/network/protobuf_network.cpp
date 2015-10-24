#include "protobuf_network.h"
#include "network.hpp"

namespace face2wind
{
	using google::protobuf::Message;
	using google::protobuf::Descriptor;
	using google::protobuf::DescriptorPool;
	using google::protobuf::MessageFactory;

	ProtobuffNetwork::ProtobuffNetwork()
	{
	}

	ProtobuffNetwork::~ProtobuffNetwork()
	{
	}

	ProtobuffNetwork * ProtobuffNetwork::GetInstance()
	{
		static ProtobuffNetwork instance;
		return &instance;
	}

	bool ProtobuffNetwork::RegistHandler(IProtobufNetworkHandler * handler)
	{
		m_proto_network_handler_set.insert(handler);
		return true;
	}

	void ProtobuffNetwork::SendProtobuffMsg(NetworkID network_id, const Message * message)
	{
		static char data[PROTOBUF_MESSAGE_MAX_LEN];

		const std::string &type_name = message->GetTypeName();
		std::string msg_serialize_str;
		message->SerializeToString(&msg_serialize_str);

		ProtobufHeader *proto_head = (ProtobufHeader*)data;
		proto_head->message_name_len = type_name.size();
		memcpy(proto_head->message_name_str, type_name.c_str(), type_name.size());

		int head_len = sizeof(ProtobufHeader) - (PROTOBUF_MESSAGE_NAME_MAX_LEN + 2 - proto_head->message_name_len);
		char *body_ptr = data + head_len;
		memcpy(body_ptr, msg_serialize_str.c_str(), msg_serialize_str.size());

		int total_len = head_len + msg_serialize_str.size();
		Network::GetInstance()->AsyncSendData(network_id, data, total_len);
	}

	void ProtobuffNetwork::OnConnect(bool is_success, NetworkID network_id, Port local_port, IPAddr remote_ip_addr, Port remote_port)
	{
		for (std::set<IProtobufNetworkHandler*>::iterator it = m_proto_network_handler_set.begin(); it != m_proto_network_handler_set.end(); ++it)
		{
			(*it)->OnConnect(is_success, network_id, local_port, remote_ip_addr, remote_port);
		}
	}

	void ProtobuffNetwork::OnAccept(bool is_success, NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port)
	{
		for (std::set<IProtobufNetworkHandler*>::iterator it = m_proto_network_handler_set.begin(); it != m_proto_network_handler_set.end(); ++it)
		{
			(*it)->OnAccept(is_success, network_id, listen_port, remote_ip_addr, remote_port);
		}
	}

	void ProtobuffNetwork::OnRecv(NetworkID network_id, const char *data, int length)
	{
		ProtobufHeader *proto_head = (ProtobufHeader*)data;
		if (proto_head->message_name_len > PROTOBUF_MESSAGE_NAME_MAX_LEN)
			return;

		char type_name[PROTOBUF_MESSAGE_NAME_MAX_LEN + 2];
		memcpy(type_name, proto_head->message_name_str, proto_head->message_name_len);
		type_name[proto_head->message_name_len] = '\0';

		int head_len = sizeof(ProtobufHeader) - (PROTOBUF_MESSAGE_NAME_MAX_LEN + 2 - proto_head->message_name_len);
		data += head_len;

		int left_len = length - head_len;

		Message *msg = this->CreateMessage(type_name);
		msg->ParseFromArray(data, left_len);
		for (std::set<IProtobufNetworkHandler*>::iterator it = m_proto_network_handler_set.begin(); it != m_proto_network_handler_set.end(); ++it)
		{
			(*it)->OnRecv(network_id, msg);
		}
		delete msg;
	}

	void ProtobuffNetwork::OnDisconnect(NetworkID network_id)
	{
		for (std::set<IProtobufNetworkHandler*>::iterator it = m_proto_network_handler_set.begin(); it != m_proto_network_handler_set.end(); ++it)
		{
			(*it)->OnDisconnect(network_id);
		}
	}

	Message *ProtobuffNetwork::CreateMessage(const std::string &type_name)
	{
		Message* message = NULL;
		const Descriptor* descriptor = DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
		if (descriptor)
		{
			const Message* prototype = MessageFactory::generated_factory()->GetPrototype(descriptor);
			if (prototype)
			{
				message = prototype->New();
			}
		}
		return message;
	}

}
