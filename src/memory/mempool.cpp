#include <memory/mempool.hpp>

namespace face2wind
{

MemoryPool::ContiguousBuffer::ContiguousBuffer(int item_len, int item_num)
    :item_len_(0), buff_len_(0), item_num_(0), buff_(nullptr)
{
  if (item_len <= 0 || item_num <= 0)
    return;

  item_len_ = item_len;
  buff_len_ = item_len * item_num;
  if (buff_len_ >= MEMORY_POOL_MAX_BUFFER_LEN)
    buff_len_ = MEMORY_POOL_MAX_BUFFER_LEN;

  // fix buffer len to item_len's times
  item_num_ = buff_len_ / item_len;
  buff_len_ = item_num_ * item_len;

  buff_ = new char[buff_len_];
  for (int i = 0; i < item_num_; ++i )
    free_buffs_.push(buff_ + (i*item_len));
}

MemoryPool::ContiguousBuffer::~ContiguousBuffer()
{
  if (buff_len_ > 0 && nullptr != buff_)
    delete []buff_;
}

bool MemoryPool::ContiguousBuffer::HasFreeBuffer()
{
  return !free_buffs_.empty();
}

void *MemoryPool::ContiguousBuffer::GetFreeBuffer()
{
  if (!this->HasFreeBuffer())
    return nullptr;

  void *tmp_buff = (void*)free_buffs_.top();
  free_buffs_.pop();
  on_use_buffers_.insert((char*)tmp_buff);
  return tmp_buff;
}

bool MemoryPool::ContiguousBuffer::FreeBuffer(void *buffer)
{
  char *tmp_buff = (char*)buffer;
  int offset = tmp_buff - buff_;
  if (item_len_ <= 0 || offset < 0 || offset >= buff_len_)
    return false;

  if (offset % item_len_ > 0)
    return false;

  if (on_use_buffers_.find(tmp_buff) == on_use_buffers_.end())
    return false;

  on_use_buffers_.erase(tmp_buff);
  free_buffs_.push(tmp_buff);
  
  return true;
}

MemoryPool::MemoryPool(int item_len)
{
  item_len_ = item_len;
}

MemoryPool::~MemoryPool()
{
  for (ContiguousBuffer *buffer:pool_item_list_)
  {
    if (nullptr != buffer)
      delete buffer;
  }
}

void *MemoryPool::Alloc()
{
  if (!free_pool_item_list_.empty())
  {
    return nullptr;
  }

  int alloc_buffer_num = total_cache_item_num_;
  if (alloc_buffer_num <= 0)
    alloc_buffer_num = MEMORY_POOL_FIRST_ALLOC_NUM;
  
  ContiguousBuffer *buffer = new ContiguousBuffer(item_len_, alloc_buffer_num);
  if (nullptr == buffer)
    return nullptr;
  
  pool_item_list_.insert(buffer);
  total_cache_item_num_ += buffer->GetItemNum();
  
  void *mem = buffer->GetFreeBuffer();
  
  if (buffer->HasFreeBuffer())
    free_pool_item_list_.push(buffer);
  
  return mem;
}

bool MemoryPool::Free(void *memory)
{
  auto buffer_it = memory_to_buffer_map_.find(memory);
  if (buffer_it != memory_to_buffer_map_.end())
  {
    bool free_succ = buffer_it->second->FreeBuffer(memory);
    if (free_succ)
      free_pool_item_list_.push(buffer_it->second);
    return free_succ;
  }

  return false;
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
  int align = sizeof(long);
  int fix_size = size + align - size % align;
  auto pool_it = memory_size_to_pool_map_.find(fix_size);
  if (pool_it == memory_size_to_pool_map_.end())
  {
    memory_size_to_pool_map_[fix_size] = new MemoryPool(fix_size);
    pool_it = memory_size_to_pool_map_.find(fix_size);
  }

  void *mem = pool_it->second->Alloc();
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

