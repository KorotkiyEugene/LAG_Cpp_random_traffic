/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_INPUT_TRUNK_TINY_H
#define LAG_INPUT_TRUNK_TINY_H

#include "types.h"
#include "systemc.h"
#include <queue>
#include <vector>

using std::vector;
using std::deque; 

class LAG_input_trunk_tiny
{
public:

  void reset()
  {
    for (int i = 0; i < n; i++)
    {
      count[i] = 0;
      allocated_pl[i] = 0;
      allocated_pl_valid[i] = false;
      fifos[i].clear(); 
    }
  }

  void push(const vector<flit_t> & data_in, const vector<bool> & push_to_pl_fifo_tail)
  {
    assert(data_in.size() == n);
    assert(push_to_pl_fifo_tail.size() == n);
    
    for (int i = 0; i < n; i++)
    {
      if (push_to_pl_fifo_tail[i])
        if (count[i] < size)
        {
          count[i]++;
          fifos[i].push_back(data_in[i]);
           
        } else
        {
          cout << sc_time_stamp() << " -- ERROR in " << name << " !!! \tFifo " << i << " overflow." << endl;
          sc_stop();
        }
    }
  }
  
  void pop(const vector<bool> & pop_from_pl_fifo_head)
  {
    assert(pop_from_pl_fifo_head.size() == n);
  
    for (int i = 0; i < n; i++)
    {
      if (pop_from_pl_fifo_head[i])
        if (count[i])
        {
          count[i]--;
          
          if (fifos[i].front().tail)
            allocated_pl_valid[i] = false;  
          
          fifos[i].pop_front();
          
        } else
        {
          cout << sc_time_stamp() << " -- ERROR in " << name << " !!! \tFifo " << i << " underflow." << endl;
          sc_stop();
        }
    }
  
  }
  
  const vector<int> & get_allocated_pl() const
  {
    return allocated_pl;
  }
  
  int get_allocated_pl(const int & pl_number) const
  {
    assert(pl_number < n);
    return allocated_pl[pl_number];
  }
  
  const vector<bool> & is_allocated_pl_valid() const
  {
    return allocated_pl_valid;
  }

  bool is_allocated_pl_valid(const int & pl_number) const
  {       
    assert(pl_number < n);
    return allocated_pl_valid[pl_number];
  }
  
  void set_allocated_pl_new(const vector<int> & new_allocated_pl, const vector<bool> & new_allocated_pl_valid)
  {
    assert(new_allocated_pl.size() == n);
    assert(new_allocated_pl_valid.size() == n);
  
    for (int i = 0; i < n; i++)
      if (new_allocated_pl_valid[i])
      {
        allocated_pl_valid[i] = true;
        allocated_pl[i] = new_allocated_pl[i];
      }
  }
  
  const vector<bool> & is_empty()
  {
    for (int i = 0; i < n; i++)
      empty[i] = !count[i];
    
    return empty;
  }
  
  const bool is_empty(const int & pl_number)
  {
    assert(pl_number < n);
    return !count[pl_number];
  }

  const vector<bool> & is_full()
  {
    for (int i = 0; i < n; i++)
      full[i] = (count[i] == size);
    
    return full;
  }
  
  const bool is_full(const int & pl_number)
  {
    assert(pl_number < n);
    return (count[pl_number] == size);
  }

  const vector<flit_t> & data_out()
  {
    for (int i = 0; i < n; i++)
      out[i] = fifos[i].front();
      
    return out;  
  }
  
  const flit_t data_out(const int & pl_number)      //try to make all small functions inline
  {
    assert(pl_number < n);
    return fifos[pl_number].front();
  }

  LAG_input_trunk_tiny(string name_,
                  int n_ = 4,                    //number of fifos per trunk
                  int fifo_size_ = 8             //length of each fifo
                  ): name(name_), n(n_), size(fifo_size_), count(n_), fifos(n_),
                  allocated_pl_valid(n_), allocated_pl(n_), full(n_), empty(n_),
                  out(n_)
  {
    reset();
  }                  

private:

  int n;                              //number of physical links per trunk
  int size;                           //length of each fifo
  string name;                        //name of module
  
  vector<int> count;                  //count of each fifo capacity
  
  vector<deque<flit_t> > fifos;

  vector<bool> allocated_pl_valid;
  vector<int> allocated_pl;
  
  vector<bool> full;
  vector<bool> empty;
  
  vector<flit_t> out;

};


#endif