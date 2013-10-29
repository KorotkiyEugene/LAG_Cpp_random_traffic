/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_TREE_ARB_H
#define LAG_TREE_ARB_H

//#include "LAG_round_robin_arb.h"
#include "LAG_matrix_arb.h"

class LAG_tree_arb
{
public:

  bool* requests;
  bool* grants;
  
  void reset()
  { 
    for (int i = 0; i < group_num; i++)
      group_arbiters[i] -> reset(); 
      
    between_group_arbiter -> reset();  
    
    for (int i = 0; i < size; i++)
      grants[i] = false;
  }
  
  void update_priorities()
  {
    between_group_arbiter -> update_priorities();
    
    for (int i = 0; i < group_num; i++)
      group_arbiters[i] -> update_priorities();
  }
  
  void update_grants()
  {
    for (int i = 0; i < group_num; i++)
    {
      between_group_arbiter -> requests[i] = false;
      
      for (int j = 0; j < group_size; j++)
      {
        between_group_arbiter -> requests[i] |= requests[i*group_size + j];
        group_arbiters[i] -> requests[j] = requests[i*group_size + j];        
      }
      
      group_arbiters[i] -> update_grants();
    }  
    
    between_group_arbiter -> update_grants();
    between_group_arbiter -> update_priorities();
    
    for (int i = 0; i < group_num; i++)
    {
      for (int j = 0; j < group_size; j++)
        grants[i*group_size + j] = group_arbiters[i] -> grants[j] && between_group_arbiter -> grants[i];  
      
     group_arbiters[i] -> success = between_group_arbiter -> grants[i]; 
     group_arbiters[i] -> update_priorities();
    }
    
  }
  
  LAG_tree_arb(int size_ = 4, int group_size_ = 2): size(size_), group_size(group_size_)
  {
    //cout << "Constructor start!" << endl;
    requests = new bool[size];
    grants = new bool[size];
    group_num = size / group_size;
    
    //between_group_arbiter = new LAG_round_robin_arb(group_num);
    between_group_arbiter = new LAG_matrix_arb(group_num);
    
    //group_arbiters = new LAG_round_robin_arb*[group_num];
    group_arbiters = new LAG_matrix_arb*[group_num];
    
    for (int i = 0; i < group_num; i++)
      //group_arbiters[i] = new LAG_round_robin_arb(group_size);
      group_arbiters[i] = new LAG_matrix_arb(group_size);
    
  }
  
  ~LAG_tree_arb()
  {
    if(requests) delete [] requests;
    if(grants) delete [] grants;
    
    if(between_group_arbiter) delete between_group_arbiter;
    
    for (int i = 0; i < group_num; i++)
      if(group_arbiters[i]) delete group_arbiters[i];
      
    if (group_arbiters) delete [] group_arbiters;  
  }
  
private:

  int size;  
  int group_size;
  int group_num;
  
  //LAG_round_robin_arb** group_arbiters;
  //LAG_round_robin_arb* between_group_arbiter;
  LAG_matrix_arb** group_arbiters;
  LAG_matrix_arb* between_group_arbiter;
  
};

#endif