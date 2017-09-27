#pragma once

// platform hpps

#include <elegance/platform/network/socket_def.hpp>
#include <elegance/platform/network/socket_accept.hpp>
#include <elegance/platform/network/socket_connect.hpp>

#include <elegance/platform/thread/mutex.hpp>
#include <elegance/platform/thread/signal.hpp>
#include <elegance/platform/thread/thread.hpp>
#include <elegance/platform/thread/thread_pool.hpp>

#include <elegance/platform/common/timer.hpp>
#include <elegance/platform/common/system_info.hpp>


// non platform hpps

#include <elegance/memory/mempool.hpp>
#include <elegance/memory/byte_array/byte_array.hpp>
#include <elegance/memory/byte_array/queue_byte_array.hpp>
#include <elegance/memory/byte_array/static_byte_array.hpp>
#include <elegance/memory/serialize/serialize_manager.hpp>

#include <elegance/common/random.hpp>
#include <elegance/common/debug_message.hpp>

#include <elegance/network/network_manager.hpp>

#include <elegance/rpc/rpc_manager.hpp>
