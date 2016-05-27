#include <memory/mempool.hpp>

namespace face2wind
{

MemoryPool::ContiguousBuffer::ContiguousBuffer(unsigned int item_len, unsigned int item_num)
{
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

}

