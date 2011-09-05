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
        char getc();
        void ungetc();

        size_t read(void* dest, size_t length);
        int seek(long int offset, int origin);
        int tell();

        size_t write(const void* src, size_t length);

        std::string getData();

        // audiere::File implementation
        virtual void ref();
        virtual void unref();
        virtual int read(void* dest, int length);
        virtual bool seek(int position, audiere::File::SeekMode seekMode);

    private:
        FileMode mode;
        DataVec data;
        size_t pos;
    };
    typedef std::shared_ptr<File> FilePtr;

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
    char vgetc(VFILE* file);
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
                    if (c == 0) {
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
