// Accessor for GMV
#ifndef GOB_GMV_FILE_HPP
#define GOB_GMV_FILE_HPP

#include <SdFat.h>
#include "gob_gmv.hpp"

namespace gob
{
class GMVFile
{
  public:
    GMVFile() {}
    ~GMVFile() { close(); }

    inline uint32_t blocks() const { return _header.blocks; }
    inline uint32_t readCount() const { return _current ? _current - 1 : 0; }
    inline bool eof() const { return _file && _current >= blocks(); }
    explicit inline operator bool() const { return (bool)_file; }
    const gob::wav_header_t& wavHeader() const { return _header.wavHeader; }
    uint32_t fps() const { return (uint32_t)_header.fps; }
    uint32_t imageSize() const { return _size[0]; }
    uint32_t wavSize() const { return _size[1]; }
    
    bool open(const String& path) { return open(path.c_str()); }
    bool open(const char* path)
    {
        _header = {};
        _current = _blockHead = 0;
        close();
        
        if(!_file.open(path) ||
           _file.read(&_header, sizeof(_header)) != sizeof(_header) ||
           _header.signature != GMVHeader::Signature)
        {
            return false;
        }
        _blockHead = _file.position();
        return true;
    }

    void close() { if(_file) { _file.close(); } }

    // 
    uint32_t readBlock(uint8_t* buf, uint32_t sz)
    {
        if(!_file || eof()) { return 0; }
        ++_current;
        _size[0] = _size[1] = 0;
        if(_file.read(_size, sizeof(_size)) != sizeof(_size)) { return 0; }

        uint32_t len = _size[0] + _size[1];
        uint32_t skip{};
        if(len > sz) { skip = len - sz; }
        auto r = _file.read(buf, std::min<uint32_t>(sz, len));
        if(skip)
        {
            _file.seek(_file.position() + skip);
        }
        return r >= _size[0] ? _size[0] : r; // Read image size
    }

    bool rewind()
    {
        _current = 0;
        return _file ? _file.seek(_blockHead) : false;
    }

  private:
    FsFile _file{};
    GMVHeader _header{};
    uint32_t _current{};
    uint32_t _blockHead{}; // Head of block
    uint32_t _size[2]; // 0:image 1:wav
};
//
}
#endif
