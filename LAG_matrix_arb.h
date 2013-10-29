/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_MATRIX_ARB_H
#define LAG_MATRIX_ARB_H

class LAG_matrix_arb
{
public:

  bool* requests;
  bool* grants;  
  bool success;
  
  void reset()
  {
    success = true; 
    
    for (int i = 0; i < size; i++)
    {
      for (int j = 0; j < size; j++)
        state[i*size + j] = true;
      
      grants[i] = false;
    }
  }
  
  inline void update_priorities()
  {  
    if (success)
      for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
          state[j*size + i] = (state[j*size + i] && !grants[j]) || grants[i];
  }
  
  inline void update_grants()
  {     
    for (int i = 0; i < size; i++)
    {
      grants[i] = true;
      
      for (int j = 0; j < size; j++)
      { 
        if (i == j)
        {
          //pri[i*size + j] = requests[i];
          grants[i] &= requests[i];
          
        } else
        {
          if (j > i)                 
          {  
            
            grants[i] &= !(requests[j] && state[j*size + i]);
          
          }
          else   
          {
          
            grants[i] &= !(requests[j] && !state[i*size + j]);
          
          }
        
        }
      }
     } 
  }
  
  LAG_matrix_arb(int size_ = 4): size(size_)
  {
    requests = new bool[size];
    grants = new bool[size]; 
    state = new bool[size*size];
    success = true;
  }
  
  ~LAG_matrix_arb()
  {
    if(requests) delete [] requests;
    if(grants) delete [] grants;
    if(state) delete [] state;
  }
  
private:

  int size;  
  bool *state;  
};

#endif