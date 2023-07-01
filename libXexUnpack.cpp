#include <cstdarg>

#include "libXexUnpack.h"
#include <iostream>
#include "mspack/lzx.h"
#include "mspack/mspack.h"

typedef struct _mem_buf {
    void *data;
    size_t length;
    const char *name;
} mem_buf;

typedef struct _mem_file {
    unsigned char *data;
    size_t length;
    size_t posn;
    const char *name;
} mem_file;

static void *mem_alloc(struct mspack_system *self, size_t bytes) {
    return malloc(bytes);
}

static void mem_free(void *buffer) {
    free(buffer);
}

static void mem_copy(void *src, void *dest, size_t bytes) {
    memcpy(dest, src, bytes);
}

static void mem_msg(mem_file *file, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    fputc((int) '\n', stderr);
    fflush(stderr);
}

static mem_file *mem_open(struct mspack_system *self, mem_buf *fn, int mode) {
    mem_file *fh;
    if (fn == nullptr || fn->data == nullptr || fn->length == 0) {
        return nullptr;
    }
    if ((fh = (mem_file *) mem_alloc(self, sizeof(mem_file)))) {
        fh->data = (unsigned char *) fn->data;
        fh->length = fn->length;
        fh->posn = (mode == MSPACK_SYS_OPEN_APPEND) ? fn->length : 0;
        fh->name = fn->name;
    }
    return fh;
}

static void mem_close(mem_file *fh) {
    if (fh == nullptr) {
        return;
    }
    mem_free(fh);
}

static int mem_read(mem_file *fh, void *buffer, int bytes) {
    if (fh == nullptr || buffer == nullptr || bytes < 0) {
        return -1;
    }
    int todo = fh->length - fh->posn;
    if (todo > bytes) {
        todo = bytes;
    }
    if (todo > 0) {
        mem_copy(&fh->data[fh->posn], buffer, (size_t) todo);
    }
    fh->posn += todo;
    return todo;
}

static int mem_write(mem_file *fh, void *buffer, int bytes) {
    if (fh == nullptr || buffer == nullptr || bytes < 0) {
        return -1;
    }
    int todo = fh->length - fh->posn;
    if (todo > bytes) {
        todo = bytes;
    }
    if (todo > 0) {
        mem_copy(buffer, &fh->data[fh->posn], (size_t) todo);
    }
    fh->posn += todo;
    return todo;
}

static int mem_seek(mem_file *fh, off_t offset, int mode) {
    if (fh == nullptr) {
        return 1;
    }
    switch (mode) {
        case MSPACK_SYS_SEEK_START:
            break;
        case MSPACK_SYS_SEEK_CUR:
            offset += (off_t) fh->posn;
            break;
        case MSPACK_SYS_SEEK_END:
            offset = (off_t) fh->length;
            break;
        default:
            return 1;
    }
    if ((offset < 0) || (offset > (off_t) fh->length)) {
        return 1;
    }
    fh->posn = (size_t) offset;
    return 0;
}

static off_t mem_tell(mem_file *fh) {
    return (fh == nullptr) ? -1 : (off_t) fh->posn;
}

static struct mspack_system mem_system = {
        (struct mspack_file *(*)(struct mspack_system *, const char *, int)) &mem_open,
        (void (*)(struct mspack_file *)) &mem_close,
        (int (*)(struct mspack_file *, void *, int)) &mem_read,
        (int (*)(struct mspack_file *, void *, int)) &mem_write,
        (int (*)(struct mspack_file *, off_t, int)) &mem_seek,
        (off_t(*)(struct mspack_file *)) &mem_tell,
        (void (*)(struct mspack_file *, const char *, ...)) &mem_msg,
        &mem_alloc,
        &mem_free,
        &mem_copy,
        nullptr
};

int getBitSize(uint32_t winSize) {
    switch (winSize) {
        case 0x8000:
            return 15;
        case 0x10000:
            return 16;
        case 0x20000:
            return 17;
        case 0x40000:
            return 18;
        case 0x80000:
            return 19;
        case 0x100000:
            return 20;
        case 0x200000:
            return 21;
    }
    return 0;
}

uint16_t getBe16(void* inval) {
    unsigned char *data = (unsigned char *) inval;
    uint16_t result = (data[0] & 0xFF) << 8;
    result |= (data[1] & 0xFF);
    return result;
}

uint32_t getBe32(void* inval) {
    unsigned char *data = (unsigned char *) inval;
    uint32_t result = (data[0] & 0xFF) << 24;
    result |= (data[1] & 0xFF) << 16;
    result |= (data[2] & 0xFF) << 8;
    result |= (data[3] & 0xFF);
    return result;
}

int unpack(uint8_t* pInData, uint32_t inLen, uint8_t* pOutData, uint32_t outLen, uint32_t windowSize) {
    int result = 1;
    struct mspack_system *sys = &mem_system;
    struct lzxd_stream *context = nullptr;

    mem_buf source = {NULL, 0, "source"};
    mem_buf output = {NULL, 0, "output"};
    mem_file *in, *out;

    source.data = pInData;
    source.length = inLen;
    output.data = pOutData;
    output.length = outLen;
    in = (mem_file *) sys->open(sys, (const char *) &source, MSPACK_SYS_OPEN_READ);
    out = (mem_file *) sys->open(sys, (const char *) &output, MSPACK_SYS_OPEN_WRITE);

    context = lzxd_init(sys, (struct mspack_file *) in, (struct mspack_file *) out, getBitSize(windowSize), 0, windowSize, (off_t) outLen, 0);
    if (context != nullptr) {
        int stLzx = lzxd_decompress(context, (off_t) outLen);
        if (stLzx == MSPACK_ERR_OK) {
            result = 1;
        }
        lzxd_free(context);
    } else {
        result = 0;
    }
    sys->close((struct mspack_file *) in);
    sys->close((struct mspack_file *) out);
    return result;
}

uint8_t* optimizePackData(uint8_t* pInData, uint32_t* finSiz, uint32_t inLen, uint32_t firstSize) {
    uint8_t *buffer = (uint8_t *) malloc(inLen);
    *finSiz = 0;
    if (buffer != nullptr) {
        uint32_t currLen = firstSize;
        uint32_t nextLen = 1;
        uint32_t compressedSz = 0;
        uint32_t currPos = 0;
        uint32_t lastPos = 0;
        while ((currPos < inLen) && (nextLen != 0)) {
            uint32_t innerLen = 1;
            nextLen = getBe32(&pInData[currPos]);
            currPos += 0x18;
            do {
                innerLen = getBe16(&pInData[currPos]);
                if (innerLen) {
                    currPos += 2;
                    memcpy(&buffer[compressedSz], &pInData[currPos], innerLen);
                    currPos += innerLen;
                    compressedSz += innerLen;
                }
            } while (innerLen);
            currPos = lastPos + currLen;
            currLen = nextLen;
            lastPos = currPos;
        }
        *finSiz = compressedSz;
        return buffer;
    }
    return nullptr;
}

TR_EXPORT int UnpackXexData(uint8_t* inputData, uint32_t inputSize, uint8_t* outputData, uint32_t outputDataSize, uint32_t windowSize, uint32_t firstDataSize) {
    int result = 0;
    uint8_t *packedData;
    uint32_t packedDataSize;
    packedData = optimizePackData(inputData, (uint32_t *) &packedDataSize, inputSize, firstDataSize);
    if (packedData != nullptr) {
        result = unpack(packedData, packedDataSize, outputData, outputDataSize, windowSize);
        free(packedData);
    }
    return result;
}