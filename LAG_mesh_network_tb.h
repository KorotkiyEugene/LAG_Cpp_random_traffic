/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_MESH_NETWORK_TB_H
#define LAG_MESH_NETWORK_TB_H

#include "LAG_router_tiny.h"
#include "LAG_traffic_sink.h"
#include "LAG_rand_traffic_src.h"

using std::stringstream;

static const int total_packets = sim_measurement_packets*network_x*network_y;

class LAG_mesh_network_tb: sc_module
{
  public:
  
  sc_in<bool> clk;
  sc_in<bool> rst_n;
  
  SC_HAS_PROCESS(LAG_mesh_network_tb);
  
  LAG_mesh_network_tb(sc_module_name name_):sc_module(name_), routers(network_y*network_x),
                                            traffic_sinks(network_y*network_x), traffic_sources(network_y*network_x),
                                            total_lat_freq(101)
  {
    startup_message = false;
    reset_complete = false;
    
    traf_sink_flit = new sc_signal<my_vector<flit_t> >[network_y*network_x];
    traf_src_flit = new sc_signal<my_vector<flit_t> >[network_y*network_x];
    traf_sink_control = new sc_signal<my_vector<bool> >[network_y*network_x];
    traf_src_control = new sc_signal<my_vector<bool> >[network_y*network_x];  
  
    dummy_in_flit = new sc_signal<my_vector<flit_t> >[2*network_y + 2*network_x];
    dummy_out_flit = new sc_signal<my_vector<flit_t> >[2*network_y + 2*network_x];
    dummy_in_control = new sc_signal<my_vector<bool> >[2*network_y + 2*network_x];
    dummy_out_control = new sc_signal<my_vector<bool> >[2*network_y + 2*network_x];
  
    between_router_flit = new sc_signal<my_vector<flit_t> >[2*(network_x - 1)*network_y + 2*(network_y - 1)*network_x];
    between_router_control = new sc_signal<my_vector<bool> >[2*(network_x - 1)*network_y + 2*(network_y - 1)*network_x];
    
    int k = 0;
    int l = 0;
    
    for (int y = 0; y < network_y; y++)
    {
      for (int x = 0; x < network_x; x++)
      {
        stringstream s_router, s_src, s_sink;
        
        //creating router at xy position
        s_router << "ROUTER" << x << y;
        routers[network_x*y + x] = new LAG_router_tiny(s_router.str().c_str(), 
                                                  x,
                                                  y,
                                                  router_buf_len,
                                                  router_trunk_num,
                                                  router_pl_num,
                                                  router_num_pls_on_entry,
                                                  router_num_pls_on_exit);
        
        //creating traffic source at xy position                                          
        s_src << "RANDOM_TRAFFIC_SRC" << x << y;
        
        traffic_sources[network_x*y + x] = new LAG_rand_traffic_src(s_src.str().c_str(),
                                                                    router_pl_num,
                                                                    network_x,
                                                                    network_y,
                                                                    x,
                                                                    y,
                                                                    sim_packet_length,
                                                                    sim_injection_rate);
        
        //creating traffic sink at xy position                                                            
        s_sink << "TRAFFIC_SINK" << x << y;
        traffic_sinks[network_x*y + x] = new LAG_traffic_sink(s_sink.str().c_str(),
                                                              network_x,
                                                              network_y,
                                                              x,
                                                              y,
                                                              sim_warmup_packets,
                                                              sim_measurement_packets,
                                                              router_pl_num,
                                                              router_num_pls_on_exit);

        
        //connecting traffic source at xy position to signals
        traffic_sources[network_x*y + x] -> clk(clk);
        traffic_sources[network_x*y + x] -> rst_n(rst_n);
        traffic_sources[network_x*y + x] -> cntrl_in(traf_src_control[network_x*y + x]);
        traffic_sources[network_x*y + x] -> flit_out(traf_src_flit[network_x*y + x]);
        
        //connecting traffic sink at xy position to signals
        traffic_sinks[network_x*y + x] -> clk(clk);
        traffic_sinks[network_x*y + x] -> rst_n(rst_n);
        traffic_sinks[network_x*y + x] -> flit_in(traf_sink_flit[network_x*y + x]);
        traffic_sinks[network_x*y + x] -> cntrl_out(traf_sink_control[network_x*y + x]);
        
        
        //connecting router at xy position to traffic source and sink signals
        routers[network_x*y + x] -> clk(clk);
        routers[network_x*y + x] -> rst_n(rst_n);
        routers[network_x*y + x] -> flit_in[TILE](traf_src_flit[network_x*y + x]);
        routers[network_x*y + x] -> flit_out[TILE](traf_sink_flit[network_x*y + x]);
        routers[network_x*y + x] -> cntrl_in[TILE](traf_sink_control[network_x*y + x]);
        routers[network_x*y + x] -> cntrl_out[TILE](traf_src_control[network_x*y + x]);
        
        //systemc forbids leave ports unconnected so we connecting dummy signals 
        //to unused inputs and outputs of router
        // x and y index router in mesh network
        // k index links corresponding to this router - we use two dim. indexing
        //for routers and one dim. (e.g. linear) indexing for links
        if (y == 0)
        {
          routers[network_x*y + x] -> flit_in[NORTH](dummy_in_flit[k]);
          routers[network_x*y + x] -> flit_out[NORTH](dummy_out_flit[k]);
          routers[network_x*y + x] -> cntrl_in[NORTH](dummy_in_control[k]);
          routers[network_x*y + x] -> cntrl_out[NORTH](dummy_out_control[k]);
            
          k++;  
        }
        
        if (x == 0)
        {
          routers[network_x*y + x] -> flit_in[WEST](dummy_in_flit[k]);
          routers[network_x*y + x] -> flit_out[WEST](dummy_out_flit[k]);
          routers[network_x*y + x] -> cntrl_in[WEST](dummy_in_control[k]);
          routers[network_x*y + x] -> cntrl_out[WEST](dummy_out_control[k]); 
            
          k++;
        }
        
        if (x == network_x - 1)
        {
          routers[network_x*y + x] -> flit_in[EAST](dummy_in_flit[k]);
          routers[network_x*y + x] -> flit_out[EAST](dummy_out_flit[k]);
          routers[network_x*y + x] -> cntrl_in[EAST](dummy_in_control[k]);
          routers[network_x*y + x] -> cntrl_out[EAST](dummy_out_control[k]); 
          
          k++;
        }
        
        if (y == network_y - 1)
        {
          routers[network_x*y + x] -> flit_in[SOUTH](dummy_in_flit[k]);
          routers[network_x*y + x] -> flit_out[SOUTH](dummy_out_flit[k]);
          routers[network_x*y + x] -> cntrl_in[SOUTH](dummy_in_control[k]);
          routers[network_x*y + x] -> cntrl_out[SOUTH](dummy_out_control[k]); 
            
          k++;
        }
        
      }
    }
    
    //now connecting routers between each other
    
    //step1 - working with horizontal links
    
    //i and j indexing bidirectional connection between (i,j) and (i+1,j) routers by x dimension
    //l index one directional link between routers (linear indexing)
    for (int j = 0; j < network_y; j++)
    {
      for (int i = 0; i < network_x - 1; i++)
      {
        //connecting flit and control west outputs of (i+1,j) router 
        //to flit and control east inputs of (i,j) router
        routers[network_x*j + i] -> flit_in[EAST](between_router_flit[l]);
        routers[network_x*j + i] -> cntrl_in[EAST](between_router_control[l]);
 
        routers[network_x*j + i + 1] -> flit_out[WEST](between_router_flit[l]);
        routers[network_x*j + i + 1] -> cntrl_out[WEST](between_router_control[l]);
        
        l++;
        
        //connecting flit and control east outputs of (i,j) router 
        //to flit and control west inputs of (i+1,j) router    
        routers[network_x*j + i + 1] -> flit_in[WEST](between_router_flit[l]);
        routers[network_x*j + i + 1] -> cntrl_in[WEST](between_router_control[l]);
 
        routers[network_x*j + i] -> flit_out[EAST](between_router_flit[l]);
        routers[network_x*j + i] -> cntrl_out[EAST](between_router_control[l]); 
        
        l++;
                       
      }
    }  
    
    //step2 - working with vertical links
    
    //i and j indexing bidirectional connection between (i,j) and (i,j+1) routers by y dimension
    //l index one directional link between routers (linear indexing)    
    for (int j = 0; j < network_y - 1; j++)
    {
      for (int i = 0; i < network_x; i++)
      {
        //connecting south outputs of (i,j) router to north inputs of (i, j+1) router
        routers[network_x*(j + 1) + i] -> flit_in[NORTH](between_router_flit[l]);
        routers[network_x*(j + 1) + i] -> cntrl_in[NORTH](between_router_control[l]);
 
        routers[network_x*j + i] -> flit_out[SOUTH](between_router_flit[l]);
        routers[network_x*j + i] -> cntrl_out[SOUTH](between_router_control[l]); 
        
        l++;
        
        //connecting north outputs of (i, j+1) router to south inputs of (i,j) router
        routers[network_x*j + i] -> flit_in[SOUTH](between_router_flit[l]);
        routers[network_x*j + i] -> cntrl_in[SOUTH](between_router_control[l]);
 
        routers[network_x*(j + 1) + i] -> flit_out[NORTH](between_router_flit[l]);
        routers[network_x*(j + 1) + i] -> cntrl_out[NORTH](between_router_control[l]); 
        
        l++;
        
      }
    }
          
    SC_METHOD(main_process);
    sensitive_pos << clk;
  
  }
  
  
  void main_process()
  {
    if (!rst_n)
    {
      reset_complete = false;
      
      if (!startup_message)
      {
        startup_message = true;
        cout << "***********************************************************" << endl;
        cout << "*                Uniform random traffic test              *" << endl;
        cout << "***********************************************************" << endl << endl;
        cout << "-- Simulation start" << endl;
        cout << "-- Entering reset state" << endl;
      }
      
      total_latency = total_min_latency = total_max_latency = 0;
      for (int i = 0; i <= 100; i++)
        total_lat_freq[i] = 0;
  
      //initializing dummy inputs by zeros
      my_vector<flit_t> zero_flits(router_pl_num);                     //all fields of flit_t is zero by default
      my_vector<bool>   zero_controls(router_pl_num, false);
      
      for (int i = 0; i < 2*network_y + 2*network_x; i++)
      {
        dummy_in_flit[i].write(zero_flits);
        dummy_in_control[i].write(zero_controls);
      }
      
    }else
    {
      startup_message = false;
      
      if (!reset_complete)
      {
        reset_complete = true;
        cout << "-- Reset Complete" << endl;
        cout << "-- Entering warmup phase (" << sim_warmup_packets << " packets per node)" << endl;
      }
      
      //cout << "+" << endl;
      
      total_rec_packets = 0; //can be optimized: we could not silly sum all rec packets every clock
                             //but do summation incrementally, every clock cycle adding only newly arriwed packets
      
      for (int y = 0; y < network_y; y++)
        for (int x = 0; x < network_x; x++)
          total_rec_packets += traffic_sinks[network_x*y + x] -> get_stats().rec_packets_count;
    
      if (total_rec_packets == total_packets)
      {
        cout << "-- Simulation end" << endl;
        
        total_min_latency = traffic_sinks[0] -> get_stats().min_latency;
        total_max_latency = traffic_sinks[0] -> get_stats().max_latency;
        
        for (int y = 0; y < network_y; y++)
          for (int x = 0; x < network_x; x++)
          {
            total_latency += traffic_sinks[network_x*y + x] -> get_stats().total_latency;
            total_min_latency = std::min(total_min_latency, traffic_sinks[network_x*y + x] -> get_stats().min_latency);
            total_max_latency = std::max(total_max_latency, traffic_sinks[network_x*y + x] -> get_stats().max_latency);
            
            for (int i = 0; i <= 100; i++)
              total_lat_freq[i] += traffic_sinks[network_x*y + x] -> get_stats().lat_freq[i];
          }
        
        cout << endl;
        cout << "**********************************************************" << endl;
        cout << "*               Simulation statistics                    *" << endl;
        cout << "**********************************************************" << endl << endl;
        
        cout << "-- Packet Length   = " << sim_packet_length << endl;
        cout << "-- Injection Rate  = " << sim_injection_rate << " (flits/cycle/node)" << endl;
        cout << "-- Average Latency = " << (float)(total_latency / total_packets) << " (cycles)" << endl;
        cout << "-- Min. Latency    = "<< total_min_latency << ", Max. Latency = " << total_max_latency << endl << endl;
        
        cout << "Average Latencies for packets rec'd at nodes [x,y] and (no. of packets received)" << endl;
        
        float average_latency_x_y = 0;
        
        for (int y = 0; y < network_y; y++)
        {
          for (int x = 0; x < network_x; x++)
          {
            average_latency_x_y = (float)(traffic_sinks[network_x*y + x] -> get_stats().total_latency / traffic_sinks[network_x*y + x] -> get_stats().rec_packets_count);
            cout << average_latency_x_y << " (" << traffic_sinks[network_x*y + x] -> get_stats().rec_packets_count << ")\t";
          
          }
          
          cout << endl;
        }
        
        cout << endl;
        
        cout << "Flits/cycle received at each node: (should approx. injection rate)" << endl;

        int x_y_measure_start = 0;
        int x_y_measure_end = 0;
        int x_y_flit_count = 0;

        for (int y = 0; y < network_y; y++)
        {
          for (int x = 0; x < network_x; x++)
          {
            x_y_measure_start = traffic_sinks[network_x*y + x] -> get_stats().measure_start;
            x_y_measure_end = traffic_sinks[network_x*y + x] -> get_stats().measure_end;
            x_y_flit_count = traffic_sinks[network_x*y + x] -> get_stats().rec_flits_count;
            cout << (float)(x_y_flit_count) / (x_y_measure_end - x_y_measure_start)<< "\t";
          }
          
          cout << endl;
        }        
        
        cout << endl;
        
        cout << "Distribution of packet latencies: " << endl;
        cout << "Latency : Frequency (as percentage of total)" << endl;
        cout << "--------------------------";
        
        int lat_freq_x_y_i = 0;
        
        for (int i = 0; i < 100; i++)
          cout << i << " " << (float)(total_lat_freq[i]) * 100 / total_packets << endl;
        
        cout << "100+ " << (float)(total_lat_freq[100]) * 100 / total_packets << endl;
          
        sc_stop();
      }
      
    }
  }
  
  
  ~LAG_mesh_network_tb()
  {
    if (traf_sink_flit) delete [] traf_sink_flit; 
    if (traf_src_flit) delete [] traf_src_flit; 
    if (traf_sink_control) delete [] traf_sink_control; 
    if (traf_src_control) delete [] traf_src_control; 
    if (dummy_in_flit) delete [] dummy_in_flit; 
    if (dummy_out_flit) delete [] dummy_out_flit; 
    if (dummy_in_control) delete [] dummy_in_control;
    if (dummy_out_control) delete [] dummy_out_control;
    if (between_router_flit) delete [] between_router_flit;
    if (between_router_control) delete [] between_router_control;  
  }
  
  private:
  
  vector<LAG_router_tiny*>               routers;    
  vector<LAG_traffic_sink*>         traffic_sinks;
  vector<LAG_rand_traffic_src*>     traffic_sources;
  
  sc_signal<my_vector<flit_t> >*    traf_sink_flit;
  sc_signal<my_vector<flit_t> >*    traf_src_flit;
  sc_signal<my_vector<bool> >*      traf_sink_control;
  sc_signal<my_vector<bool> >*      traf_src_control;
  
  sc_signal<my_vector<flit_t> >*    dummy_in_flit;
  sc_signal<my_vector<flit_t> >*    dummy_out_flit;
  sc_signal<my_vector<bool> >*      dummy_in_control;
  sc_signal<my_vector<bool> >*      dummy_out_control;
    
  sc_signal<my_vector<flit_t> >*    between_router_flit;
  sc_signal<my_vector<bool> >*      between_router_control;
  
  bool startup_message;
  bool reset_complete;
  
  int total_rec_packets;
  int total_latency, total_min_latency, total_max_latency;
  vector<int> total_lat_freq;
  
};

#endif