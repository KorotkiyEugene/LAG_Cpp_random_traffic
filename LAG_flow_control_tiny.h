/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_FLOW_CONTROL_TINY_H
#define LAG_FLOW_CONTROL_TINY_H

#include "systemc.h"
#include "types.h"

class LAG_flow_control_tiny
{
public:

  // when flits_valid[pl_number] = 1,  flit leaves router through this pl during this clock cycle
  //pl_status[pl_number] = 1 if pl is blocked (downstream fifo is full)
  //i.e. credit count for this link == 0
  
  void reset()
  {
    for (int i = 0; i < num_pls; i++)
      counter[i] = init_credits;
    
    vector<bool> false_bools(num_pls, false);
      
    pl_status = false_bools;
  }
  
  const vector<bool> & get_pl_fc_status() const
  {   
    return pl_status;
  }
  
  const bool is_output_blocked(const int & pl_number) const
  {
    assert(pl_number < num_pls);
    return pl_status[pl_number];
  }
  
  void process_input_credits(const vector<bool> & input_credits)
  {
    assert(input_credits.size() == num_pls);
    
    for (int i = 0; i < num_pls; i++)
      if (input_credits[i])
        if (counter[i] < init_credits)
        {
          counter[i]++;
          pl_status[i] = !counter[i];
        }
        else
        {
          cout << sc_time_stamp() << " ERROR in module " << name << " Credit counter " << i << " overflow" << endl;
          sc_stop();
        }
  }
  
  void process_flits_out_valid(const vector<bool> & flits_out_valid)
  {
    assert(flits_out_valid.size() == num_pls);
    
    for (int i = 0; i < num_pls; i++)
      if (flits_out_valid[i])
        if (counter[i] > 0)
        {
          counter[i]--;
          pl_status[i] = !counter[i];
        }  
        else
        {
          cout << sc_time_stamp() << " ERROR in module " << name << " Credit counter " << i << " underflow" << endl;
          sc_stop();
        }  
  }
  
  LAG_flow_control_tiny(string name_,
                   int num_pls_ = 2,  //number of physical links per trunk
                   int init_credits_ = 4
                  ) : name(name_), num_pls(num_pls_), init_credits(init_credits_),
                  counter(num_pls_), pl_status(num_pls_)
  { 
    reset();  
  }

private:

  string name;
  
  int num_pls;
  int init_credits;
  vector<int> counter;
  vector<bool> pl_status;
  
};

#endif