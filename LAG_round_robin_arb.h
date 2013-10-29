/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_ROUND_ROBIN_ARB_H
#define LAG_ROUND_ROBIN_ARB_H

class LAG_round_robin_arb
{
public:

  bool* requests;
  bool* grants;  
  bool success;
  
  void reset()
  {
    max_pri_input = 0;   
    success = true; 
    
    for (int i = 0; i < size; i++)
      grants[i] = false;
  }
  
  inline void update_priorities()
  {  
    if (success)
      for (int i = 0; i < size; i++)
        {
        
          if(grants[i])
          { 
            max_pri_input = (i < size - 1) ? (i + 1) : 0;   
            return;
          }           
    
        }   
  }
  
  inline void update_grants()
  {     
    for (int i = 0; i < size; i++)
      grants[i] = false;      
      
    for (int i = max_pri_input; i < size; i++)  
      if(requests[i])
      {
        grants[i] = true;  
        return;      
      }

    for (int i = 0; i < max_pri_input; i++)
      if(requests[i])
      {
        grants[i] = true;   
        return;      
      }
  }
  
  LAG_round_robin_arb(int size_ = 4): size(size_), max_pri_input(0)
  {
    requests = new bool[size];
    grants = new bool[size]; 
    success = true;
  }
  
  ~LAG_round_robin_arb()
  {
    if(requests) delete [] requests;
    if(grants) delete [] grants;
  }
  
private:

  int size;  
  int max_pri_input;
    
};

#endif