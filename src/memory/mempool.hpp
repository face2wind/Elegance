#pragma once

#include <platform/thread/mutex.hpp>

#include <map>
#include <set>
#include <stack>

namespace face2wind
{

class MemoryPoolManager;

class MemoryPool
{
  class ContiguousBuffer
  {
   public:
    ContiguousBuffer(unsigned int item_len, unsigned int item_num);
    ~ContiguousBuffer();

    bool HasFreeBuffer();
    void *GetFreeBuffer();
    void FreeBuffer(void *buffer);

   private:
    unsigned int buff_len_;
    char * buff_;
    
    std::stack<char*> free_buffs_;
    std::set<char*> on_use_buffers_;
  };

 public:
  void *Alloc();
  void Free(void *memory);

 private:
  unsigned int item_len;
  unsigned int total_cache_item_num;

  std::set<ContiguousBuffer *> pool_item_list_;
  
  std::stack<ContiguousBuffer*> free_pool_item_list_;
  std::map<void*, ContiguousBuffer*> memory_to_buffer_map_;
};

// use for memory alloc and free frequently
// cache memory to speed up alloc and free
// useage :
// MemoryPoolManager pool;
// void *data = pool.Alloc(64);
// pool.Free(data);
// But, When pool object are free, the memory created by pool will free too
// So, let pool be global var? or local var? as you choose !
class MemoryPoolManager
{
 public:
  void *Alloc(unsigned int size);
  void Free(void *memory);

 private:
  Mutex operate_lock_;
  std::map<void*, unsigned int> memory_to_size_map_;

  std::map<unsigned int, MemoryPool> memory_size_to_head_map_;
};

}
