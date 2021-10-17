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

int snd_play_at(snd_stream_t* input, bool loop, bool* until, void(*callback)(void*, snd_buffer_t*), void* arg)
{
    snd_buffer_t* buffer = snd_alloc_buffer(&input->fmt);
    for (int count = 0; until == NULL || *until; ++count) {

        int ret = snd_read(input, buffer);
        if (ret != 0) {
            if (!loop) {
                snd_free_buffer(buffer);
                return 0;
            }
            snd_rewind(input, 0);
            ret = snd_read(input, buffer);
            if (ret != 0) {
                snd_free_buffer(buffer);
                return -1;
            }
        }

        callback(arg, buffer);
    }
    snd_free_buffer(buffer);
    return 0;
}

int snd_play(snd_stream_t* input, snd_stream_t* output, bool loop, bool* until)
{
    if (memcmp(&input->fmt, &output->fmt, sizeof(snd_format_t)) != 0)
        return -1;

    return snd_play_at(input, loop, until, snd_write, output);
}

int snd_play_wavefile(const char* filename, bool loop, bool* until)
{
    snd_stream_t* sample = snd_open_input(filename);
    if (sample == NULL)
        return -1;
    snd_stream_t* speaker = snd_open_speaker(&sample->fmt, 0);
    if (speaker == NULL) {
        snd_close(sample);
        return -1;
    }

    int ret = snd_play_at(sample, loop, until, snd_write, speaker);
    snd_close(speaker);
    snd_close(sample);
    return ret;
}
