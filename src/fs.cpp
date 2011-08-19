
#include "fs.h"
#include <algorithm>
#include <cstdio>
#include <cassert>

namespace verge {

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
        std::copy(&data[pos], &data[pos + s], p);
        pos += s;
        return s;
    }

    /*
    FS::FS(pp::Instance* instance)
        : instance(instance)
    { }
    */

    FilePtr FS::open(const std::string& filename, FileMode mode) {
        assert(mode == FileMode::Read && "Writing is not yet implemented");

        if (files.count(filename)) {
            auto f = files[filename];
            f->open(mode);
            return f;
        }

        return FilePtr();
    }

    void FS::set(const std::string& filename, DataVec data) {
        files[filename].reset(new File(data));
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
            auto v = value;

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

            //printf("Scanned string '%s'\n", v);
            return count;
        }
    }
}
