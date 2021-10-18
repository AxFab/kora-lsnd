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
#include "snd-io.h"
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#ifndef O_BINARY
# define O_BINARY 0
#endif
typedef struct wav_header wav_header_t;

struct wav_header {
    uint32_t chunk_id;
    uint32_t chunk_size;
    uint32_t format;
    uint32_t fmt_chunk_id;
    uint32_t fmt_chunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint32_t data_chunk_id;
    uint32_t data_chunk_size;
};

static int snd_wav_rewind(snd_stream_t* strm, int ms)
{
    int fd = (int)strm->data;
    int len = strm->fmt.sample_rate / 100 * strm->fmt.word_size * strm->fmt.channel;
    strm->position = ms / 10;
    lseek(fd, sizeof(wav_header_t) + strm->position * len, SEEK_SET);
    return -1;
}

static int snd_wav_wr_close(snd_stream_t* strm)
{
    wav_header_t header;
    header.chunk_id = 0x46464952;
    header.chunk_size = strm->length + sizeof(wav_header_t);
    header.format = 0x45564157;
    header.fmt_chunk_id = 0x20746d66;
    header.fmt_chunk_size = 16;
    header.audio_format = 1;
    header.num_channels = strm->fmt.channel;
    header.sample_rate = strm->fmt.sample_rate;
    header.byte_rate = strm->fmt.sample_rate * strm->fmt.channel * strm->fmt.word_size;
    header.block_align = strm->fmt.channel * strm->fmt.word_size;
    header.bits_per_sample = strm->fmt.word_size * 8;
    header.data_chunk_id = 0x61746164;
    header.data_chunk_size = strm->length;

    int fd = (int)strm->data;
    lseek(fd, 0, SEEK_SET);
    write(fd, &header, sizeof(header));
    close(fd);
    return 0;
}

int snd_wav_open_input(snd_stream_t* strm, const char* path)
{
    strm->data = NULL;
    int fd = open(path, O_RDONLY | O_BINARY);
    if (fd < 0)
        return -1;

    wav_header_t header;
    read(fd, &header, sizeof(header));
    if (header.chunk_id != 0x46464952 || // "RIFF"
        header.format != 0x45564157 ||  // "WAVE"
        header.fmt_chunk_id != 0x20746d66 || // "fmt "
        header.data_chunk_id != 0x61746164) { // "data"
        close(fd);
        return -1;
    }

    strm->data = (void*)fd;
    strm->length = header.data_chunk_size;
    strm->position = 0;
    strm->fmt.sample_rate = header.sample_rate;
    strm->fmt.channel = header.num_channels;
    strm->fmt.word_size = header.bits_per_sample / 8;
    strm->read = snd_fd_read;
    strm->write = NULL;
    strm->rewind = snd_wav_rewind;
    strm->close = snd_fd_close;
    return 0;
}

int snd_wav_open_output(snd_stream_t* strm, const char* path, snd_format_t* format)
{
    strm->data = NULL;
    int fd = open(path, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        return -1;

    wav_header_t header;
    header.chunk_id = 0x46464952;
    header.chunk_size = 0;
    header.format = 0x45564157;
    header.fmt_chunk_id = 0x20746d66;
    header.fmt_chunk_size = 0;
    header.audio_format = 1;
    header.num_channels = format->channel;
    header.sample_rate = format->sample_rate;
    header.byte_rate = format->sample_rate * format->channel * format->word_size;
    header.block_align = format->channel * format->word_size;
    header.bits_per_sample = format->word_size * 8;
    header.data_chunk_id = 0x61746164;
    header.data_chunk_size = 0;

    write(fd, &header, sizeof(header));
    strm->data = (void*)fd;
    strm->length = 0;
    strm->position = 0;
    strm->fmt = *format;
    strm->read = NULL;
    strm->write = snd_fd_write;
    strm->rewind = snd_wav_rewind;
    strm->close = snd_wav_wr_close;
    return 0;
}
