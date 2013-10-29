/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_FIFO_H
#define LAG_FIFO_H

#include "systemc.h"
#include <queue>

using std::deque;

class LAG_fifo: public sc_module 
{
public:
  sc_in <bool> clk, rst_n, push, pop;
  sc_in <flit_t> data_in;
  sc_out <bool> full, empty;
  sc_out <flit_t> data_out;

  SC_HAS_PROCESS(LAG_fifo);

  void process()
  {
    if (!rst_n){
      count = 0;
      empty = true;
      full = false;
      fifo.clear();
    }else {
      add = push && !pop;
      sub = pop && !push;
    
      if (add && sub)
      {
        cout <<"FIFO Error --> add and sub cant be true simultaneously";
        sc_stop();
      }
    
      if (add && count == size)
      {
        cout << "FIFO Error --> fifo overflow; count = " << count << " size = " << size << endl;
        sc_stop();
      }
    
      if (sub && count == 0)
      {
        cout << "FIFO Error --> fifo underflow";
        sc_stop();
      }
    
      if (add) count++;
      if (sub) count--;
    
      if (push) fifo.push_back(data_in);
      if (pop) fifo.pop_front();
    
      data_out = count ? fifo.front() : zero_flit;  
    
      full = (count == size);
      empty = !count;  
    }
};


  LAG_fifo(sc_module_name name_, 
            int size_ = 8, 
            bool trace = false, 
            sc_trace_file* trace_file_ptr = NULL): sc_module(name_), size(size_) 
  {
    SC_METHOD(process);
    sensitive_pos << clk;
    count = 0;
    empty.initialize(true);
    full.initialize(false);
    
    if (trace && trace_file_ptr != NULL)
    {
      sc_trace(trace_file_ptr, clk , "clk" );
      sc_trace(trace_file_ptr, rst_n , "rst_n" );
      sc_trace(trace_file_ptr, push , "push" );
      sc_trace(trace_file_ptr, pop , "pop" );
      sc_trace(trace_file_ptr, full , "full" );     
      sc_trace(trace_file_ptr, empty , "empty" );
      sc_trace(trace_file_ptr, data_in , "data_in" );
      sc_trace(trace_file_ptr, data_out , "data_out" );
    } 
  }

private:  
  int count;
  int size;
  bool add, sub;
  deque<flit_t> fifo;  
  const flit_t zero_flit;
};

#endif // LAG_FIFO_H
