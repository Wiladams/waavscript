#pragma once

#include "ps_type_file.h"
#include "ps_type_dictionary.h"
#include "psvm.h"
#include "ps_charcats.h"

#include <memory>
#include <vector>


namespace waavs 
{



    class ASCII85DecodeFilter : public PSFile
    {
    private:
        std::shared_ptr<PSFile> _source;
        std::vector<uint8_t> _buffer;
        size_t _bufferPos;
        bool _finished;

    public:
        explicit ASCII85DecodeFilter(std::shared_ptr<PSFile> source)
            : _source(source), 
            _bufferPos(0), 
            _finished(false)
        {
        }

        bool readByte(uint8_t& byte) override
        {
            if (_bufferPos >= _buffer.size()) {
                if (!refillBuffer())
                    return false;
            }

            byte = _buffer[_bufferPos++];
            return true;
        }

        bool isEOF() const override
        {
            return _finished && _bufferPos >= _buffer.size();
        }


        void finalize() override
        {
            if (_finished)
                return;

            // Try to finish parsing the trailer
            uint8_t c;
            while (_source->readByte(c)) {
                if (c == '~') {
                    uint8_t next;
                    if (_source->readByte(next) && next == '>') {
                        _finished = true;
                        return;
                    }
                }

            }

            _source->finalize();

        }

    private:

        bool refillBuffer()
        {
            if (_finished)
                return false;

            _buffer.clear();
            _bufferPos = 0;

            uint8_t in[5] = {};
            int count = 0;

            while (count < 5) {
                uint8_t c;
                if (!_source->readByte(c)) {
                    _finished = true;
                    return false;
                }

                if (PSCharClass::isWhitespace(c)) {
                    continue;
                }

                if (c == '~') {
                    uint8_t next;
                    if (_source->readByte(next) && next == '>') {
                        _finished = true;
                        break;
                    }
                    else {
                        _finished = true;
                        return false; // Invalid end sequence
                    }
                }

                if (c == 'z' && count == 0) {
                    _buffer.insert(_buffer.end(), 4, 0);
                    return true;
                }

                if (c < '!' || c > 'u') {
                    _finished = true;
                    return false; // Invalid character
                }

                in[count++] = c;
            }

            for (int i = count; i < 5; ++i)
                in[i] = 'u';

            uint32_t value = 0;
            for (int i = 0; i < 5; ++i)
                value = value * 85 + (in[i] - 33);

            for (int i = 3; i >= 0; --i)
                _buffer.push_back((value >> (i * 8)) & 0xFF);

            if (count < 5) {
                _buffer.resize(count - 1);
                _finished = true;
            }

            return !_buffer.empty();
        }
    };



    //=====================================================
    // Run Length Decode Filter
    //=====================================================
    class RunLengthDecodeFilter : public PSFile
    {
    public:
        explicit RunLengthDecodeFilter(std::shared_ptr<PSFile> source)
            : _source(source), _mode(Mode::Idle), _pos(0), _count(0), _finished(false)
        {
        }

        bool readByte(uint8_t& out) override
        {
            while (true) {
                if (_finished)
                    return false;

                if (_pos < _count) {
                    out = _buffer[_pos++];
                    return true;
                }

                // Refill buffer
                uint8_t control;
                if (!_source->readByte(control)) {
                    _finished = true;
                    return false;
                }

                if (control == 128) {
                    _finished = true;
                    return false; // EOD
                }

                if (control <= 127) {
                    // Literal run of (control + 1) bytes
                    _count = control + 1;
                    _pos = 0;
                    _buffer.resize(_count);
           
                    for (size_t i = 0; i < _count; ++i) {
                        if (!_source->readByte(_buffer[i])) {
                            _finished = true;
                            return false;
                        }
                    }
                }
                else {
                    // Repeated byte
                    _count = 257 - control;
                    _pos = 0;
                    uint8_t repeated;
                    if (!_source->readByte(repeated)) {
                        _finished = true;
                        return false;
                    }
                    //_buffer.resize(_count, repeated);
                    _buffer.assign(_count, repeated);
                }
            }
        }

        bool isEOF() const override
        {
            return _finished && (_pos >= _count);
        }

        //bool hasCursor() const override { return false; }
        void finalize() override
        {
            if (_finished)
                return;
            // Try to read until EOD
            uint8_t control;
            while (_source->readByte(control)) {
                if (control == 128) {
                    _finished = true;
                    return; // EOD
                }
                // Ignore other bytes, we just want to reach EOD
            }
            _source->finalize();
            _finished = true;
        }



    private:
        enum class Mode { Idle, Literal, Repeat };

        std::shared_ptr<PSFile> _source;
        std::vector<uint8_t> _buffer;
        size_t _pos;
        size_t _count;
        bool _finished;
        Mode _mode;
    };



}


