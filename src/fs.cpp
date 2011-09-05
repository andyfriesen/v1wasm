
#include "fs.h"
#include <algorithm>
#include <cstdio>
#include <cassert>
#include <cctype>

extern verge::VFILE* pcxf;

namespace verge {

    namespace {
        std::string toLower(std::string s) {
            for (size_t i = 0; i < s.length(); ++i) {
                s[i] = tolower(s[i]);
            }
            return s;
        }
    }

    File::File(DataVec data)
        : mode(FileMode::None)
        , data(data)
        , pos(0)
    {
    }

    void File::open(FileMode m) {
        pos = 0;
        mode = m;
    }

    void File::close() {
        mode = FileMode::None;
    }

    char File::getc() {
        assert(mode == FileMode::Read);

        if (pos < data.size()) {
            return data[pos++];
        } else {
            return 0;
        }
    }

    void File::ungetc() {
        assert(mode == FileMode::Read);

        if (pos > 0) {
            --pos;
        }
    }

    size_t File::read(void* dest, size_t length) {
        assert(mode == FileMode::Read);

        auto s = std::min(pos + length, data.size());
        auto p = static_cast<char*>(dest);
        std::copy(&data[pos], &data[s], p);
        pos = s;
        return s;
    }

    int File::seek(long int origin, int offset) {
        assert(mode != FileMode::None);

        switch (offset) {
            case SEEK_SET: pos = size_t(std::max<int>(0, origin)); break;
            case SEEK_CUR: pos += origin; break;
            case SEEK_END: pos = size_t(std::max<int>(0, data.size() + origin)); break;
        }
        if (pos > data.size()) {
            pos = 0;
            return 1;
        } else {
            return 0;
        }
    }

    int File::tell() {
        assert(mode != FileMode::None);

        return pos;
    }

    size_t File::write(const void* src, size_t length) {
        assert(mode == FileMode::Write);

        auto p(static_cast<const char*>(src));

        auto newSize = std::max(pos + length, data.size());
        data.reserve(newSize);
        data.insert(data.begin() + pos, p, p + length);
        pos += length;

        return length;
    }

    std::string File::getData() {
        std::string result(data.begin(), data.end());
        return result;
    }

    // audiere::File implementation

    void File::ref() {
    }

    void File::unref() {
    }

    int File::read(void* dest, int length) {
        return read(dest, size_t(length));
    }

    bool File::seek(int position, audiere::File::SeekMode seekMode) {
        int origin = seekMode == audiere::File::BEGIN ? SEEK_SET
            : seekMode == audiere::File::END ? SEEK_END
            : SEEK_CUR
            ;
        long int offset = position;
        return 0 == seek(offset, origin);
    }

    //

    FilePtr FS::open(const std::string& filename, FileMode mode) {
        auto fn = toLower(filename);
        FilePtr f;

        if (mode == FileMode::Write) {
            f.reset(new File(DataVec()));
            files[fn] = f;
        } else if (files.count(fn)) {
            f = files[fn];
        }

        if (f) {
            f->open(mode);
        }

        return f;
    }

    void FS::set(const std::string& filename, DataVec data) {
        auto fn = toLower(filename);
        files[fn].reset(new File(data));
    }

    namespace {
        FS fs;
    }

    void vset(const std::string& filename, DataVec data) {
        fs.set(filename, data);
    }

    VFILE* vopen(const char* fname, const char* mode) {
        auto m = FileMode::None;
        if (*mode == 'r') {
            m = FileMode::Read;
        } else if (*mode == 'w') {
            m = FileMode::Write;
        } else {
            assert(!"Unknown mode");
        }

        return fs.open(fname, m).get(); // cheating.  Sorta.
    }

    void vclose(VFILE* f) {
        assert(f && "Attempt to close 0 handle");
        f->close();
    }

    size_t vread(void* dest, size_t size, size_t length, VFILE* f) {
        return f->read(dest, size * length);
    }

    char* vgets(char* dest, int num, VFILE* file) {
        auto d = dest;
        do {
            auto c = file->getc();
            if (c == 0 || c == '\n') {
                *d = 0;
                break;
            } else {
                *d++ = c;
            }
        } while (true);

        return dest;
    }

    char vgetc(VFILE* f) {
        return f->getc();
    }

    int vseek(VFILE* f, long int offset, int origin) {
        return f->seek(offset, origin);
    }

    int vtell(VFILE* f) {
        return f->tell();
    }

    size_t vwrite(const void* ptr, size_t size, size_t count, VFILE* stream) {
        return stream->write(ptr, size * count);
    }

    namespace impl {
        bool iswhite(char c) {
            return c == ' '
                || c == '\t'
                || c == '\n'
                || c == '\r'
                ;
        }

        int scan(VFILE* file, int* value) {
            auto count = 0;
            auto negative = false;

            *value = 0;

            char c = file->getc();
            if (c == 0) {
                return 0;
            }

            if (c == '-') {
                ++count;
                negative = true;
            }
            file->ungetc();

            while (true) {
                c = file->getc();
                if (c >= '0' && c <= '9') {
                    *value = *value * 10 + (c - '0');
                    ++count;
                } else {
                    break;
                }
            }

            return negative ? -count : count;
        }

        int scan(VFILE* file, char* value) {
            int count = 0;

            do {
                auto c = file->getc();
                if (c == 0) {
                    break;
                } else if (!iswhite(c)) {
                    *value++ = c;
                    continue;
                } else {
                    file->ungetc();
                    break;
                }
            } while (true);
            *value = 0;
            return count;
        }
    }
}
