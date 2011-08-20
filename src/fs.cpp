
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
        mode = mode;
    }

    void File::close() {
        mode = FileMode::None;
    }

    char File::getc() {
        if (pos < data.size()) {
            return data[pos++];
        } else {
            return 0;
        }
    }

    void File::ungetc() {
        if (pos > 0) {
            --pos;
        }
    }

    size_t File::read(void* dest, size_t length) {
        auto s = std::min(pos + length, data.size());
        auto p = static_cast<char*>(dest);
        std::copy(&data[pos], &data[s], p);
        pos = s;
        return s;
    }

    int File::seek(long int origin, int offset) {
        switch (offset) {
            case SEEK_SET: pos = size_t(std::max<int>(0, origin)); break;
            case SEEK_CUR: pos += origin; break;
            case SEEK_END: pos = size_t(std::max<int>(0, data.size() - origin)); break;
        }
        if (pos > data.size()) {
            pos = 0;
            return 1;
        } else {
            return 0;
        }
    }

    int File::tell() {
        return pos;
    }

    /*
    FS::FS(pp::Instance* instance)
        : instance(instance)
    { }
    */

    FilePtr FS::open(const std::string& filename, FileMode mode) {
        assert(mode == FileMode::Read && "Writing is not yet implemented");

        auto fn = toLower(filename);

        if (files.count(fn)) {
            auto f = files[fn];
            f->open(mode);
            return f;
        }

        return FilePtr();
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
        } else {
            assert(!"Unknown mode");
        }

        return fs.open(fname, m).get(); // cheating.  Sorta.
    }

    void vclose(VFILE* f) {
        f->close();
    }

    size_t vread(void* dest, size_t size, size_t length, VFILE* f) {
        return f->read(dest, size * length);
    }

    size_t vwrite(const void* src, size_t size, size_t length, VFILE* f) {
        return 0;//f->write(src, size * length);
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