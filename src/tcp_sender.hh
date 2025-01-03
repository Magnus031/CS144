#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>
#include <utility>

using namespace std;

class TCPSender
{
public:
  /**
   * @param input the input bytestream
   * @param isn   Wrap32
   * @param uint64_t the time fragment for waiting the last eldest outstanding segement;
   * 
  */
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), initial_RTO_ms_( initial_RTO_ms )
  {
    // initial the rto value;
    this->current_RTO = initial_RTO_ms;
  }

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
  Writer& writer() { return input_.writer(); }
  const Writer& writer() const { return input_.writer(); }

  // Access input stream reader, but const-only (can't read from outside)
  const Reader& reader() const { return input_.reader(); }

private:
  // Variables initialized in constructor
  ByteStream input_;
  // The initial zero point.
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  
  // The helper members we predefined;
  // The outStanding segements queue;
  queue<pair<uint64_t,TCPSenderMessage>> outStandingQueue{};
  // The number we have pushed into the byteStream;
  // Also it represents the index in the absolute sequence;
  uint64_t number{0};
  // Next Index, The stream index that we have successfully pushed;
  uint64_t first_id{0};
  // The state information that we need to change after receiving
  uint64_t window_size{1};
  // Retransmission
  uint64_t retransmission_times{0};
  uint64_t current_RTO{0};
  uint64_t current_waiting{0};
  // Finish label
  bool finished{false};
  bool SYN{false};
};
