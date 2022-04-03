#include "stream_reassembler.hh"
#include <iostream>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _unassembled(), _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    _first_unread = _output.bytes_read();
    _first_unassembled = _first_unread + _output.buffer_size();
    _first_unacceptable = _first_unread + _capacity;
//    if (index >= _first_unacceptable) {
//        return;
//    }
    Segment segment {data, index};
    if (index < _first_unassembled && index + data.length() > _first_unassembled) {
        segment._index = _first_unassembled;
        segment._data = data.substr(_first_unassembled - index, data.length() - (_first_unassembled - index));
    }
    if (index + data.length() > _first_unacceptable) {
        segment._data = segment._data.substr(0, _first_unacceptable - index);
    }
    if (eof) {
        _eof = index + data.length();
    }
    _cache_unassembled(segment);
    _assemble();
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembled_bytes; }

bool StreamReassembler::empty() const { return _unassembled.empty(); }

void StreamReassembler::_cache_unassembled(StreamReassembler::Segment& seg) {
    for (auto iter = _unassembled.begin(); iter != _unassembled.end();) {
        if (!_is_overlaped(*iter, seg)) {
            ++iter;
            continue;
        }
        _handle_overlaped_segment(seg, *iter);
        _unassembled_bytes -= iter->_data.length();
        iter = _unassembled.erase(iter);
    }
    _unassembled_bytes += seg._data.length();
    _unassembled.emplace(seg);
}

void StreamReassembler::_assemble() {
    for (auto iter = _unassembled.begin(); iter != _unassembled.end();) {
        if (iter->_index == _first_unassembled) {
            _first_unassembled += _output.write(iter->_data);
            _unassembled_bytes -= iter->_data.length();
            iter = _unassembled.erase(iter);
            if (_eof != static_cast<size_t>(-1) && _eof == _output.bytes_written()) {
                _output.end_input();
                break;
            }
        } else {
            ++iter;
        }
    }
}

bool StreamReassembler::_is_overlaped(const StreamReassembler::Segment &seg1, const StreamReassembler::Segment &seg2) {
    if (seg1._index < seg2._index)
        return seg1._index + seg1._data.length() > seg2._index;
    else
        return seg2._index + seg2._data.length() > seg1._index;
}

void StreamReassembler::_handle_overlaped_segment(StreamReassembler::Segment &seg1,
                                                  const StreamReassembler::Segment &seg2) {
    size_t merge_index = seg1._index <= seg2._index ? seg1._index : seg2._index;

    if (seg1._index <= seg2._index && seg1._index + seg1._data.length() < seg2._index + seg2._data.length()) {
        size_t start_pos = seg1._index + seg1._data.length() - seg2._index;
        seg1._data += seg2._data.substr(start_pos, seg2._data.length() - start_pos);
    } else if (seg1._index > seg2._index && seg1._index + seg1._data.length() >= seg2._index + seg2._data.length()) {
        seg1._data = seg2._data.substr(0, seg1._index - seg2._index) + seg1._data;
    } else if (seg1._index > seg2._index && seg2._index + seg2._data.length() > seg1._index + seg1._data.length()) {
        std::string tmp = seg1._data;
        seg1._data = seg2._data;
        size_t offset = seg1._index - seg2._index;
        for (size_t i = 0; i < tmp.length(); ++i) {
            seg1._data[i + offset] = tmp[i];
        }
    }

    seg1._index = merge_index;
}

