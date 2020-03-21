#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <stdio.h>
#include "audiere/audiere.h"

#undef getc

namespace pp { class Instance; }

namespace verge {
    typedef std::vector<char> DataVec;

    enum class FileMode {
        None = 0,
        Read,
        Write,
    };

    struct File : public audiere::File {
        explicit File(DataVec data);

        void open(FileMode mode);
        void close();
        int getc();
        void ungetc();

        size_t read(void* dest, size_t length);
        int seek(long int offset, int origin);
        int tell() override;

        size_t write(const void* src, size_t length);

        std::string getData();

        // audiere::File implementation
        void ref() override;
        void unref() override;
        int read(void* dest, int length) override;
        bool seek(int position, audiere::File::SeekMode seekMode) override;

    private:
        FileMode mode;
        DataVec data;
        size_t pos;
    };
    using FilePtr = std::shared_ptr<File>;

    struct FS {
        friend class File;

        FilePtr open(const std::string& filename, FileMode mode);

        void set(const std::string& filename, DataVec data);

    private:
        std::map<std::string, FilePtr> files;
    };

    // API
    typedef File VFILE;

    void vset(const std::string& filename, DataVec data);

    VFILE* vopen(const char* fname, const char* mode);
    void vclose(VFILE* f);
    size_t vread(void* dest, size_t size, size_t length, VFILE* f);
    size_t vwrite(const void* src, size_t size, size_t length, VFILE* f);
    char* vgets(char* dest, int num, VFILE* file);
    int vgetc(VFILE* file);
    int vseek(VFILE* file, long int offset, int origin);
    int vtell(VFILE* file);

    namespace impl {
        bool iswhite(char c);
        int scan(VFILE* file, int* value);
        int scan(VFILE* file, char* value);

        template <int N>
        int scan(VFILE* file, char (*value)[N]) {
            return scan(file, (char*)value);
        }
    }

    template <typename T, typename... Args>
    int vscanf(VFILE* file, const char* format, T value, Args... args) {
        int count = 0;

        while (format[count]) {
            if (format[count] == '%' && format[count + 1] != '%') {
                do {
                    auto c = file->getc();
                    if (c == -1) {
                        return count;
                    } else if (impl::iswhite(c)) {
                        count++;
                    } else {
                        file->ungetc();
                        break;
                    }
                } while (true);

                count += 2 + impl::scan(file, value);
                return vscanf(file, format + count, args...);
            }
            ++count;
        }
        return count;
    }

    inline int vscanf(VFILE* file, const char* format) {
        // nothing more to do
        return 0;
    }
}
