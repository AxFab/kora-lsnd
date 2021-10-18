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
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

double snd_read_value(snd_buffer_t* buf, int channel, int idx)
{
    int k = idx * buf->fmt.channel + channel;
    if (buf->fmt.word_size == 1)
        return ((double)((uint8_t*)buf->ptr)[k] - 128.0) / 128.0;
    else if (buf->fmt.word_size == 2)
        return (double)((int16_t*)buf->ptr)[k] / 32768.0;
    else if (buf->fmt.word_size == 4)
        return (double)((int32_t*)buf->ptr)[k] / 2147483648.0;
    return 0.0;
}

double snd_write_value(snd_buffer_t* buf, int channel, int idx, double val)
{
    int k = idx * buf->fmt.channel + channel;
    if (buf->fmt.word_size == 1)
        ((uint8_t*)buf->ptr)[k] = (uint8_t)(val * 128 + 128.0);
    else if (buf->fmt.word_size == 2)
        ((int16_t*)buf->ptr)[k] = (int16_t)(val * 32768.0);
    else if (buf->fmt.word_size == 4)
        ((int32_t*)buf->ptr)[k] = (int32_t)(val * 2147483648.0);
    else
        return 0.0;
    return val;
}

snd_buffer_t* snd_alloc_buffer(snd_format_t* format)
{
    snd_buffer_t* buf = malloc(sizeof(snd_buffer_t));
    buf->fmt = *format;
    buf->size = format->sample_rate / 100;
    buf->len = buf->size * format->word_size * format->channel;
    buf->ptr = malloc(buf->len);
    memset(buf->ptr, 0, buf->len);
    return buf;
}

snd_buffer_t* snd_alloc_buffer_from(snd_stream_t* strm)
{
    return snd_alloc_buffer(&strm->fmt);
}


void snd_free_buffer(snd_buffer_t* buf)
{
    free(buf->ptr);
    free(buf);
}

LIBAPI snd_format_t* snd_format(snd_stream_t* strm)
{
    return &strm->fmt;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

int snd_fd_read(snd_stream_t* strm, snd_buffer_t* buf)
{
    int fd = (int)strm->data;
    if (memcmp(&strm->fmt, &buf->fmt, sizeof(snd_format_t)) != 0)
        return -1;
    for (int i = 0; i < strm->fmt.channel; ++i)
        buf->prevs[i] = snd_read_value(buf, i, buf->size - 1);
    int lg = read(fd, buf->ptr, buf->len);
    if (lg <= 0)
        return -1;
    strm->position++;
    if (lg < buf->len) {
        memset(&((char*)buf->ptr)[lg], 0, buf->len - lg);
        return 0;
    }
    return 0;
}

int snd_fd_write(snd_stream_t* strm, snd_buffer_t* buf)
{
    int fd = (int)strm->data;
    if (memcmp(&strm->fmt, &buf->fmt, sizeof(snd_format_t)) != 0)
        return -1;
    for (int i = 0; i < strm->fmt.channel; ++i)
        buf->prevs[i] = snd_read_value(buf, i, buf->size - 1);
    int lg = write(fd, buf->ptr, buf->len);
    if (lg < buf->len)
        return -1;
    strm->position++;
    if (strm->length < strm->position * buf->len)
        strm->length = strm->position * buf->len;
    return 0;
}

int snd_fd_close(snd_stream_t* strm)
{
    int fd = (int)strm->data;
    if (fd != 0)
        close(fd);
    strm->data = NULL;
    return 0;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

snd_stream_t* snd_open_fd(int fd, snd_format_t* format)
{
    snd_stream_t* strm = malloc(sizeof(snd_stream_t));
    strm->data = (void*)fd;
    strm->length = 0;
    strm->position = 0;
    memcpy(&strm->fmt, format, sizeof(snd_format_t));
    strm->read = snd_fd_read;
    strm->write = snd_fd_read;
    strm->rewind = NULL;
    strm->close = snd_fd_close;
    return strm;
}

snd_stream_t* snd_open_input(const char* path)
{
    snd_stream_t* strm = malloc(sizeof(snd_stream_t));
    if (snd_wav_open_input(strm, path) != 0) {
        free(strm);
        return NULL;
    }
    return strm;
}

snd_stream_t* snd_open_output(const char* path, snd_format_t* format)
{
    snd_stream_t* strm = malloc(sizeof(snd_stream_t));
    if (snd_wav_open_output(strm, path, format) != 0) {
        free(strm);
        return NULL;
    }
    return strm;
}

void snd_close(snd_stream_t* strm)
{
    if (strm->close)
        strm->close(strm);
}

int snd_read(snd_stream_t* strm, snd_buffer_t* buf)
{
    if (!strm->read)
        return -1;
    return strm->read(strm, buf);
}

int snd_write(snd_stream_t* strm, snd_buffer_t* buf)
{
    if (!strm->write)
        return -1;
    return strm->write(strm, buf);
}

int snd_rewind(snd_stream_t* strm, int ms)
{
    if (!strm->rewind)
        return -1;
    return strm->rewind(strm, ms);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void snd_convert(snd_buffer_t* in, snd_buffer_t* out)
{
    if (in->fmt.channel != out->fmt.channel)
        return;

    for (int to = 0;; ++to) {
        double ti = (double)to * (double)in->fmt.sample_rate / (double)out->fmt.sample_rate;
        int bl = (int)floor(ti);
        int up = (int)ceil(ti);
        double r = ti - bl;
        if (up >= in->size)
            break;

        for (int i = 0; i < in->fmt.channel; ++i) {
            double v1 = snd_read_value(in, i, bl);
            double v2 = snd_read_value(in, i, up);
            double val = v1 * (1.0 - r) + v2 * r;
            snd_write_value(out, i, to, val);
        }
    }
}

void snd_lowpass(snd_buffer_t* buf, double freq)
{
    // low - pass: output[N] = input[N] * Y + output[N - 1] * X
    double Fc = freq / (double)buf->fmt.sample_rate;
    double X = exp(-2.0 * 3.1415926535897932384626433832795 * Fc);
    double Y = 1.0 - X;

    double prevs[8];
    memcpy(prevs, buf->prevs, sizeof(prevs));
    for (int j = 0; j < buf->size; ++j) {
        for (int i = 0; i < buf->fmt.channel; ++i) {
            double val = snd_read_value(buf, i, j);
            snd_write_value(buf, i, j, val * Y + prevs[i] * X);
            prevs[i] = val;
        }
    }
}

void snd_highpass(snd_buffer_t* buf, double freq)
{
    // high - pass: output[N] = Y * input[N] + Z * input[N - 1] + X * output[N - 1]
    double Fc = freq / (double)buf->fmt.sample_rate;
    double X = exp(-2.0 * 3.1415926535897932384626433832795 * Fc);
    double Y = (1.0 + X) / 2.0;
    double Z = -(1.0 + X) / 2.0;

    double prevs[8];
    memcpy(prevs, buf->prevs, sizeof(prevs));
    for (int j = 0; j < buf->size; ++j) {
        for (int i = 0; i < buf->fmt.channel; ++i) {
            double val = snd_read_value(buf, i, j);
            snd_write_value(buf, i, j, val * Y + prevs[i] * X + Z * prevs[i]); // Bad value !
            prevs[i] = val;
        }
    }
}
