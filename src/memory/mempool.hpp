#pragma once

#include <platform/thread/mutex.hpp>

#include <map>
#include <set>
#include <stack>

namespace face2wind
{

static const int MEMORY_POOL_MAX_BUFFER_LEN = 1024 * 1024 * 10;
static const int MEMORY_POOL_FIRST_ALLOC_NUM = 5;

class MemoryPoolManager;

class MemoryPool
{
  class ContiguousBuffer
  {
   public:
    ContiguousBuffer(int item_len, int item_num);
    ~ContiguousBuffer();

    int GetItemNum() { return item_num_; }
    bool HasFreeBuffer();
    void *GetFreeBuffer();
    bool FreeBuffer(void *buffer);

   private:
    int item_len_;
    int buff_len_;
    int item_num_;
    char * buff_;
    
    std::stack<char*> free_buffs_;
    std::set<char*> on_use_buffers_;
  };

 public:
  MemoryPool(int item_len);
  ~MemoryPool();
  
  void *Alloc();
  bool Free(void *memory);

 private:
  int item_len_;
  int total_cache_item_num_;

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
  MemoryPoolManager();
  ~MemoryPoolManager();
  
  void *Alloc(int size);
  bool Free(void *memory);

 private:
  Mutex operate_lock_;
  std::map<void*, int> memory_to_size_map_;

  std::map<int, MemoryPool*> memory_size_to_pool_map_;
};

}
