#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

// This is the main workhorse method. TCPReceiver::segment received() will be called each
// time a new segment is received from the peer.
// This method needs to:
//   • Set the Initial Sequence Number if necessary. The sequence number of the firstarriving segment that has the SYN
//   flag set is the initial sequence number. You’ll want
// to keep track of that in order to keep converting between 32-bit wrapped seqnos/acknos
// and their absolute equivalents. (Note that the SYN flag is just one flag in the header.
// The same segment could also carry data and could even have the FIN flag set.)
//   • Push any data, or end-of-stream marker, to the StreamReassembler. If the
// FIN flag is set in a TCPSegment’s header, that means that the last byte of the payload
// is the last byte of the entire stream. Remember that the StreamReassembler expects
// stream indexes starting at zero; you will have to unwrap the seqnos to produce these.
//
// Thinking about:
// SYN with DATA
// SYN with DATA with FIN
// SYN + FIN
// SYN + DATA with FIN + DATA
void TCPReceiver::segment_received(const TCPSegment &seg) {
    DUMMY_CODE(seg);

    const TCPHeader header = seg.header();
    WrappingInt32 seqno(header.seqno);

    if (not header.syn && not this->_isn.has_value()) {
        return;
    }

    if (header.syn) {
        this->_isn = WrappingInt32(header.seqno);
        seqno = seqno + 1;
    }

    size_t checkpoint = this->stream_out().bytes_written();
    size_t abs_seqno_64 = unwrap(seqno, *this->_isn, checkpoint);
    size_t stream_index = abs_seqno_64 - 1;

    this->_reassembler.push_substring(seg.payload().copy(), stream_index, header.fin);
}

// Returns an optional<WrappingInt32> containing the sequence number of the first byte that
// the receiver doesn’t already know. This is the windows’s left edge: the first byte the receiver
// is interested in receiving. If the ISN hasn’t been set yet, return an empty optional.
optional<WrappingInt32> TCPReceiver::ackno() const {
    if (not this->_isn.has_value()) {
        return {};
    }

    auto stream_bytes_writen = this->stream_out().bytes_written();

    if (this->stream_out().input_ended()) {
        return wrap(stream_bytes_writen + 1, *this->_isn) + 1;
    }

    return wrap(stream_bytes_writen + 1, *this->_isn);
}

// Returns the distance between the “first unassembled” index (the index corresponding to the
// ackno) and the “first unacceptable” index.
size_t TCPReceiver::window_size() const { return this->stream_out().remaining_capacity(); }
