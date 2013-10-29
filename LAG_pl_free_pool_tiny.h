/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_PL_FREE_POOL_TINY_H
#define LAG_PL_FREE_POOL_TINY_H

#include "systemc.h"
#include "types.h"

class LAG_pl_free_pool_tiny
{
public:
  
  void reset()
  {
    for(int i = 0; i < num_pls_global; i++)
        pl_alloc_status[i] = (i < num_pls_local);
  }

  const vector<bool> & get_pl_alloc_status() const
  {
    return pl_alloc_status;
  }
  
  const bool get_pl_alloc_status(const int & pl_number) const
  {
    assert(pl_number < num_pls_global);
    return pl_alloc_status[pl_number];
  }
  
  void set_pl_busy(const vector<bool> & new_pl_busy)
  {
    assert(new_pl_busy.size() == num_pls_global);
    
    for (int i = 0; i < num_pls_global; i++)
      if (new_pl_busy[i])
        pl_alloc_status[i] = false;  
  }
  
  void set_pl_busy(const int & num_of_pl_busy)
  {
    assert(num_of_pl_busy < num_pls_global);
    pl_alloc_status[num_of_pl_busy] = false;  
  }
  
  void set_pl_free(const vector<bool> & new_pl_free)
  {
    assert(new_pl_free.size() == num_pls_global);
    
    for (int i = 0; i < num_pls_global; i++)
      if (new_pl_free[i])
        pl_alloc_status[i] = true; 
  }
  
  void set_pl_free(const int & num_of_pl_free)
  {
    assert(num_of_pl_free < num_pls_global);
    pl_alloc_status[num_of_pl_free] = true;  
  }
  
  void process_pl_allocated(const vector<bool> & pl_alocated)
  {
    assert(pl_alocated.size() == num_pls_global);
    
    for (int i = 0; i < num_pls_global; i++)
      if (pl_alocated[i])
        pl_alloc_status[i] = false;
  }
  
  void process_tail_flits(const vector<bool> & flits_tail, const vector<bool> & flits_valid)
  {
    assert(flits_tail.size() == num_pls_global);
    assert(flits_valid.size() == num_pls_global);
    
    for (int i = 0; i < num_pls_global; i++)
      if (flits_tail[i] && flits_valid[i]) 
        pl_alloc_status[i] = true;
  }
  
  LAG_pl_free_pool_tiny(string name_,
                    int num_pls_global_ = 2,
                    int num_pls_local_ = 2
                    ) : name(name_), num_pls_global(num_pls_global_), 
                    num_pls_local(num_pls_local_), pl_alloc_status(num_pls_global_)
  {    
    reset();
  }
  
  
private:

  string name;

  int num_pls_global;
  int num_pls_local;
  
  vector<bool> pl_alloc_status;
    
};

#endif