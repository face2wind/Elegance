#include <elegance/memory/mempool.hpp>

namespace face2wind
{

MemoryPool::MemoryPool(int item_len, int increase_num)
{
  item_len_ = item_len;
  increase_num_ = increase_num;
}

MemoryPool::~MemoryPool()
{
  for (auto buff_it : real_buffer_ptrs_)
  {
    delete []buff_it;
  }
}

void *MemoryPool::Alloc()
{
  if (free_buffers_.empty())
  {
    char *buff_ = new char[increase_num_ * item_len_];
    real_buffer_ptrs_.push_back(buff_);
    for (int i = 0; i < increase_num_; ++i )
      free_buffers_.push(buff_ + (i*item_len_));
  }

  void *mem = (void*)free_buffers_.top();

  free_buffers_.pop();
  return mem;
}

bool MemoryPool::Free(void *memory)
{
  free_buffers_.push((void*)memory);
  return true;
}

MemoryPoolManager::MemoryPoolManager()
{

}

MemoryPoolManager::~MemoryPoolManager()
{
  for (auto pool_it : memory_size_to_pool_map_)
    delete pool_it.second;
  memory_size_to_pool_map_.clear();
}

void *MemoryPoolManager::Alloc(int size)
{
  void *mem = nullptr;

  int align = sizeof(long);
  int fix_size = size + align - size % align;
  auto pool_it = memory_size_to_pool_map_.find(fix_size);
  if (pool_it == memory_size_to_pool_map_.end())
  {
    MemoryPool *pool = new MemoryPool(fix_size);
    memory_size_to_pool_map_[fix_size] = pool;
    mem = pool->Alloc();
  }
  else
  {
    mem = pool_it->second->Alloc();
  }

  if (nullptr != mem)
    memory_to_size_map_[mem] = fix_size;
  
  return mem;
}

bool MemoryPoolManager::Free(void *memory)
{
  auto size_it = memory_to_size_map_.find(memory);
  if (size_it == memory_to_size_map_.end())
    return false;

  auto pool_it = memory_size_to_pool_map_.find(size_it->second);
  if (pool_it == memory_size_to_pool_map_.end())
    return false;

  //memory_to_size_map_.erase(size_it); no need erase, it's always valid
  
  return pool_it->second->Free(memory);
}

}

