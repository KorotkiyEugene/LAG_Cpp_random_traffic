/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef TYPES_H
#define TYPES_H

#include "systemc.h"
#include "parameters.h"
#include <vector>
#include <string>

using std::string;
using std::stringstream;
using std::vector;

enum port_names {NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3, TILE = 4};

typedef int data_t;

class chan_cntrl_t 
{
public:
  bool* credits;
  
  chan_cntrl_t(int pl_number_)
  {
    credits = new bool[pl_number_];
  }
  
  ~chan_cntrl_t()
  {
    if(credits) delete [] credits;
  }

};

struct sim_stats_t      //not a signal
{
  int total_latency;
  int min_latency, max_latency;
  
  int measure_start, measure_end;
  int rec_flits_count, rec_packets_count;
  
  vector<int> lat_freq;
  
  /*inline bool operator == (const sim_stats_t & rhs) const
  {
    return (rhs.total_latency == total_latency && rhs.total_hops == total_hops && \
            rhs.min_latency == min_latency && rhs.max_latency == max_latency && \
            rhs.min_hops == min_hops && rhs.max_hops == max_hops && \
            rhs.measure_start == measure_start && rhs.measure_end == measure_end && \
            rhs.flit_count == flit_count && rhs.lat_freq == lat_freq); 
  }*/
  
  sim_stats_t(): lat_freq(101)
  {
    rec_packets_count = 0;
    rec_flits_count = 0;
    measure_start = -1;
    measure_end = -1;
    min_latency = -1;
    max_latency = 0;
    total_latency = 0;
      
    for (int i = 0; i <= 100; i++)
      lat_freq[i] = 0;
  } 

};

class debug_t{

public:
  
  bool measure;
  int flit_id;
  int packet_id;
  long long int inject_time;
  int x_dest, y_dest;
  int x_src, y_src;

  inline bool operator == (const debug_t & rhs) const
  {
    return (rhs.measure == measure && rhs.flit_id == flit_id && rhs.packet_id == rhs.packet_id && \
            rhs.inject_time == inject_time && rhs.x_dest == x_dest && rhs.y_dest == y_dest && \
            rhs.x_src == x_src && rhs.y_src == y_src);
  }
  
  inline friend ostream& operator << (ostream& os, const debug_t& v) 
  {
    os << "(" << v.measure << "," << v.flit_id << "," << v.packet_id << "." \
      << v.inject_time << "," << v.x_dest << "," << v.y_dest << "," << v.x_src \
      << "," << v.y_src << ")";
    
    return os;
  }
    
  debug_t() 
  {
      measure = 0;
      flit_id = 0;
      packet_id = 0;
      inject_time = 0;
      x_dest = 0;
      y_dest = 0;
      x_src = 0;
      y_src = 0;
  };
  
};

class route_t {

public: 
  
  int dx;
  int dy;
  int output_trunk; 

  inline bool operator == (const route_t & rhs) const
  {
    return (rhs.dx == dx && rhs.dy == dy && rhs.output_trunk == output_trunk);
  } 
  
  inline friend ostream& operator << (ostream& os, const route_t& v) 
  {
    os << "(" << v.dx << "," << v.dy << "," << v.output_trunk << ")";
    
    return os;
  }
    
  route_t()
  {
    dx = dy = output_trunk = 0;
  }
  
};

class flit_t {

public:  
  
  bool valid; 
  bool head;
  bool tail;
  route_t route;
  data_t data;
  debug_t debug;
  
  inline bool operator == (const flit_t & rhs) const
  {
    return (rhs.valid == valid && rhs.head == head && rhs.tail == tail && \
            rhs.debug == debug && rhs.route == route && rhs.data == data);
  } 
  
  inline friend ostream& operator << (ostream& os, const flit_t& v) 
  {
    os << "(" << v.valid << "," << v.head << "," << v.tail << "," << \
      v.route << "," << v.data << "," << v.debug << ")";
      
    return os;
  }
    
  flit_t ()
  {
    valid = head = tail = data = 0;
  }
  
};

void sc_trace(sc_trace_file *tf, const flit_t & flit, const string & name)       // Function to trace type flit_t
{                                                                                // We override sc_trace only for flit_t, because 
  sc_trace(tf, flit.valid, name + ".valid");                                     // route_t and debug_t are components of flit_t
  sc_trace(tf, flit.head, name + ".head");                                       // and do not used apart from it
  sc_trace(tf, flit.tail, name + ".tail" );
  sc_trace(tf, flit.route.dx, name + ".route.dx");
  sc_trace(tf, flit.route.dy, name + ".route.dy");
  sc_trace(tf, flit.route.output_trunk, name + ".route.output_trunk");
  sc_trace(tf, flit.data, name + ".data");
  sc_trace(tf, flit.debug.measure, name + ".debug.measure");
  sc_trace(tf, flit.debug.flit_id, name + ".debug.flit_id");
  sc_trace(tf, flit.debug.packet_id, name + ".debug.packet_id");
  sc_trace(tf, flit.debug.inject_time, name + ".debug.inject_time");
  sc_trace(tf, flit.debug.x_dest, name + ".debug.x_dest");
  sc_trace(tf, flit.debug.y_dest, name + ".debug.y_dest");
  sc_trace(tf, flit.debug.x_src, name + ".debug.x_src");
  sc_trace(tf, flit.debug.y_src, name + ".debug.y_src");
}

template <class X> class my_vector : public vector <X>
{
public:
  explicit my_vector(void) : vector <X> ()
  {
  }
    
  explicit my_vector(const int& n, const X& value = X()) : vector <X> (n, value)
  {
  }
  
  inline friend ostream& operator << (ostream& os, const vector<X> & v)
  {
    for(int i = 0; i < v.size(); i++)
      os << v[i] << " ";
      
    return os;
  }

};

void sc_trace(sc_trace_file *tf, const my_vector<bool> & v, const string & name)
{
 for (int i = 0; i < v.size(); i++)
 {  
  stringstream ss;
  ss << name << i; 
  sc_trace(tf, v[i], ss.str().c_str());
 }
}

void sc_trace(sc_trace_file *tf, const my_vector<flit_t> & v, const string & name)
{
 for (int i = 0; i < v.size(); i++)
 {
  stringstream ss;
  ss << name << i;
  sc_trace(tf, v[i], ss.str().c_str());
 }
}

void sc_trace(sc_trace_file *tf, const my_vector<int> & v, const string & name)
{ 
 for (int i = 0; i < v.size(); i++)
 {
  stringstream ss;
  ss << name << i;
  sc_trace(tf, v[i], ss.str().c_str());
 }
}


#endif // TYPES_H