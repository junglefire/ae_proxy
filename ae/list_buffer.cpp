#include "list_buffer.h"

#include <iostream>

using namespace smart;
using namespace std;

ListBuffer::ListBuffer() {
}

ListBuffer::~ListBuffer() {
}

int ListBuffer::append(const char* data, uint64_t size) {
    for (uint64_t i = 0; i < size; i++) {
        buffer_.push_back(data[i]);
    }
    return 0;
}

int ListBuffer::read_buffer_n(std::string& data, uint64_t N) {
    if (N > (uint64_t)buffer_.size()) {
        return -1;
    }

    auto it = buffer_.begin();
    for (uint64_t i = 0; i < N; i++) {
        data.push_back(*it);
        it++;
    }
    return 0;
}

uint64_t ListBuffer::read_size() {
    return (uint64_t)buffer_.size();
}

int ListBuffer::clear_buffer_n(uint64_t N) {
    for (uint64_t i = 0; i < N; i++) {
        if (buffer_.empty())
            break;
        buffer_.pop_front();
    }
    return 0;
}

int ListBuffer::clear() {
    buffer_.clear();
    return 0;
}
