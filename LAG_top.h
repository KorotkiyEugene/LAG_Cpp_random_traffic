/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_TOP_H
#define LAG_TOP_H

#include "systemc.h"
#include "LAG_mesh_network_tb.h"

class LAG_top: sc_module
{
  public:
  sc_signal<bool> rst_n;
  sc_clock clk; 

  SC_HAS_PROCESS(LAG_top);

  LAG_top(sc_module_name name_) : sc_module(name_)
  {
    mesh_tb = new LAG_mesh_network_tb("MESH_TB");
    mesh_tb -> clk(clk);
    mesh_tb -> rst_n(rst_n);
    
    SC_THREAD(my_thread);
  }
  
  
  void my_thread()
  {
    rst_n = false;  
    wait(10, SC_NS);
    rst_n = true;
  }
  
  
  ~LAG_top()
  {
    if (mesh_tb) delete mesh_tb;
  }
 
  LAG_mesh_network_tb* mesh_tb;

};

#endif