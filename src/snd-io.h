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
#ifndef _SRC_SND_IO_H
#define _SRC_SND_IO_H 1

#include <snd.h>

struct snd_buffer { // 10ms
    snd_format_t fmt;
    int size;
    int len;
    void* ptr;
    double prevs[8]; // Save last values
};

struct snd_stream {
    snd_format_t fmt;
    int length;
    int position;

    // stream codec
    int (*read)(snd_stream_t* strm, snd_buffer_t* buf);
    int (*write)(snd_stream_t* strm, snd_buffer_t* buf);
    int (*rewind)(snd_stream_t* strm, int ms);
    int (*close)(snd_stream_t* strm);
    void* data;
};

double snd_read_value(snd_buffer_t* buf, int channel, int idx);
double snd_write_value(snd_buffer_t* buf, int channel, int idx, double val);

int snd_wav_open_input(snd_stream_t* strm, const char* path);
int snd_wav_open_output(snd_stream_t* strm, const char* path, snd_format_t* format);

int snd_fd_read(snd_stream_t* strm, snd_buffer_t* buf);
int snd_fd_write(snd_stream_t* strm, snd_buffer_t* buf);
int snd_fd_close(snd_stream_t* strm);


#endif /* _SRC_SND_IO_H */
