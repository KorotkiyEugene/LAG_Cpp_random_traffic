/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_TRAFFIC_SINK_H
#define LAG_TRAFFIC_SINK_H

#include "systemc.h"
#include "types.h"

class LAG_traffic_sink: public sc_module
{
  public:
  
  sc_in<bool> clk, rst_n;
  sc_in<my_vector<flit_t> > flit_in;
  sc_out<my_vector<bool> > cntrl_out; 
  
  SC_HAS_PROCESS(LAG_traffic_sink);
  
  LAG_traffic_sink(sc_module_name name_,
                    int x_dim_ = 4,           //mesh network size
                    int y_dim_ = 4,
                    int x_pos_ = 0,           //traffic src position in mesh network
                    int y_pos_ = 0,
                    int warmup_packets_ = 100,
                    int measurement_packets_ = 100,
                    int num_pls_ = 2,
                    int num_pls_on_exit_ = 2
                    ): sc_module(name_), x_dim(x_dim_), y_dim(y_dim_), x_pos(x_pos_), 
                      y_pos(y_pos_), warmup_packets(warmup_packets_), num_pls(num_pls_),
                      measurement_packets(measurement_packets_), num_pls_on_exit(num_pls_on_exit_),
                      expected_flit_id(num_pls_on_exit_), head_injection_time(num_pls_on_exit_) 
  {
    SC_METHOD(process);
    sensitive_pos << clk;
  }
  
  
  void process()
  {
    //in
    my_vector<flit_t> flit_in_i;
    
    //out
    my_vector<bool> cntrl_out_o(num_pls);
    
    if(!rst_n)
    {
      my_vector<bool> false_bools(num_pls, false);
    
      cntrl_out = false_bools;
      
      sys_time = 0;
      
      for (int i = 0; i < num_pls; i++)
      {
        expected_flit_id[i] = 1;
        head_injection_time[i] = 0;
        cntrl_out_o[i] = false;
      }
      
      cntrl_out.write(cntrl_out_o);
      
    }else
    {
      //reading and checking input signals
      flit_in_i = flit_in.read();
      sc_assert(flit_in_i.size() == num_pls);
      
      //processing input signals and generating output signals
      for (int i = 0; i < num_pls; i++)      //can be optimized - placed in one for cycle with the statistics counting
        if (flit_in_i[i].valid)
        {
          if (i < num_pls_on_exit)
          {
            cntrl_out_o[i] = true;
            
          }else
          {
            cout << "Error: Flit Channel ID is out-of-range for exit from network!" << endl;
            sc_stop();
          }
        } else
        {
          cntrl_out_o[i] = false;
        } 
      
      sys_time++;
      
      for (int i = 0; i < num_pls_on_exit; i++)
        if (flit_in_i[i].valid)
        {
          //
	        // check flit was destined for this node!
	        // 
          if ((flit_in_i[i].debug.x_dest != x_pos) || (flit_in_i[i].debug.y_dest != y_pos))
          {
            cout << "Error: Flit arrived at wrong destination!" << endl;
	          sc_stop();
          }
          
          //
	        // check flit didn't originate at this node
	        //
	        if ((flit_in_i[i].debug.x_dest == flit_in_i[i].debug.x_src) && (flit_in_i[i].debug.y_dest == flit_in_i[i].debug.y_src)) 
	        {
            cout << "Error: Received flit originated from this node?" << endl;
            sc_stop();
	        }
          
          //
	        // check flits for each packet are received in order
	        //
          if (flit_in_i[i].debug.flit_id != expected_flit_id[i]) 
          {
            cout << "Error: Out of sequence flit received? (packet generated at ";
            cout << flit_in_i[i].debug.x_src << " " << flit_in_i[i].debug.y_src << ")" << endl;
            cout << "-- Flit ID = " << flit_in_i[i].debug.flit_id << " Expected = " << expected_flit_id[i] << endl;
            cout << "-- Packet ID = " << flit_in_i[i].debug.packet_id << endl;
            sc_stop();
          }
          
          expected_flit_id[i]++;
          
          // #####################################################################
	        // Head of new packet has arrived
	        // #####################################################################
          
          if (flit_in_i[i].debug.flit_id == 1)
          {
            head_injection_time[i] = flit_in_i[i].debug.inject_time;
          }
	    
	    	  // count all flits received in measurement period
	        if ((flit_in_i[i].debug.packet_id > warmup_packets) && (stats.measure_start == -1))  
            stats.measure_start = sys_time;
	    
          if (flit_in_i[i].debug.packet_id <= warmup_packets + measurement_packets)
            if (stats.measure_start != -1) stats.rec_flits_count++;
	      
          // #####################################################################
          // Tail of packet has arrived
          // Remember, latency = (tail arrival time) - (head injection time)
          // #####################################################################
	    
          if (flit_in_i[i].tail)
          {
            expected_flit_id[i] = 1;
          
            if ((flit_in_i[i].debug.packet_id > warmup_packets) && \
                (flit_in_i[i].debug.packet_id <= warmup_packets + measurement_packets))
            {
              stats.rec_packets_count++;
              
		          stats.measure_end = sys_time;
            
              //
              // gather latency stats.
              //
              
              latency = sys_time - head_injection_time[i]; 
		          stats.total_latency = stats.total_latency + latency;

		          stats.min_latency = std::min(stats.min_latency, latency);
		          stats.max_latency = std::max(stats.max_latency, latency);
		  
              //
              // display progress estimate
              //
		          if (stats.rec_packets_count % (measurement_packets/100) == 0)
              { 
		            cout << sc_time_stamp() << " sink(" << x_pos << "," << y_pos << "): " << (double)stats.rec_packets_count*100 / (double)measurement_packets;
                cout << " % complete (this packet's latency was " << latency << endl;
              }
              
              stats.lat_freq[std::min(latency, 100)]++;
              		  
            }
          
          }
	    
        }
      
      //writing output signals
      cntrl_out.write(cntrl_out_o);
      
    }
    
  }
  
  
  sim_stats_t & get_stats()
  {
    return stats;
  }
  
  private:

  sim_stats_t stats;
  
  int x_dim;
  int y_dim;
  int x_pos;
  int y_pos;
  int warmup_packets;
  int measurement_packets;
  int num_pls;
  int num_pls_on_exit;
  
  vector<int> expected_flit_id;       //dim. num_pls_on_exit, not num_pls
  vector<int> head_injection_time;    //dim. num_pls_on_exit, not num_pls
  int   latency, sys_time;
  
};

#endif