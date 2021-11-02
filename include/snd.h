/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2021  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 */
#ifndef _SND_H 
#define _SND_H 1

#include <stddef.h>
#include <stdbool.h>

#ifndef LIBAPI
# if defined(WIN32) || defined(_WIN32)
#  define LIBAPI __declspec(dllexport)
# else
#  define LIBAPI
# endif
#endif

typedef struct snd_buffer snd_buffer_t;
typedef struct snd_stream snd_stream_t;
typedef struct snd_format snd_format_t;

struct snd_format {
    int sample_rate;
    int channel;
    int word_size;
};


LIBAPI snd_buffer_t* snd_alloc_buffer(snd_format_t* format);
LIBAPI snd_buffer_t* snd_alloc_buffer_from(snd_stream_t* strm);
LIBAPI void snd_free_buffer(snd_buffer_t* buf);
LIBAPI snd_format_t *snd_format(snd_stream_t* strm);

LIBAPI snd_stream_t* snd_open_fd(int fd, snd_format_t* format);
LIBAPI snd_stream_t* snd_open_input(const char* path);
LIBAPI snd_stream_t* snd_open_output(const char* path, snd_format_t* format);

LIBAPI void snd_close(snd_stream_t* strm);
LIBAPI int snd_read(snd_stream_t* strm, snd_buffer_t* buf);
LIBAPI int snd_write(snd_stream_t* strm, snd_buffer_t* buf);
LIBAPI int snd_rewind(snd_stream_t* strm, int ms);

LIBAPI void snd_convert(snd_buffer_t* in, snd_buffer_t* out);
LIBAPI void snd_lowpass(snd_buffer_t* buf, double freq);
LIBAPI void snd_highpass(snd_buffer_t* buf, double freq);


LIBAPI snd_stream_t* snd_open_speaker(snd_format_t* format, int device);
LIBAPI int snd_volume(snd_stream_t* strm, int level);


LIBAPI int snd_play_raw(snd_format_t* format, bool* until, int(*producer)(void*, snd_buffer_t*), void* producer_arg, int(*consumer)(void*, snd_buffer_t*), void* consumer_arg);
LIBAPI int snd_play(snd_stream_t* input, snd_stream_t* output, bool loop, bool* until);
LIBAPI int snd_convert_and_play(snd_stream_t* input, snd_stream_t* output, bool loop, bool* until);
LIBAPI int snd_play_file(const char* filename, bool loop, bool* until);


// Ideas
LIBAPI int snd_gain(snd_buffer_t* buffer, int gain);
LIBAPI int snd_mix_buffer(snd_buffer_t* buffer, int count, ...);
LIBAPI int snd_mixer(snd_stream_t* output, int count, ...);
LIBAPI int snd_speed(snd_stream_t* input, snd_stream_t* output, float speed);
LIBAPI int snd_pitch(snd_stream_t* input, snd_stream_t* output, float pitch);

#endif /*_SND_H */
