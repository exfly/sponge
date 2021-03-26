#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), _next_index(0), _unassembled_map() {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    DUMMY_CODE(data, index, eof);

    for (size_t i = 0; i < data.size(); ++i) {
        size_t position = index + i;
        if (position < this->_next_index) {
            // 已经 write to stream 的部分不需要在此处理
            continue;
        }

        bool _eof = false;
        if (i == data.size() - 1) {
            _eof = eof;
        }
        this->_unassembled_map[position] = DataChunk{data.substr(i, 1), i, _eof};
    }

    bool will_eof = false;
    for (; this->_unassembled_map.count(this->_next_index) > 0;) {
        auto pair = this->_unassembled_map.find(this->_next_index);
        size_t writen_size = this->stream_out().write(pair->second.data);
        if (writen_size == 0) {
            break;
        }
        this->_unassembled_map.erase(pair->first);
        this->_next_index += writen_size;
        will_eof = pair->second.eof;
    }

    if ((will_eof && empty()) || (eof && empty())) {
        this->stream_out().end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return this->_unassembled_map.size(); }

bool StreamReassembler::empty() const { return this->unassembled_bytes() == 0; }
