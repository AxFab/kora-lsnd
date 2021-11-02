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
#include <string.h>

static int _snd_read_loop(snd_stream_t* stream, snd_buffer_t* buffer)
{
    int ret = snd_read(stream, buffer);
    if (ret != 0) {
        snd_rewind(stream, 0);
        ret = snd_read(stream, buffer);
        if (ret != 0)
            return -1;
    }
}

static int _snd_write_convert(void** params, snd_buffer_t* buf_in)
{
    snd_stream_t* output = params[0];
    snd_buffer_t* buf_out = params[1];
    snd_convert(buf_in, buf_out);
    snd_lowpass(buf_out, buf_in->fmt.sample_rate / 2);
    snd_write(output, buf_out);
}

int snd_play_raw(snd_format_t* format, bool* until, int(*producer)(void*, snd_buffer_t*), void* producer_arg, int(*consumer)(void*, snd_buffer_t*), void* consumer_arg)
{
    int ret = 0;
    snd_buffer_t* buffer = snd_alloc_buffer(format);
    for (int count = 0; until == NULL || *until; ++count) {
        ret = producer(producer_arg, buffer);
        if (ret != 0)
            break;
        ret = consumer(consumer_arg, buffer);
        if (ret != 0)
            break;
    }
    snd_free_buffer(buffer);
    return ret;
}

int snd_play(snd_stream_t* input, snd_stream_t* output, bool loop, bool* until)
{
    if (memcmp(&input->fmt, &output->fmt, sizeof(snd_format_t)) != 0)
        return -1;

    return snd_play_raw(&input->fmt, until, loop ? _snd_read_loop : snd_read, input, snd_write, output);
}

int snd_convert_and_play(snd_stream_t* input, snd_stream_t* output, bool loop, bool* until)
{
    if (input->fmt.channel != output->fmt.channel)
        return -1;

    snd_buffer_t* buffer = snd_alloc_buffer(&output->fmt);
    void* params[2];
    params[0] = output;
    params[1] = buffer;

    int ret = snd_play_raw(&input->fmt, until, loop ? _snd_read_loop : snd_read, input, _snd_write_convert, params);
    snd_free_buffer(buffer);
    return ret;
}


int snd_play_file(const char* filename, bool loop, bool* until)
{
    snd_stream_t* sample = snd_open_input(filename);
    if (sample == NULL)
        return -1;
    snd_stream_t* speaker = snd_open_speaker(&sample->fmt, 0);
    if (speaker == NULL) {
        snd_close(sample);
        return -1;
    }

    int ret = snd_play_raw(&sample->fmt, until, loop ? _snd_read_loop : snd_read, sample, snd_write, speaker);
    snd_close(speaker);
    snd_close(sample);
    return ret;
}
