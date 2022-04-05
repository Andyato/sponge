#include "tcp_receiver.hh"

#include <string>

#include "tcp_header.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader& header = seg.header();
    if (!_syn && !header.syn) {
        return;
    }
    if (header.syn) {
        _syn = true;
        _isn = header.seqno;
    }
    if (header.fin) {
        _fin = header.fin;
    }

    const std::string data = seg.payload().copy();
    static size_t checkpoint = 0;
    size_t absolute_seqno = unwrap(header.seqno, _isn, checkpoint);
    size_t stream_index = header.syn ? 0 : absolute_seqno - 1;
    _reassembler.push_substring(data, stream_index, header.fin);
    checkpoint = absolute_seqno;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_syn) {
        return {};
    }
    size_t absolute_ack = _reassembler.stream_out().bytes_written() + 1;
    if (_fin && _reassembler.unassembled_bytes() == 0) {
        absolute_ack++;
    }
    WrappingInt32 ackno = wrap(absolute_ack, _isn);
    return optional(std::move(ackno));
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size(); }
