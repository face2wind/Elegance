#pragma once

namespace face2wind
{

// use for memory alloc and free frequently
// cache memory to speed alloc and free
// useage :
// MemoryPool pool;
// void *data = pool.Alloc(64);
// pool.Free(data);
// But, When pool object are free, the memory created by pool will free too
// So, let pool be global var? or local var? as you choose !
class MemoryPool
{
  class PoolItem
  {
    PoolItem *next_item_;
  };
  
 public:
  void *Alloc(unsigned int size);
  void Free(void *memory);

 private:
  Mutex operate_lock_;
  std::map<void*, unsigned int> memory_to_size_map_;

  std::map<unsigned int, std::set or not <PoolItem*> > memory_size_to_head_map_;
};

}
