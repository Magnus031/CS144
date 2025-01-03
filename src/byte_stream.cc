#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {
  
}

bool Writer::is_closed() const
{
  // Your code here.
  return finished_write;
}

void Writer::push( std::string data )
{
  // Your code here.
  uint64_t stringLen = data.size();
  uint64_t avaiableSize = available_capacity();
  if(avaiableSize == 0)
    return ;
  if(stringLen > avaiableSize){
    data = data.substr(0,avaiableSize);
  }
  // records the number of bytes which pushed into the byteStream;
  numberOfBytes += data.size();
  byteFlow+=data;
  return;
}

void Writer::close()
{
  // Your code here.
  finished_write = true;
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  // We use the maximum capacity to reduce the 
  return capacity_-byteFlow.size();
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return numberOfBytes;
}

bool Reader::is_finished() const
{
  // Your code here.
  return byteFlow.size()==0&&finished_write;
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return numberOfPop;
}

string_view Reader::peek() const
{
  // Your code here.
  return byteFlow;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  uint64_t stringLen = byteFlow.size();
  if(len>stringLen){
    byteFlow = "";
    numberOfPop+=stringLen;
  }else{
    byteFlow = byteFlow.substr(len,stringLen-len);
    numberOfPop+=len;
  }
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return byteFlow.size();
}
