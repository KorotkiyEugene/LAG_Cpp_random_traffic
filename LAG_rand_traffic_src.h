/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_RAND_TRAFFIC_SRC_H
#define LAG_RAND_TRAFFIC_SRC_H

#include "systemc.h"
#include "types.h"
#include "LAG_fifo.h"
#include "LAG_flow_control.h"
#include "LAG_route.h"
#include "parameters.h"

class LAG_rand_traffic_src: public sc_module
{
  public:

  sc_in<bool>                 clk, rst_n;
  sc_in<my_vector<bool> >     cntrl_in;
  sc_out<my_vector<flit_t> >  flit_out;

  SC_HAS_PROCESS(LAG_rand_traffic_src);

  //in this configuration we ignore parameter router_num_pls_on_entry - data injects to the network only through one (zero) channel
  LAG_rand_traffic_src(sc_module_name name_,
                        int pl_num_ = 2,
                        int x_dim_ = 4,           //mesh network size
                        int y_dim_ = 4,
                        int x_pos_ = 0,           //traffic src position in mesh network
                        int y_pos_ =0,
                        int packet_length_ = 4,
                        double rate_ = 0.3           //flits injection rate flit/cycle
                        ): sc_module(name_), pl_num(pl_num_), x_dim(x_dim_),
                        y_dim(y_dim_), x_pos(x_pos_), y_pos(y_pos_), 
                        packet_length(packet_length_), rate(rate_)
  {
    fifo = new LAG_fifo("TRAFFIC_GEN_FIFO", 2 + packet_length*4);
    fifo -> clk(clk);
    fifo -> rst_n(rst_n);
    fifo -> data_in(fifo_in);
    fifo -> data_out(fifo_out);
    fifo -> full(fifo_full);
    fifo -> empty(fifo_empty);
    fifo -> push(fifo_push);
    fifo -> pop(fifo_pop);
    
    flow_control = new LAG_flow_control("TRAFFIC_GEN_FLOW_CONTROL", pl_num, router_buf_len);
    flow_control -> clk(clk);
    flow_control -> rst_n(rst_n);
    flow_control -> channel_credits(cntrl_in);
    flow_control -> flits_valid(flits_valid);
    flow_control -> pl_status(out_blocked);
    
    SC_METHOD(main_method);
    sensitive_pos << clk;
    
    SC_METHOD(out_driver);
    sensitive << fifo_out << fifo_empty << out_blocked;
    
    tmp = router_buf_len;
    
  }
  
  
  void main_method()
  {
    //in
    bool fifo_full_i;
    bool fifo_empty_i;
    //my_vector<bool> out_blocked_i;
    
    if(!rst_n)
    {
      //init vars
      prob = 10000*rate/packet_length;
      sys_time = 0;
      i_time = 0;
      flit_count = 0;
      inject_count = 0;
      flits_buffered = 0;
      flits_sent = 0;
      fifo_ready = 1;
      injecting_packet = 0; 
      seed = x_pos*50 + y_pos;
      srand(seed);
      
    }else             
    {
      /*if (cntrl_in.read()[0] && x_pos == 0 && y_pos == 1)
      {
        cout << sc_time_stamp() << "+++++++++++++++++++++++++++ Credit come ; Blocked = " << out_blocked.read()[0] << " Count = " << ++tmp << endl;
      }
      
      if (flit_out.read()[0].valid && x_pos == 0 && y_pos == 1)
      {
        cout << sc_time_stamp() << "++++ Flit out ---- Blocked = " << out_blocked.read()[0] << " Count = " << --tmp << endl;
        
      }  */
      
      //reading and checking input signals
      fifo_full_i = fifo_full.read();
      //out_blocked_i = out_blocked.read();
      //sc_assert(out_blocked_i.size() == pl_num);
      
      if(fifo_pop.read())       //if (!fifo_empty_i && !out_blocked_i[0]) 
        flits_sent++; 
        
      if (fifo_push.read()) flits_buffered++;  
    
      //
      // start buffering next flit when there is room in FIFO
      //
      if ( ( flits_buffered - flits_sent ) <= packet_length ) 
	     fifo_ready = 1;

      if (fifo_ready)
	     while ((i_time!=sys_time)&&(injecting_packet==0)) 
	     { 
	       // **********************************************************
	       // Random Injection Process
	       // **********************************************************
          
          if (rand()%10000 <= prob)
		        injecting_packet++;
	        
	       i_time++;
      }
	 
      if (injecting_packet && !fifo_ready)
	     sc_assert(flit_count == 0);
	 
	    if ( !(fifo_ready && injecting_packet) )
	    {
        fifo_push.write(false);
        
      }else
	    {

        // random source continues as we buffer flits in FIFO 
          
        if (rand()%10000 <= prob) 
		      injecting_packet++;
         
	      i_time++;
	    
        flit_count++;
	    
        fifo_push.write(true);

        //
        // Send Head Flit
        //
        if (flit_count != 1)
        {
          tmp_flit.head = false;
        }else
        {
	     
	       tmp_flit = zero_flit;
	       
	       inject_count++;
	       
	       //
	       // set random displacement to random destination
	       //
	       x_dest = rand() % x_dim;
	       y_dest = rand() % y_dim;
	       
         while ( (x_pos == x_dest) && (y_pos == y_dest) )
         {
            // don't send to self...
	         x_dest = rand() % x_dim;
	         y_dest = rand() % y_dim;
	       }
	       
	       tmp_flit.route.dx = x_dest - x_pos;
	       tmp_flit.route.dy = y_dest - y_pos;
	       
	       tmp_flit.head = true;
	      
	       tmp_flit.debug.x_dest = x_dest;
	       tmp_flit.debug.y_dest = y_dest;
	       tmp_flit.debug.x_src = x_pos;
	       tmp_flit.debug.y_src = y_pos;
	       
        }
        
        tmp_flit.debug.inject_time = i_time;
        tmp_flit.debug.flit_id = flit_count;
        tmp_flit.debug.packet_id = inject_count;
        
        if (flit_count == packet_length)
        {
          tmp_flit.tail = true;
          injecting_packet--;
          flit_count = 0;
          fifo_ready = 0;
        }
        
      } //if (fifo_ready && injecting_packet)
	 
	    sys_time++;
	    fifo_in.write(tmp_flit);
	     
    } //if(!rst_n)
  }
  
  
  void out_driver()
  {
    //in
    flit_t  fifo_out_i;
    my_vector<bool> out_blocked_i;
    
    //out
    my_vector<flit_t>   flit_out_o(pl_num);
    my_vector<bool>     flits_valid_o(pl_num);
    
    if(!rst_n)
    {
      my_vector<flit_t> zero_flits(pl_num);
      my_vector<bool>   false_bools(pl_num, false); 
      
      flit_out = zero_flits;
      flits_valid = false_bools;
    
    }else
    {
      //reading and checking input signals
      fifo_out_i = fifo_out.read();
      out_blocked_i = out_blocked.read();
      sc_assert(out_blocked_i.size() == pl_num);
      
      //processing input signals and generating output signals
      flit_out_o[0] = fifo_out_i.head ? route(fifo_out_i) : fifo_out_i;
      flit_out_o[0].valid = !fifo_empty.read() && !out_blocked_i[0];
      flits_valid_o[0] = flit_out_o[0].valid;
      
      /*if (flit_out_o[0].valid && x_pos == 1 && y_pos == 0 && flit_out_o[0].debug.flit_id == 11)
      {
        cout << "!!!!!!!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        cout << "Out Blocked = " << out_blocked_i[0] << endl;
      } */
      

        
      //writing output signals
      if (flit_out_o[0].valid)
        fifo_pop.write(true);
      else
        fifo_pop.write(false);
        
      flit_out.write(flit_out_o);
      flits_valid.write(flits_valid_o);
    }  
  
  }
  
  
  ~LAG_rand_traffic_src()
  {
    if(fifo) delete fifo;
    if(flow_control) delete flow_control;
  }

  private:

  int tmp;

  int pl_num;
  int x_dim;
  int y_dim;
  int x_pos;
  int y_pos;
  int packet_length;
  double rate;   
  
  double prob;                   //probability of packet creation
  long long int sys_time;
  long long int i_time;       //packet injection time
  int flit_count;
  int inject_count;
  int flits_buffered;
  int flits_sent;
  bool fifo_ready;
  int injecting_packet; 
  int seed; //rng seed value 
  int x_dest, y_dest;
  
  flit_t tmp_flit;
  const flit_t zero_flit; //all member vars are zeros after creation 
  
  LAG_fifo*             fifo;
  LAG_flow_control*     flow_control;
  
  //fifo signals
  sc_signal<flit_t> fifo_in, fifo_out;
  sc_signal<bool> fifo_full, fifo_empty, fifo_push, fifo_pop;
  
  //flow control signals
  sc_signal<my_vector<bool> > flits_valid;
  sc_signal<my_vector<bool> > out_blocked;
  
};


#endif