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
#include "../snd-io.h"
#include <Windows.h>
#include <threads.h>

#pragma comment(lib, "Winmm.lib")

struct snd_win_spk {
    HWAVEOUT handle;
    int buf_count;
    int buf_pen;
    int buf_length;
    WAVEHDR* bufs;
};

static int win32_wav_write(snd_stream_t* strm, snd_buffer_t* buf)
{
    if (memcmp(&strm->fmt, &buf->fmt, sizeof(snd_format_t)) != 0)
        return -1;

    struct snd_win_spk* spk = strm->data;

    if (spk->buf_pen >= spk->buf_count)
        spk->buf_pen = 0;

    while ((spk->bufs[spk->buf_pen].dwFlags & WHDR_DONE) == 0) {
        xtime xs;
        xs.sec = 0;
        xs.nsec = 10 * 1000000;
        thrd_sleep(&xs, NULL);
    }

    memcpy(spk->bufs[spk->buf_pen].lpData, buf->ptr, buf->len);
    MMRESULT res = waveOutWrite(spk->handle, &spk->bufs[spk->buf_pen], sizeof(WAVEHDR));
    spk->buf_pen++;
    return res == 0 ? 0 : -1;
}

static int win32_wav_close(snd_stream_t* strm)
{
    struct snd_win_spk* spk = strm->data;
    if (spk == NULL)
        return -1;

    waveOutClose(spk->handle);
    for (int i = 0; i < spk->buf_count; ++i) {
        waveOutUnprepareHeader(spk->handle, &spk->bufs[i], sizeof(WAVEHDR));
        free(spk->bufs[i].lpData);
    }
    free(spk->bufs);
    free(spk);
    strm->data = NULL;
    return 0;
}

snd_stream_t* snd_open_speaker(snd_format_t* format, int device)
{
    WAVEFORMATEX fx;
    fx.wFormatTag = WAVE_FORMAT_PCM;
    fx.nChannels = format->channel;
    fx.nSamplesPerSec = format->sample_rate;
    fx.wBitsPerSample = format->word_size * 8;
    fx.cbSize = 0;

    fx.nBlockAlign = fx.nChannels * (fx.wBitsPerSample / 8);
    fx.nAvgBytesPerSec = fx.nSamplesPerSec * fx.nBlockAlign;

    HWAVEOUT wo;
    MMRESULT res = waveOutOpen(&wo, WAVE_MAPPER, &fx, 0, 0, CALLBACK_NULL);
    if (res != 0)
        return NULL;

    struct snd_win_spk* spk = malloc(sizeof(struct snd_win_spk));
    spk->handle = wo;
    spk->buf_pen = 0;
    spk->buf_count = 50; // 100 par sec
    spk->buf_length = fx.nAvgBytesPerSec / 100;
    spk->bufs = malloc(spk->buf_count * sizeof(WAVEHDR));
    memset(spk->bufs, 0, spk->buf_count * sizeof(WAVEHDR));

    for (int i = 0; i < spk->buf_count; ++i) {
        spk->bufs[i].lpData = malloc(spk->buf_length);
        spk->bufs[i].dwBufferLength = spk->buf_length;
        res = waveOutPrepareHeader(wo, &spk->bufs[i], sizeof(WAVEHDR));
        spk->bufs[i].dwFlags |= WHDR_DONE;
    }

    snd_stream_t* strm = malloc(sizeof(snd_stream_t));
    strm->data = spk;
    strm->length = 0;
    strm->position = 0;
    strm->fmt.sample_rate = format->sample_rate;
    strm->fmt.channel = format->channel;
    strm->fmt.word_size = format->word_size;
    strm->write = win32_wav_write;
    strm->close = win32_wav_close;
    strm->read = NULL;
    strm->rewind = NULL;
    return strm;
}

int snd_volume(snd_stream_t* strm, int level)
{
    if (level < 0)
        level = 0;
    if (level > 100)
        level = 100;
    struct snd_win_spk* spk = strm->data;
    MMRESULT res = waveOutSetVolume(spk->handle, level == 100 ? 0xFFFF : (level * (0xFFFF / 100)));
    return res == 0 ? 0 : -1;
}
