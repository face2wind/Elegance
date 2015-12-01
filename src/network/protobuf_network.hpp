#ifndef __PROTOBUF_NETWROK_H__
#define __PROTOBUF_NETWROK_H__

#include <set>
#include "i_network.hpp"
#include "google/protobuf/message.h"

/////////////////////////////////////////////////////////////////////////////
//    this file has not test yet
/////////////////////////////////////////////////////////////////////////////

namespace face2wind
{
  using google::protobuf::Message;

  static const int PROTOBUF_MESSAGE_MAX_LEN = 512;
  static const int PROTOBUF_MESSAGE_NAME_MAX_LEN = 80;

  struct ProtobufHeader
  {
    short message_name_len;
    char message_name_str[PROTOBUF_MESSAGE_NAME_MAX_LEN + 2];
  };

  class IProtobufNetworkHandler
  {
 public:
    IProtobufNetworkHandler() {}
    virtual ~IProtobufNetworkHandler() {}

    virtual void OnConnect(bool is_success, NetworkID network_id, Port local_port, IPAddr remote_ip_addr, Port remote_port) = 0;
    virtual void OnAccept(bool is_success, NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port) = 0;
    virtual void OnRecv(NetworkID network_id, const Message *msg) = 0;
    virtual void OnDisconnect(NetworkID network_id) = 0;
  };

  class ProtobuffNetwork : public INetworkHandler
  {
 public:
    ProtobuffNetwork();
    virtual ~ProtobuffNetwork();

    static ProtobuffNetwork *GetInstance();

    virtual bool RegistHandler(IProtobufNetworkHandler *handler);
    virtual void SendProtobuffMsg(NetworkID network_id, const Message *message);

    friend class Network;

 protected:
    virtual void OnConnect(bool is_success, NetworkID network_id, Port local_port, IPAddr remote_ip_addr, Port remote_port);
    virtual void OnAccept(bool is_success, NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port);
    virtual void OnRecv(NetworkID network_id, const char *data, int length);
    virtual void OnDisconnect(NetworkID network_id);

    Message *CreateMessage(const std::string &type_name);

 protected:
    std::set<IProtobufNetworkHandler*> m_proto_network_handler_set;
  };

}

#endif
