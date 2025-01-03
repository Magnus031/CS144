#include "tcp_sender.hh"
#include "tcp_config.hh"
#include "byte_stream.hh" 

using namespace std;

// Get the number of sequence are outstanding;
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return number - first_id;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return retransmission_times;
}
/**
 * As many as possible to transmit the message as possible;
 * But the lenth is between min(windowsize,TCPConfig::MAX_PAYLOAD_SIZE)
 * 1. Remember to push the TCPSenderMessage into the queue;
 * 
*/
void TCPSender::push( const TransmitFunction& transmit )
{
  
  // Your code here.
  // If the windowsize have the enough space for sending, it will send again;
  while((window_size == 0 ? 1: window_size)>(number-first_id)){
    
    uint64_t remaining_window = (window_size == 0 ? 1 : window_size) - (number - first_id);
    
    uint64_t len = remaining_window > TCPConfig::MAX_PAYLOAD_SIZE ? 
                   TCPConfig::MAX_PAYLOAD_SIZE : remaining_window;

    if(remaining_window==0)
      return ;

    if(finished){
      break;
    }
    
    bool syn = false;

    // whether it is the SYN package;
    if(!SYN){
      SYN = true;
      syn = true;
      len --;
    }


    
    string temp = "";
    if(input_.reader().bytes_buffered()<len){
      len = input_.reader().bytes_buffered();
    }
    
    read(input_.reader(),len,temp);
    
    bool FIN = false;
    if(remaining_window - len >0 && input_.reader().is_finished()){
      finished = true;
      FIN = true;
    }

    TCPSenderMessage message = TCPSenderMessage(Wrap32::wrap(number,isn_),syn,temp,FIN,input_.has_error());
    if(message.sequence_length()==0){
      break;
    }
    transmit(message);


    outStandingQueue.push({number,message});
    number+=message.sequence_length();
  }
}

/**
 * This make_empty_message function is to send the empty message;
 *    The TCPSender should generate and send a zero_length message with
 * seq number set correctly;
*/
TCPSenderMessage TCPSender::make_empty_message() const
{
  // Your code here.
  TCPSenderMessage message = TCPSenderMessage{
    Wrap32::wrap(number,isn_),false,"",false,input_.has_error()
  };

  return message;
}

/**
 * TCPReceiverMessage return 2 items:
 * 1. ackno
 * 2. window_sizes; the available_capacity;
 * @param msg The TCPReceiverMessage.
 * 
 * Receive task : 
 * 1. We need to update the meta message of the Sender, especially ackno and window_sizes;
 * 2. We need to update the outStandingList; Because some information have been received;
 * 3. Set back RTO to its "inital value";
 * 4. Reset the number of "consecutive retransmissions" into zero;
 * 5. Remember to update the checkpoint;
*/
void TCPSender::receive( const TCPReceiverMessage& msg )
{
  // std::optional<Wrap32> type represents that it may have the value also not have the value;
  /**
   * update the message receiving message;
   * 1. We need to compare the msg with the segement storing in the outStandingList;
   * 2. msg.ackno -> represent the next byte we need to send;
  */
  // Here we get a false message, so we need to stop the process of receiving; 
  if(input_.has_error())
    return;
  if(msg.RST){
    input_.set_error();
    return ;
  }

  
  window_size = msg.window_size;
  // here ackno represents the next index which you would need to push;
  uint64_t index_temp = msg.ackno->unwrap(isn_,number);
  // we directly ignore the wrong ackno;
  if(index_temp > number)
    return ;
  bool flag = false;
  while(!outStandingQueue.empty()){
    auto message = outStandingQueue.front().second;
    uint64_t len = first_id + message.sequence_length();
    if(len > index_temp){
      break;
    }
    flag = true;
    first_id += message.sequence_length();
    outStandingQueue.pop();
  } 

  if(flag){
    current_RTO = initial_RTO_ms_;
    current_waiting = 0;
    retransmission_times = 0;
  }
  // here we need to Do step 2;
  // Record whether you have popped something;
  


}
/**
 * @param ms_since_last_tick the elapsed time that lasting since we called the tick method last time
 * @param transmit The transmitFunction that we need to retransmit 
 * 
 * This function's task:
 *    Every time we call the tick method, we need to update the elapsed time, and to 
 * test whether it is over the current_RTO, which means that we need to retransmit the 
 * TCPSender in the OutStandingList with the eldest waiting time. 
 * -> If the windowsize is nonzero
 *  1. keep the number of conRetransmission, if it over a value that we will terminated the process
 *  2. we need to double the RTO;
 *    Because of the reason that "Exponential backoff"，it slow down retransmissions on lousy networks to avoid further 
 *    gumming work;
 * Reset the retransmission time. and restart it when it expires the RTO milliseconds;
*/
void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Your code here.
  // update the time_since_last_transmit;
  
  current_waiting += ms_since_last_tick;
    
  if(current_waiting>=current_RTO){
    if(!outStandingQueue.empty()){
      // Here we need to retransmit
      auto it = outStandingQueue.front();
      // Here we get the eldest TCPSenderMessage which wait to retransmit;
      transmit(it.second);
      // The case that window_size is nonzero;
      if(window_size>0){
        retransmission_times++;
        current_RTO*=2;
      }
      // reset the time waiting;
      current_waiting=0;
    }
  }
  
}
