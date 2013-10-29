/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_ROUTER_TINY_H
#define LAG_ROUTER_TINY_H

#include "systemc.h"
#include "types.h"
#include "LAG_crossbar_tiny.h"
#include "LAG_flow_control_tiny.h"
#include "LAG_input_trunk_tiny.h"
#include "LAG_pl_allocator_tiny.h"
#include "LAG_pl_free_pool_tiny.h"
#include "LAG_route.h"

using std::vector;
using std::stringstream;
using std::string;

class LAG_router_tiny: public sc_module
{
public:

  sc_in<bool> rst_n, clk;
  sc_in<my_vector<flit_t> >* flit_in;          //using of 1 dim. array to represent 2 dim. array
  sc_in<my_vector<bool> >* cntrl_in;
  sc_out<my_vector<bool> >* cntrl_out;
  sc_out<my_vector<flit_t> >* flit_out;        //using of 1 dim. array to represent 2 dim. array
  
  SC_HAS_PROCESS(LAG_router_tiny);

  LAG_router_tiny(sc_module_name name_, 
              int x_ = 0,                //position x in mesh network
              int y_ = 0,                //position y in mesh network
              int buf_len_ = 4,           //length of fifos in phusical links
              int trunk_num_ = 5,         //number of trunks per router
              int pl_num_ = 2,             //number of physical links per router
              int num_pls_on_entry_ = 1,
              int num_pls_on_exit_ = 2
              ) : sc_module(name_), x(x_), y(y_), buf_len(buf_len_), trunk_num(trunk_num_),
              pl_num(pl_num_), num_pls_on_entry(num_pls_on_entry_), num_pls_on_exit(num_pls_on_exit_),
              flow_control(trunk_num_), input_trunk(trunk_num_), pl_free_pool(trunk_num_),
              expected_flit_id(trunk_num_*pl_num_), input_trunk_pop(trunk_num_), 
              flits_out_valid(trunk_num_), flits_out_tail(trunk_num_),
              cntrl_out_o(pl_num_), flit_out_o(trunk_num_), false_bools(pl_num, false), 
              zero_flits(pl_num), zero_flits_2(trunk_num_), false_bools_2(trunk_num_),
              alloc_req_i(trunk_num_), alloc_out_trunk_i(trunk_num_), alloc_pl_status_i(trunk_num_),
              alloc_pl_new_o(trunk_num_), alloc_pl_new_valid_o(trunk_num_), alloc_pl_allocated_o(trunk_num_) 
  {
    //allocating dynamically ports
    flit_in = new sc_in<my_vector<flit_t> >[trunk_num];
    flit_out = new sc_out<my_vector<flit_t> >[trunk_num];
    cntrl_in = new sc_in<my_vector<bool> >[trunk_num];
    cntrl_out = new sc_out<my_vector<bool> >[trunk_num];
    
    //allocating dynamically modules
    crossbar = new LAG_crossbar_tiny("CROSSBAR", trunk_num, pl_num);
    pl_allocator = new LAG_pl_allocator_tiny("PL_ALLOCATOR", trunk_num, pl_num);
    
    for (int i = 0 ; i < trunk_num; i++)
    {     
      stringstream ss;
      ss << i;
      
      string flow_control_name = "FLOW_CONTROL" + ss.str();
      string input_trunk_name = "INPUT_TRUNK" + ss.str();
      string pl_free_pool_name = "PL_FREE_POOL" + ss.str();
      
      flow_control[i] = new LAG_flow_control_tiny(flow_control_name.c_str(), pl_num, buf_len);  
      input_trunk[i] = new LAG_input_trunk_tiny(input_trunk_name.c_str(), pl_num, buf_len);
      
      input_trunk_pop[i].resize(pl_num);
      flits_out_valid[i].resize(pl_num);
      flits_out_tail[i].resize(pl_num);
      flit_out_o[i].resize(pl_num);
      zero_flits_2[i].resize(pl_num);
      false_bools_2[i].resize(pl_num);
      alloc_req_i[i].resize(pl_num);
      alloc_out_trunk_i[i].resize(pl_num);
      alloc_pl_status_i[i].resize(pl_num);
      alloc_pl_new_o[i].resize(pl_num);
      alloc_pl_new_valid_o[i].resize(pl_num);            
      alloc_pl_allocated_o[i].resize(pl_num);
      
      if(i == TILE)
        pl_free_pool[i] = new LAG_pl_free_pool_tiny(pl_free_pool_name.c_str(), pl_num, num_pls_on_exit);
      else
        pl_free_pool[i] = new LAG_pl_free_pool_tiny(pl_free_pool_name.c_str(), pl_num, pl_num);
          
    } 
    
    //SC_METHOD(in_out_verbose);
    //sensitive_pos << clk;
    
    SC_METHOD(in_out_flit_sequence_check);
    sensitive_pos << clk;
    
    SC_METHOD(main_process);
    sensitive_pos << clk;
    
  }
  
  
  void main_process()
  {
    int i,j,k,l;
    
    if (!rst_n)
    { 
      pl_allocator -> reset();
      
      for (i = 0; i < trunk_num; i++)
      {
        input_trunk[i] -> reset();
        flow_control[i] -> reset();
        pl_free_pool[i] -> reset();
        
        flit_out[i].write(zero_flits);
        cntrl_out[i].write(false_bools); 
      }
      
      for (i = 0; i < trunk_num; i++)
        input_trunk_pop[i] = false_bools;
      
    } else
    {
      
      for (i = 0; i < trunk_num; i++)
      {
        input_trunk[i] -> pop(input_trunk_pop[i]); //removing flits, which were just transmitted, from input fifos
        
        pl_free_pool[i] -> process_tail_flits(flits_out_tail[i], flits_out_valid[i]);
        
        for (j = 0; j < pl_num; j++)
        {     
          k = alloc_out_trunk_i[i][j];
          l = input_trunk[i] -> get_allocated_pl(j);
          input_trunk_pop[i][j] = !(input_trunk[i] -> is_empty(j)) && \
                                (input_trunk[i] -> is_allocated_pl_valid(j)) && \
                                !(flow_control[k] -> is_output_blocked(l));
        
        }
      }
      
      for (i = 0; i < trunk_num; i++)
      {
        for (j = 0; j < pl_num; j++)
          cntrl_out_o[j] = input_trunk_pop[i][j];
        
        cntrl_out[i].write(cntrl_out_o);   //sending credits to the upstream router, which is connected to the i-th input port
      }
      
      //Imitation of crossbar start
      flit_out_o = zero_flits_2;
      flits_out_valid = false_bools_2;
      flits_out_tail = false_bools_2;
      
      for(i = 0; i < trunk_num; i++)
        for(j = 0; j < pl_num; j++) 
        {   
          k = alloc_out_trunk_i[i][j]; // k - output trunk
          l = input_trunk[i] -> get_allocated_pl(j);  // l - output link
            
          if(input_trunk[i] -> is_allocated_pl_valid(j))
          {         
            tmp_flit = input_trunk[i] -> data_out(j);
            flit_out_o[k][l] = tmp_flit.head ? route(tmp_flit) : tmp_flit;  
      
            flit_out_o[k][l].valid = input_trunk_pop[i][j];
            flits_out_valid[k][l] = flit_out_o[k][l].valid;
            if (flit_out_o[k][l].tail)
              flits_out_tail[k][l] = true;      
          }  
        }  
      
      //imitation of crossbar end
      
      for (i = 0; i < trunk_num; i++)
      {
        flit_out[i].write(flit_out_o[i]);   //writing flits to output ports
        flow_control[i] -> process_flits_out_valid(flits_out_valid[i]); // i index output trunk which corresponded to flow control module
        
        vector<bool> push(pl_num, false);
        my_vector<flit_t> data_in = flit_in[i].read();
        sc_assert(data_in.size() == pl_num);
        
        for (j = 0; j < pl_num; j++)
          push[j] = data_in[j].valid;
      
        input_trunk[i] -> push(data_in, push);
      }
      
      for (i = 0; i < trunk_num; i++)
      {
        for (j = 0; j < pl_num; j++)
        {
          alloc_req_i[i][j] = !(input_trunk[i] -> is_empty(j)) && !(input_trunk[i] -> is_allocated_pl_valid(j));
          
          tmp_flit = input_trunk[i] -> data_out(j);
          
          if(tmp_flit.head)
            alloc_out_trunk_i[i][j] = tmp_flit.route.output_trunk;
          
        }
        
        alloc_pl_status_i[i] = pl_free_pool[i] -> get_pl_alloc_status();
        
      }
      
      //allocating physical links
      pl_allocator -> allocate(alloc_req_i, 
                                alloc_out_trunk_i, 
                                alloc_pl_status_i, 
                                alloc_pl_new_o, 
                                alloc_pl_new_valid_o, 
                                alloc_pl_allocated_o
                                );
      
        
      for (i = 0; i < trunk_num; i++)
      { 
        input_trunk[i] -> set_allocated_pl_new(alloc_pl_new_o[i], alloc_pl_new_valid_o[i]);
        pl_free_pool[i] -> process_pl_allocated(alloc_pl_allocated_o[i]); 
      }
    
      //taking into account incoming credits
      for (i = 0; i < trunk_num; i++)
      {
        cntrl_in_i = cntrl_in[i].read();  //reading incoming credits
        
        sc_assert(cntrl_in_i.size() == pl_num);
      
        flow_control[i] -> process_input_credits(cntrl_in_i);
        
      }
        
    }
  }
  
  
  void in_out_verbose()
  {
    my_vector<flit_t> trunk_data; 
    
    if (!rst_n)
    {
    
    } else
    {
      for (int i = 0; i < trunk_num; i++)
      {
        trunk_data = flit_in[i].read();     //reading from port i'th input trunk  
        
        sc_assert(trunk_data.size() == pl_num);
        
        for (int j = 0; j < pl_num; j++)
          if(trunk_data[j].valid)
          {
            cout << "+------------------------------------------------------" << endl;
            cout << sc_time_stamp() << " Router(" << x << ", " << y;
            cout << ", In trunk = " << i << " physical link = " << j << "):" << endl;
            cout << "Flit " << trunk_data[j].debug.flit_id;
            cout << ", from (" << trunk_data[j].debug.x_src << "," << trunk_data[j].debug.y_src << ")";
            cout << " for (" << trunk_data[j].debug.x_dest << "," << trunk_data[j].debug.y_dest << ")" << endl;   
            cout << "Tail = " << trunk_data[j].tail << endl;     
          }
        
        trunk_data = flit_out[i].read();
        
        sc_assert(trunk_data.size() == pl_num);
        
        for (int j = 0; j < pl_num; j++)
          if(trunk_data[j].valid)
          {
            cout << "+------------------------------------------------------" << endl;
            cout << sc_time_stamp() << " Router(" << x << ", " << y;
            cout << ", Out trunk = " << i << " physical link = " << j << "):" << endl;
            cout << "Flit " << trunk_data[j].debug.flit_id;
            cout << ", from (" << trunk_data[j].debug.x_src << "," << trunk_data[j].debug.y_src << ")";
            cout << " for (" << trunk_data[j].debug.x_dest << "," << trunk_data[j].debug.y_dest << ")" << endl;      
            cout << "Tail = " << trunk_data[j].tail << endl;  
            
          }
              
      }
    } 
  }
  
  void in_out_flit_sequence_check()
  {
    my_vector<flit_t> trunk_data;
    
    if(!rst_n)
    {
    
      for (int i = 0; i < trunk_num; i++)
        for (int j = 0; j < pl_num; j++)
          expected_flit_id[i*pl_num + j] = 1;   
      
    } else
    {
      /*if (cntrl_in[2].read()[0] && x == 0 && y == 0)
      {
        cout << sc_time_stamp() << "+++++++++++++++++++++++++++ Credit come ; " << " Count = " << ++tmp << "  PL stat = " << fc_pl_status[2].read()[0] << " pop = " << it_pop[1].read()[0] << endl;
      }
      
      if (flit_out[2].read()[0].valid && x == 0 && y == 0)
      {
        cout << sc_time_stamp() << "++++ Flit out ---- " << " Count = " << --tmp << "  PL stat = " << fc_pl_status[2].read()[0] << " pop = " << it_pop[1].read()[0] << endl;
        
      } */
      
      for (int i = 0; i < trunk_num; i++)
      {
        trunk_data = flit_in[i].read();
        
        sc_assert(trunk_data.size() == pl_num);
        
        for (int j = 0; j < pl_num; j++)
        {
          if (trunk_data[j].valid)
          {
            if(trunk_data[j].debug.flit_id != expected_flit_id[i*pl_num + j])
            {
              cout << "+------------------------------------------------------" << endl;
              cout << "Error: x=" << x << ", y=" << y << ", trunk=" << i;
              cout << ", physical link=" << j << ", time=" << sc_time_stamp() << endl;
              cout << "Flits out of sequence: flit_id=" << trunk_data[j].debug.flit_id;
              cout << ", expected=" << expected_flit_id[i*pl_num + j] << endl;
              cout << "Tail = " << trunk_data[j].tail << endl;
              cout << ", from (" << trunk_data[j].debug.x_src << "," << trunk_data[j].debug.y_src << ")";
              cout << " for (" << trunk_data[j].debug.x_dest << "," << trunk_data[j].debug.y_dest << ")" << endl;
            }
          }
          
          if(trunk_data[j].valid)
          {
            if(trunk_data[j].tail)
              expected_flit_id[i*pl_num + j] = 1;  
            else  
              expected_flit_id[i*pl_num + j]++;
          }  
        }
        
      }
    }  
  }
  
  
  ~LAG_router_tiny()
  {
    for (int i = 0 ; i < trunk_num; i++)
    {
      if(flow_control[i]) delete flow_control[i];
      if(input_trunk[i]) delete input_trunk[i];
      if(pl_free_pool[i]) delete pl_free_pool[i];
    }
    
    if(crossbar) delete crossbar;
    if(pl_allocator) delete pl_allocator;
    
    if(flit_in) delete [] flit_in;
    if(flit_out) delete [] flit_out;
    if(cntrl_in) delete [] cntrl_in;
    if(cntrl_out) delete [] cntrl_out;
  }

private:

  int x,y; //position in mesh network
  int buf_len, trunk_num, pl_num, num_pls_on_entry, num_pls_on_exit;
  
  LAG_crossbar_tiny*       crossbar;
  LAG_pl_allocator_tiny*   pl_allocator;
  
  vector<LAG_flow_control_tiny*>   flow_control;
  vector<LAG_input_trunk_tiny*>    input_trunk;
  vector<LAG_pl_free_pool_tiny*>   pl_free_pool;
  
  vector<int> expected_flit_id;         //2 dim array as 1 dim. array
  
  vector<vector<bool> > input_trunk_pop;
  
  vector<vector<bool> > flits_out_valid;
  vector<vector<bool> > flits_out_tail;
  
  vector<bool> cntrl_in_i;
  
  my_vector<bool> cntrl_out_o;
  my_vector<my_vector<flit_t> > flit_out_o;
  
  flit_t zero_flit;
  flit_t tmp_flit;
  my_vector<flit_t> zero_flits;
  my_vector<my_vector<flit_t> > zero_flits_2;
  my_vector<bool>   false_bools;
  
  vector<vector<bool> > false_bools_2;
  
  vector<vector<bool> > alloc_req_i;
  vector<vector<int> > alloc_out_trunk_i;
  vector<vector<bool> > alloc_pl_status_i;
  vector<vector<int> > alloc_pl_new_o;
  vector<vector<bool> > alloc_pl_new_valid_o;
  vector<vector<bool> > alloc_pl_allocated_o;
      
};

#endif