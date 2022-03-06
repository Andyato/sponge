#include "byte_stream.hh"
#include <iostream>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

using namespace std;

ByteStream::ByteStream(const size_t capacity)  : _capacity(capacity) {}

size_t ByteStream::write(const string &data) {
    if (_end_input) {
        return 0;
    }
    size_t wrsz = 0, hold_size = _bytes.size();
    for (char c : data) {
        if (wrsz + hold_size >= _capacity) {
            break;
        }
        _bytes.emplace_back(std::move(c));
        ++wrsz;
    }
    _bytes_written += wrsz;
    return wrsz;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    std::string peeked;
    peeked.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        if (i >= _capacity) {
            break;
        }
        peeked += _bytes[i];
    }
    return peeked;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (i >= _capacity) {
            break;
        }
        ++_bytes_read;
        _bytes.pop_front();
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    std::string peeked;
    peeked.reserve(len);
    for (size_t i = 0; i < len; ++i) {
        if (i >= _capacity) {
            break;
        }
        peeked += std::move(_bytes.front());
        _bytes.pop_front();
    }
    _bytes_read += peeked.size();
    return peeked;
}

void ByteStream::end_input() { _end_input = true; }

bool ByteStream::input_ended() const { return _end_input; }

size_t ByteStream::buffer_size() const { return _bytes.size(); }

bool ByteStream::buffer_empty() const { return _bytes.empty(); }

bool ByteStream::eof() const { return _bytes.empty() && _end_input; }

size_t ByteStream::bytes_written() const { return _bytes_written; }

size_t ByteStream::bytes_read() const { return _bytes_read; }

size_t ByteStream::remaining_capacity() const { return _capacity - _bytes.size(); }
