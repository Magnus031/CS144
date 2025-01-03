#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // Your code here.
  uint64_t capacity = output_.writer().available_capacity();
  // We can't push more 
  if(capacity==0)
    return;
  
  if(is_last_substring&&(first_index+data.size()<=start+capacity)){
    // we have meet the finish part;
    finish = true;
    finish_end = first_index+data.size();
  }

  // case 1 : we need to discard this part of the string;
  if(first_index+data.size()>start+capacity){
    data = data.substr(0,start+capacity-first_index);
  }
  // we do all the operation in the buffer;  
  push_buffer(first_index,data); 
  // it ensures that the internal_store must exist the element;
  if(buffer_start<=start){
    uint64_t pos = start - buffer_start;
    uint64_t s2 = buffer_start;
    auto node = pop_buffer();
    string node_string = node.second;
    if(node_string.size()+s2>=start){
      node_string = node_string.substr(pos,node_string.length()-(pos));
      buffer.append(node_string);
      start = buffer.length();
      output_.writer().push(node_string);
    }
    if(is_last_substring||(finish&&start>=finish_end)){
      output_.writer().close();
    }
  }
}
uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return pending_size;
}


void Reassembler::push_buffer( uint64_t first_index,string data){
    // case 1 : if it is empty, we directly push into;
    if(internal_store.empty()){
      internal_store.insert(make_pair(first_index,data));
      pending_size += data.size();
      buffer_start = first_index;
      return ;
    }
    // case 2 : it is not empty;
    // case 2.1 : we directyly push in the front;
    uint64_t end = first_index + data.length() -1;
    if(end == buffer_start-1){
      buffer_start = first_index;
      auto t = internal_store.begin();
      auto it_item = *t;
      internal_store.erase(t);
      pending_size += data.size();
      internal_store.insert({first_index,data+it_item.second});
      return ;
    }else if(end<buffer_start){
      buffer_start = first_index;
      pending_size += data.size();
      internal_store.insert(make_pair(first_index,data));
      return;
    }
    // case 2.2 : we need to do some operation in the buffer;
    // it means the first iterator element which will be poped;
    uint64_t s = first_index;
    uint64_t e = first_index+data.size();

    if(data.length()==0)
      return ;

    string temp = data;
    // to record whether to change any node of the set;
    bool flag = false;
    for(auto it:internal_store){
      uint64_t end_string = it.first+it.second.length();
      if(it.first<=first_index&&end_string>=e){
        return ;
      }
      if(it.first<=first_index&&end_string>=first_index){
        temp = it.second+data.substr(end_string-first_index,data.length()-(end_string-first_index));
        s = it.first;
        flag = true;
      }
      uint64_t index_end = first_index+data.size();
      if(!flag&&it.first>=first_index&&it.first+it.second.size()<=e)
        flag = true;
      if(it.first<=index_end&&end_string>=index_end){
        flag = true;
        e = end_string;
        temp += it.second.substr(index_end-it.first,it.second.length()-(index_end-it.first));
        break;
      }
    }

    // case that we do not need to do any change;
    if(!flag){
      internal_store.insert(make_pair(first_index,data));
      pending_size += data.size();
    }else{
      // here we need to do change;
      pending_size = 0;
      for(auto it=internal_store.begin();it!=internal_store.end();){
          uint64_t start_it = it->first;
          uint64_t end_it = it->first+it->second.size();
          if(start_it>=s&&end_it<=e){
            it=internal_store.erase(it);
          }else{
            pending_size+=it->second.size();
            it++;
          }
      }
      internal_store.insert({s,temp});
      if(s<buffer_start)
        buffer_start = s;
      pending_size+=temp.size();
    }

}

/**
 * also need to update the internal_start;
*/
pair<uint64_t,string> Reassembler::pop_buffer(){

    auto it = internal_store.begin();
    auto it_item = *it;
    pending_size -= it_item.second.size();
    internal_store.erase(it);
     
    // update the buffer start position;
    if(!internal_store.empty()){
      auto node = internal_store.begin();
      buffer_start = (*node).first;
    }
    return it_item;
}