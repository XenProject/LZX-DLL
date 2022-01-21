#include "stdafx.h"
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include "lzx.h"
#include "mspack.h"
#include "system.h"
extern "C"
{
	inline bool bit_scan_forward(uint32_t v, uint32_t* out_first_set_index) {
		return _BitScanForward(reinterpret_cast<unsigned long*>(out_first_set_index), v) != 0;
	}
	typedef struct mspack_memory_file_t {
		mspack_system sys;
		void* buffer;
		off_t buffer_size;
		off_t offset;
	} mspack_memory_file;

	mspack_memory_file* mspack_memory_open(mspack_system* sys, void* buffer, const size_t buffer_size) {
		//assert_true(buffer_size < INT_MAX);
		if (buffer_size >= INT_MAX) {
			return NULL;
		}
		auto memfile = (mspack_memory_file*)malloc(sizeof(mspack_memory_file));
		if (!memfile) {
			return NULL;
		}
		memfile->buffer = buffer;
		memfile->buffer_size = (off_t)buffer_size;
		memfile->offset = 0;
		return memfile;
	}

	void mspack_memory_close(mspack_memory_file* file) {
		auto memfile = (mspack_memory_file*)file;
		free(memfile);
	}

	int mspack_memory_read(mspack_file* file, void* buffer, int chars) {
		auto memfile = (mspack_memory_file*)file;
		const off_t remaining = memfile->buffer_size - memfile->offset;
		const off_t total = min(static_cast<off_t>(chars), remaining);
		memcpy(buffer, (uint8_t*)memfile->buffer + memfile->offset, total);
		memfile->offset += total;
		return (int)total;
	}

	int mspack_memory_write(mspack_file* file, void* buffer, int chars) {
		auto memfile = (mspack_memory_file*)file;
		const off_t remaining = memfile->buffer_size - memfile->offset;
		const off_t total = min(static_cast<off_t>(chars), remaining);
		memcpy((uint8_t*)memfile->buffer + memfile->offset, buffer, total);
		memfile->offset += total;
		return (int)total;
	}

	void* mspack_memory_alloc(mspack_system* sys, size_t chars) {
		return malloc(chars);
	}

	void mspack_memory_free(void* ptr) { free(ptr); }

	void mspack_memory_copy(void* src, void* dest, size_t chars) {
		memcpy(dest, src, chars);
	}

	mspack_system* mspack_memory_sys_create() {
		auto sys = (mspack_system*)malloc(sizeof(mspack_system));
		if (!sys) {	return NULL; }
		sys->read = mspack_memory_read;
		sys->write = mspack_memory_write;
		sys->alloc = mspack_memory_alloc;
		sys->free = mspack_memory_free;
		sys->copy = mspack_memory_copy;
		return sys;
	}

	void mspack_memory_sys_destroy(struct mspack_system* sys) { free(sys); }

	typedef union XeVersion {
		uint32_t Value;
		struct {
			uint32_t Major : 4;
			uint32_t Minor : 4;
			uint32_t Build : 16;
			uint32_t QFE : 8;
		} version_t;
	} xe_version_t;

    __declspec(dllexport) unsigned int Version ()
    {
        xe_version_t* ver_t = (xe_version_t*)malloc(sizeof(xe_version_t));

		ver_t->version_t.Major = 0;
		ver_t->version_t.Minor = 0;
		ver_t->version_t.Build = 944;
		ver_t->version_t.QFE = 0;
		
		unsigned int buf = ver_t->Value;
		free(ver_t);
		return buf;
    }
    __declspec(dllexport) int Decompress(BYTE* CompData, int CDSize, BYTE* OutputData, int ODSize, unsigned int WindowSize)
    {
		int result_code = 1;

		uint32_t window_bits;
		if (!bit_scan_forward(WindowSize, &window_bits)) {
			return result_code;
		}
		mspack_system* sys = mspack_memory_sys_create();
		mspack_memory_file* lzxsrc = mspack_memory_open(sys, (void*)CompData, CDSize);
		mspack_memory_file* lzxdst = mspack_memory_open(sys, (void*)OutputData, ODSize);
		lzxd_stream* lzxd = lzxd_init(sys, (mspack_file*)lzxsrc, (mspack_file*)lzxdst, window_bits, 0, 0x8000, (off_t)ODSize, 0);

		result_code = lzxd_decompress(lzxd, (off_t)ODSize);

	    lzxd_free(lzxd);
		lzxd = NULL;

		if (lzxsrc) {
			mspack_memory_close(lzxsrc);
			lzxsrc = NULL;
		}

		if (lzxdst) {
		    mspack_memory_close(lzxdst);
			lzxdst = NULL;
		}

		if (sys) {
			mspack_memory_sys_destroy(sys);
			sys = NULL;
		}
		return 0;
	}
}