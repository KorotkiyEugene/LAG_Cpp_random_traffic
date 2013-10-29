/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_FLOW_CONTROL
#define LAG_FLOW_CONTROL

#include "systemc.h"
#include "types.h"

class LAG_flow_control: public sc_module
{
public:

  sc_in<bool> clk, rst_n;
  sc_in<my_vector<bool> > channel_credits;
  sc_in<my_vector<bool> > flits_valid;    // when flits_valid[pl_number] = 1,  flit leaves router through this pl during this clock cycle
  sc_out<my_vector<bool> > pl_status;     //pl_status[pl_number] = 1 if pl is blocked (downstream fifo is full) 
                                          //i.e. credit count for this link == 0
  
  SC_HAS_PROCESS(LAG_flow_control);
  
  void process()
  {
    if (!rst_n) {
    
      for (int i=0; i < num_pls; i++)
        counter[i] = init_credits;
    
      my_vector<bool> false_bools(num_pls, false);
      
      pl_status = false_bools;
      
    } else 
    {
      channel_credits_i = channel_credits.read();
      flits_valid_i = flits_valid.read();
    
      sc_assert(channel_credits_i.size() == num_pls);
      sc_assert(flits_valid_i.size() == num_pls);
      
      for (int i=0; i < num_pls; i++) 
      {
      
        if (flits_valid_i[i])
          counter[i]--;
        
        if (channel_credits_i[i])
          counter[i]++;
        
        pl_status_o[i] = !counter[i];  
      }
      
      pl_status.write(pl_status_o);
          
    }
  }
  
  LAG_flow_control(sc_module_name name_,
                   int num_pls_ = 2,  //number of physical links per trunk
                   int init_credits_ = 4
                  ) : sc_module(name_), num_pls(num_pls_), init_credits(init_credits_),
                  counter(num_pls_), pl_status_o(num_pls_)
  { 
    SC_METHOD(process);
    sensitive_pos << clk;
  }

private:

  int num_pls;
  int init_credits;
  my_vector<int> counter;
  my_vector<bool> channel_credits_i, flits_valid_i;
  my_vector<bool> pl_status_o;
  
};

#endif