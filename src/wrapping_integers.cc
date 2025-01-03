#include "wrapping_integers.hh"

using namespace std;

/**
 * @param n the absoute sequence number 
 * @param zero_point initial value
*/
Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  return zero_point+n;
}

/**
 * @param zero_point ISN;
 * @param checkpoint the value of referrence
 * @return a absolute sequence number 
*/
uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  uint32_t start_index = raw_value_ - zero_point.raw_value_;
  uint64_t start_ab_index = (uint64_t) start_index;
  uint64_t standard_index = (1UL<<31)-1;
  uint64_t interval = (1UL<<32);

  if(start_ab_index>=checkpoint){
    return start_ab_index;
  }
  // Here it is the case that start_ab_index > checkpoints
  uint64_t k = (checkpoint - start_ab_index)/interval;
  start_ab_index += k*interval;
  if(checkpoint-start_ab_index> standard_index)
    start_ab_index+=interval;
  return start_ab_index;
  
}
