/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_CROSSBAR_TINY_H
#define LAG_CROSSBAR_TINY_H

#include "types.h"

class LAG_crossbar_tiny
{
public:

  void perform_switch(const vector<int> & select_trunk_i,
                      const vector<int> & select_link_i,
                      const vector<bool> & select_link_valid_i,
                      const vector<flit_t> & data_in_i,
                      vector<flit_t> & data_out_o
                      )
  {
    int k,l;
      
    assert(select_trunk_i.size() == trunks_num*links_num);
    assert(select_link_i.size() == trunks_num*links_num);
    assert(select_link_valid_i.size() == trunks_num*links_num);
    assert(data_in_i.size() == trunks_num*links_num);
    assert(data_out_o.size() == trunks_num*links_num);
      
    for(k = 0; k < trunks_num; k++)
      for(l = 0; l < links_num; l++)
        data_out_o[k*links_num + l] = zero_flit;
        
    for(int i = 0; i < trunks_num; i++)
      for(int j = 0; j < links_num; j++) 
      {   
        k = select_trunk_i[i*links_num + j]; // k - output trunk
        l = select_link_i[i*links_num + j];  // l - output link
          
        if(select_link_valid_i[i*links_num + j])         
          data_out_o[k*links_num + l] = data_in_i[i*links_num + j];      
          
      }
  }
  
  LAG_crossbar_tiny(string name_,
                int trunks_num_ = 5,
                int links_num_ = 2
                ) : name(name_), trunks_num(trunks_num_), 
                links_num(links_num_)
  {
    
  }
  
private:

  string name;
  
  int trunks_num;
  int links_num;   // number of physical links per trunk  
  
  const flit_t zero_flit;
   
};

#endif