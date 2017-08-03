#pragma once

#include <platform/thread/mutex.hpp>

#include <map>
#include <set>
#include <stack>
#include <vector>

namespace face2wind
{

static const int MEMORY_POOL_MAX_BUFFER_LEN = 1024 * 1024 * 10;
static const int MEMORY_POOL_DEFAULT_INCREASE_ALLOC_NUM = 10;

class MemoryPoolManager;

// use for memory alloc and free frequently
// cache memory to speed up alloc and free
// useage :
// MemoryPool pool(64);
// void *data = pool.Alloc();
// pool.Free(data);
// But, When pool object are free, the memory created by pool will free too
// Be carefully, Free memory param must come from Alloc, here will not check
class MemoryPool
{
 public:
  MemoryPool(int item_len, int increase_num = MEMORY_POOL_DEFAULT_INCREASE_ALLOC_NUM);
  ~MemoryPool();
  
  void *Alloc();
  bool Free(void *memory);
  
 private:
  int item_len_;
  int increase_num_;

  std::stack<void*> free_buffers_;
  std::vector<char*> real_buffer_ptrs_;
};

// base on MemoryPool, make alloc and free more like default functions
// BUT !  if you need more efficient, do not use MemoryPoolManager, use MemoryPool instead
// useage :
// MemoryPoolManager pool;
// void *data = pool.Alloc(64);
// pool.Free(data);
// When pool object are free, the memory created by pool will free too
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
