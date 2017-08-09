#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <linux/api.h>
#include <log.h>


inline static const char* parse_number(const char* string, const char* end, uint32_t number_ptr[restrict static 1]) {
	uint32_t number = 0;
	while (string != end) {
		const uint32_t digit = (uint32_t) (*string) - (uint32_t) '0';
		if (digit >= 10) {
			return string;
		}
		number = number * UINT32_C(10) + digit;
		string += 1;
	}
	*number_ptr = number;
	return end;
}

bool cpuinfo_linux_parse_small_file(const char* filename, size_t buffer_size, cpuinfo_smallfile_callback callback, void* context) {
	int file = -1;
	bool status = false;
	char* buffer = (char*) alloca(buffer_size);

	#if CPUINFO_LOG_DEBUG_PARSERS
		cpuinfo_log_debug("parsing small file %s", filename);
	#endif

	file = open(filename, O_RDONLY);
	if (file == -1) {
		cpuinfo_log_error("failed to open %s: %s", filename, strerror(errno));
		goto cleanup;
	}

	char* buffer_end = &buffer[buffer_size];
	size_t buffer_position = 0;
	ssize_t bytes_read;
	do {
		bytes_read = read(file, &buffer[buffer_position], buffer_size - buffer_position);
		if (bytes_read < 0) {
			cpuinfo_log_error("failed to read file %s at position %zu: %s", filename, buffer_position, strerror(errno));
			goto cleanup;
		}
		buffer_position += (size_t) bytes_read;
		if (buffer_position >= buffer_size) {
			cpuinfo_log_error("failed to read file %s: insufficient buffer of size %zu", filename, buffer_size);
			goto cleanup;
		}
	} while (bytes_read != 0);

	status = callback(buffer, &buffer[buffer_position], context);

cleanup:
	if (file != -1) {
		close(file);
		file = -1;
	}
	return status;
}
