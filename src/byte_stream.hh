#pragma once

#include <cstdint>
#include <string>
#include <string_view>

class Reader;
class Writer;
/**
 * What do we need to do?
 * ByteStream 是一个字节流的类，我们把它抽象成一个对象，什么对象呢？就是简单的来说具有两种方法
 * 1. 写 Writer() 也就是我们对这个有规定尺寸的字节流长度的ByteStream进行抽象，具有最大的容量 Capacity_
 *    - 每次对ByteStream进行写，push(std::string data).我们要对data的size进行判定，是否能继续的往ByteStream中输入; 
 *    - 
 * 2. 读 Reader() 
 * 
 * 
*/
class ByteStream
{
public:
  explicit ByteStream( uint64_t capacity );

  // Helper functions (provided) to access the ByteStream's Reader and Writer interfaces
  Reader& reader();
  const Reader& reader() const;
  Writer& writer();
  const Writer& writer() const;

  void set_error() { error_ = true; };       // Signal that the stream suffered an error.
  bool has_error() const { return error_; }; // Has the stream had an error?

// protected represents that it can be used in other class which borned from this class;
protected:
  // Please add any additional state to the ByteStream here, and not to the Writer and Reader interfaces.
  uint64_t capacity_;
  bool error_ {};
  bool finished_write{false};
  // It records the number of bytes that we 
  uint64_t numberOfBytes=0;
  uint64_t numberOfPop=0;
  std::string byteFlow="";
};

class Writer : public ByteStream
{
public:
  void push( std::string data ); // Push data to stream, but only as much as available capacity allows.
  void close();                  // Signal that the stream has reached its ending. Nothing more will be written.

  bool is_closed() const;              // Has the stream been closed?
  uint64_t available_capacity() const; // How many bytes can be pushed to the stream right now?
  uint64_t bytes_pushed() const;       // Total number of bytes cumulatively pushed to the stream
};

class Reader : public ByteStream
{
public:
  std::string_view peek() const; // Peek at the next bytes in the buffer
  void pop( uint64_t len );      // Remove `len` bytes from the buffer

  bool is_finished() const;        // Is the stream finished (closed and fully popped)?
  uint64_t bytes_buffered() const; // Number of bytes currently buffered (pushed and not popped)
  uint64_t bytes_popped() const;   // Total number of bytes cumulatively popped from stream
};

/*
 * read: A (provided) helper function thats peeks and pops up to `len` bytes
 * from a ByteStream Reader into a string;
 */
void read( Reader& reader, uint64_t len, std::string& out );
