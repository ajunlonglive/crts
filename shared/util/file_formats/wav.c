#include "posix.h"

#include <string.h>
#include <stdlib.h>

#include "shared/util/assets.h"
#include "shared/util/file_formats/wav.h"
#include "shared/util/log.h"
#include "shared/util/mem.h"

enum chunk_type {
	chunk_type_riff,
	chunk_type_fmt,
	chunk_type_data,
	chunk_type_unknown,
};

struct chunk_hdr {
	enum chunk_type type;
	uint32_t size;
};

struct wav_format {
	uint16_t wFormatTag;
	uint16_t nChannels;
	uint32_t nSamplesPerSec;
	uint32_t nAvgBytesPerSec;
	uint16_t nBlockAlign;
	uint16_t wBitsPerSample;
	/* uint16_t cbSize; */
	/* uint16_t wValidBitsPerSample; */
	/* uint32_t dwChannelMask; */
	/* uint8_t SubFormat[16]; */
};
_Static_assert(sizeof(struct wav_format) == 16, "");

static void
wav_err(const char *path, const char *err)
{
	LOG_W(log_misc, "error loading '%s': %s", path, err);
}

static void
parse_chunk_hdr(struct file_data *fd, struct chunk_hdr *hdr, uint32_t *bufi)
{
	static const char *chunk_type_tbl[] = {
		[chunk_type_riff] = "RIFF",
		[chunk_type_fmt]  = "fmt ",
		[chunk_type_data] = "data",
	};

	for (hdr->type = 0; hdr->type < chunk_type_unknown; ++hdr->type) {
		if (memcmp(chunk_type_tbl[hdr->type], &fd->data[*bufi], 4) == 0) {
			break;
		}
	}
	*bufi += 4;

	memcpy(&hdr->size, &fd->data[*bufi], 4);
	*bufi += 4;
}

static bool
parse_fmt_chunk(struct file_data *fd, struct chunk_hdr *hdr, struct wav_format *fmt, uint32_t *bufi)
{
	if (hdr->size < 16) {
		wav_err(fd->path, "fmt chunk is too small");
		return false;
	}

	memcpy(fmt, &fd->data[*bufi], 16);
	*bufi += hdr->size;

	switch (fmt->wFormatTag) {
	case 0x0001:  // WAVE_FORMAT_PCM PCM
		break;
	case 0x0003:  // WAVE_FORMAT_IEEE_FLOAT IEEE float
		wav_err(fd->path, "WAVE_FORMAT_IEEE_FLOAT not supported");
		return false;
	case 0x0006:  // WAVE_FORMAT_ALAW 8 - bit ITU - T G .711 A - law
		wav_err(fd->path, "WAVE_FORMAT_ALAW not supported");
		return false;
	case 0x0007:  // WAVE_FORMAT_MULAW 8 - bit ITU - T G .711 µ - law
		wav_err(fd->path, "WAVE_FORMAT_MULAW not supported");
		return false;
	case 0xFFFE:  // WAVE_FORMAT_EXTENSIBLE Determined by SubFormat
		wav_err(fd->path, "WAVE_FORMAT_EXTENSIBLE not supported");
		return false;
	}

	if (fmt->nChannels != 2) {
		wav_err(fd->path, "only 2 channels are supported");
		return false;
	} else if (fmt->wBitsPerSample != 16) {
		wav_err(fd->path, "only 16-bit samples are supported");
		return false;
	}

	/* fprintf(stderr, "wFormatTag: %d\n" */
	/* 	"nChannels: %d\n" */
	/* 	"nSamplesPerSec: %d\n" */
	/* 	"nAvgBytesPerSec: %d\n" */
	/* 	"nBlockAlign: %d\n" */
	/* 	"wBitsPerSample: %d\n", */
	/* 	fmt->wFormatTag, */
	/* 	fmt->nChannels, */
	/* 	fmt->nSamplesPerSec, */
	/* 	fmt->nAvgBytesPerSec, */
	/* 	fmt->nBlockAlign, */
	/* 	fmt->wBitsPerSample); */

	return true;
}

static bool
parse_data_chunk(struct file_data *fd, struct chunk_hdr *hdr, struct wav_format *fmt, struct wav *wav, uint32_t *bufi)
{
	assert(!wav->len);

	wav->len = hdr->size / 2; // 2 bytes per sample

	/* put an extra (empty) sample (+2 channels) at the end of the asset so
	 * that we always have another sample to blend with */
	wav->data = z_calloc(wav->len + 2, sizeof(double));

	uint32_t i, samplei = 0;
	int16_t *samples = (int16_t *)&fd->data[*bufi];

	for (i = 0; i < wav->len; ++i) {
		wav->data[i] = ((((double)samples[samplei] - (double)INT16_MIN) / (double)UINT16_MAX) * 2.0) - 1.0;

		++samplei;
	}

	*bufi += hdr->size;
	if (hdr->size & 0x1) {
		*bufi += 1;
	}

	return true;
}

bool
load_wav(const char *path, struct wav *wav)
{
	uint32_t bufi = 0;
	struct file_data *fd;
	struct chunk_hdr main = { 0 }, cur;
	struct wav_format fmt;

	if (!(fd = asset(path))) {
		return false;
	}

	parse_chunk_hdr(fd, &main, &bufi);

	if (main.type != chunk_type_riff) {
		wav_err(path, "missing RIFF tag");
		return false;
	} else if (main.size + 8 != fd->len) {
		wav_err(path, "wav size mismatch");
		return false;
	} else if (memcmp(&fd->data[bufi], "WAVE", 4) != 0) {
		wav_err(path, "WAVEID is not WAVE");
		return false;
	}
	bufi += 4;

	while (bufi < fd->len) {
		parse_chunk_hdr(fd, &cur, &bufi);

		switch (cur.type) {
		case chunk_type_riff:
			wav_err(path, "unexpected RIFF tag");
			return false;
		case chunk_type_fmt:
			if (!parse_fmt_chunk(fd, &cur, &fmt, &bufi)) {
				return false;
			}
			break;
		case chunk_type_data:
			if (wav->len) {
				L(log_misc, "ignoring additional data block");
				bufi += cur.size;
			} else if (!parse_data_chunk(fd, &cur, &fmt, wav, &bufi)) {
				return false;
			}
			break;
		case chunk_type_unknown:
			L(log_misc, "ignoring unknown chunk");
			break;
		}
	}

	return true;
}
