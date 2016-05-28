#include <memory/mempool.hpp>

namespace face2wind
{

MemoryPool::ContiguousBuffer::ContiguousBuffer(unsigned int item_len, unsigned int item_num)
{
  buff_len_ = item_len * item_num;
  if (buff_len_ >= MEMORY_POOL_MAX_BUFFER_LEN)
    buff_len_ = MEMORY_POOL_MAX_BUFFER_LEN;

  // fix buffer len to item_len's times
  buff_len_ = (buff_len_ / item_len) * item_len;
}

MemoryPool::ContiguousBuffer::~ContiguousBuffer()
{
}

bool MemoryPool::ContiguousBuffer::HasFreeBuffer()
{
  return false;
}

void *MemoryPool::ContiguousBuffer::GetFreeBuffer()
{
  return nullptr;
}

void MemoryPool::ContiguousBuffer::FreeBuffer(void *buffer)
{
}

void *MemoryPool::Alloc()
{
  return nullptr;
}

void MemoryPool::Free(void *memory)
{

}

void *MemoryPoolManager::Alloc(unsigned int size)
{
  return nullptr;
}

void MemoryPoolManager::Free(void *memory)
{
}

}

