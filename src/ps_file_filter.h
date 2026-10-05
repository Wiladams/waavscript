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


    //=====================================================
    // EExec Decode Filter
    //=====================================================
    class EExecDecodeFilter : public PSFile
    {
    private:
        static constexpr uint16_t kInitialKey = 55665u;
        static constexpr uint16_t kC1 = 52845u;
        static constexpr uint16_t kC2 = 22719u;

        std::shared_ptr<PSFile> _source;
        std::vector<uint8_t> _decoded;

        bool _finished = false;
        bool _valid = false;

    public:
        explicit EExecDecodeFilter(std::shared_ptr<PSFile> source)
            : _source(std::move(source))
        {
            if (!_source || !_source->isValid())
                return;

            if (!decode())
                return;

            fCursor = OctetCursor(_decoded.data(), _decoded.size());
            _valid = true;
        }

        bool hasCursor() const override
        {
            return true;
        }

        size_t size() const override
        {
            return _decoded.size();
        }

        bool isValid() const override
        {
            return _valid;
        }

        bool readByte(uint8_t& out) override
        {
            if (_finished || fCursor.empty())
                return false;

            out = *fCursor;
            ++fCursor;
            return true;
        }

        bool readBytes(uint8_t* out, size_t count) override
        {
            if (_finished || fCursor.size() < count)
                return false;

            std::memcpy(out, fCursor.begin(), count);
            fCursor.advance(count);
            return true;
        }

        size_t position() const override
        {
            if (_decoded.empty())
                return 0;

            return static_cast<size_t>(fCursor.begin() - _decoded.data());
        }

        bool setPosition(size_t pos) override
        {
            if (pos > _decoded.size())
                return false;

            fCursor = OctetCursor(_decoded.data() + pos, _decoded.size() - pos);
            _finished = false;
            return true;
        }

        void rewind() override
        {
            fCursor = OctetCursor(_decoded.data(), _decoded.size());
            _finished = false;
        }

        bool isEOF() const override
        {
            return _finished || fCursor.empty();
        }

        void finalize() override
        {
            _finished = true;

            // Terminate execution of this filtered file immediately.
            // Do not finalize the underlying source: the outer PostScript
            // file must resume after the encrypted block.
            fCursor = OctetCursor();
        }

    private:
        static bool isWhitespace(uint8_t ch) noexcept
        {
            return ch == 0 || ch == 9 || ch == 10 || ch == 12 || ch == 13 || ch == 32;
        }

        static bool isHexDigit(uint8_t ch) noexcept
        {
            return (ch >= '0' && ch <= '9') ||
                (ch >= 'A' && ch <= 'F') ||
                (ch >= 'a' && ch <= 'f');
        }

        static uint8_t hexValue(uint8_t ch) noexcept
        {
            if (ch >= '0' && ch <= '9')
                return static_cast<uint8_t>(ch - '0');

            if (ch >= 'A' && ch <= 'F')
                return static_cast<uint8_t>(ch - 'A' + 10);

            if (ch >= 'a' && ch <= 'f')
                return static_cast<uint8_t>(ch - 'a' + 10);

            return 0xFF;
        }

        bool decode()
        {
            uint16_t key = kInitialKey;
            uint32_t discard = 4;

            _decoded.clear();

            while (true)
            {
                const size_t pairStart = _source->position();

                uint8_t hi;
                uint8_t lo;

                if (!readHexDigit(hi))
                    break;

                if (!readHexDigit(lo))
                {
                    // The first character may itself have been a valid hex digit
                    // belonging to the following PostScript token, as in
                    // "cleartomark". Restore the source to before that pair.
                    if (!_source->setPosition(pairStart))
                        return false;

                    break;
                }

                const uint8_t cipher = static_cast<uint8_t>((hi << 4) | lo);
                const uint8_t plain = static_cast<uint8_t>(cipher ^ (key >> 8));

                // BUGBUG
                //printf("%c", plain);

                key = static_cast<uint16_t>((static_cast<uint32_t>(cipher + key) * kC1 + kC2) & 0xFFFFu);

                if (discard != 0)
                {
                    --discard;
                    continue;
                }

                _decoded.push_back(plain);
            }

            // print the decoded data to stdout for debugging
            //for (uint8_t b : _decoded)
            //    printf("%c", b);

            return discard == 0 && !_decoded.empty();
        }

        bool readHexDigit(uint8_t& value)
        {
            uint8_t ch;

            while (_source->readByte(ch))
            {
                if (isWhitespace(ch))
                    continue;

                if (!isHexDigit(ch))
                    return false;

                value = hexValue(ch);
                return true;
            }

            return false;
        }
    };
}


