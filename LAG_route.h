/* -----------------------------------------------------------------------------
 * (C)2012 Korotkyi Ievgen
 * National Technical University of Ukraine "Kiev Polytechnic Institute"
 * -----------------------------------------------------------------------------
 */

#ifndef LAG_ROUTE_H
#define LAG_ROUTE_H

#include "systemc.h"
#include "types.h"

flit_t route(const flit_t& flit_in)
{
  flit_t flit_out;
  int output_trunk;
  int dx;
  int dy;
  
  dx = flit_in.route.dx;
  dy = flit_in.route.dy;
  
  if (dx != 0)
  {
    if (dx > 0)
    {
    
      output_trunk = EAST;
      dx--;
      
    }else
    {
    
      output_trunk = WEST;
      dx++;
    
    }
  } else
  {
    if(dy == 0)
    {
      output_trunk = TILE;
      
    } else if (dy > 0)
    {
      
      output_trunk = SOUTH;
      dy--;
    
    } else
    {
      
      output_trunk = NORTH;
      dy++;    
    
    }
    
  }
  
  flit_out = flit_in;
  flit_out.route.output_trunk = output_trunk;
  flit_out.route.dx = dx;
  flit_out.route.dy = dy;
  
  return flit_out;

}

#endif