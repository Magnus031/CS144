#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  // If the byteStream has error, we directly abort the receivation;
  // Part 1: Error detection
  if(writer().has_error())
    return ;
  if(message.RST){
    reader().set_error();
    return ;
  }

  // Part2 : detection for SYN;
  if(!has_value){
    if(!message.SYN){
      return ;
    }
    zero_point = Wrap32(message.seqno.get_raw_value());
    has_value = true;
  }
  
  if(message.payload.length()!=0&&message.seqno==zero_point&&!message.SYN)
    return ;
  
  
  uint64_t first_unassembled = reassembler_.writer().bytes_pushed()+1;
  uint64_t stream_index = message.seqno.unwrap(zero_point,first_unassembled)+static_cast<uint64_t>(message.SYN)-1;
 
  reassembler_.insert(stream_index,message.payload,message.FIN);
  
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  uint16_t windows_size = writer().available_capacity() > UINT16_MAX 
                          ? UINT16_MAX :
                          static_cast<uint16_t> (writer().available_capacity());

  if(has_value){
    const uint64_t ack_for_seqno = writer().bytes_pushed()+1+static_cast<uint64_t>(writer().is_closed());
    return TCPReceiverMessage{Wrap32::wrap(ack_for_seqno,zero_point),windows_size,writer().has_error()};
  }
  return TCPReceiverMessage{nullopt,windows_size,writer().has_error()};
}
