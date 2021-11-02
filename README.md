# Library Sound `lsnd.so`

 The `lsnd` library aims to support the most basic way to utilize a machine
 speaker. The goal is to offer the most basic yet reliable way to access
 audio on multiple platform.

 The library can be build for several distributions. As it serves as the
 primary access method for Kora OS, it also work on Win32 API.

 The default libray is able to open wave file format. Plan is made to support
 plugin to develop codex for more notorious music files.

## Usage

 The most simple way to use `lsnd` is playing a wav file, which come in this
 simple way. Becareful the call is synchronous and will play until
 `keep_looping` is set to false.

```c
    bool loop = true;
    bool keep_looping = true;
    snd_play_file("MyAudioSample.wav", loop, &keep_looping);
```

 To manipulate a file here's a more complex example:

```c
void convert_wave_file(const char* filein, const char* fileout, int word_size, int rate)
{
    snd_format_t format; // Set the format as wanted
    format.word_size = word_size; // 1(s8), 2(u16) or 4(u32)
    format.sample_rate = (rate + 99) % 100; // roundup to 100

    // Open input and output audio streams
    snd_stream_t* sample = snd_open_input("input.wav");
    format.channel = snd_format(sample)->channel; // Keep the same number of channels
    snd_stream_t* output = snd_open_output("output.wav", &format);

    // Allocate buffer - All buffer on lsnd.so represent 10ms
    snd_buffer_t* buf_in = snd_alloc_buffer_from(sample);
    snd_buffer_t* buf_out = snd_alloc_buffer(&format);

    // Read all the file
    while (snd_read(sample, buf_in) == 0) {
        // Do a sample_rate convertion
        snd_convert(buf_in, buf_out);
        snd_lowpass(buf_out, snd_format(sample)->sample_rate / 2);
        // Write on output
        snd_write(output, buf_out);
    }

    // Free all buffers and streams
    snd_free_buffer(buf_in);
    snd_free_buffer(buf_out);
    snd_close(sample);
    snd_close(output);
}
```

 Here we dig a little more under the object of the library, manipulating
 streams and buffers. The library also got some basic wave modification
 routines.


## Stream

 - `snd_open_input`
 - `snd_open_output`
 - `snd_open_speaker`
 - `snd_open_fd`
 - `snd_close`

 - `snd_read`
 - `snd_write`
 - `snd_rewind`

## Buffers

 On `lsnd` a buffer is always a container for chunk of 10ms of audio, and have
 a fixed format (channels, sample rate, word size).

 - `snd_alloc_buffer`
 - `snd_alloc_buffer_from`
 - `snd_free_buffer`
 - `snd_format`

## Wave opertions

 - `snd_convert`
 - `snd_lowpass`
 - `snd_highpass`


## Helper

 - `snd_volume` - Will only work on a speaker or audio device.
 - `snd_play` - Write one stream into another
 - `snd_play_file` - Play a file into a native speaker.
 - `snd_convert_and_play` - Write one stream into another and convert format
 - `snd_play_raw` - Play as long as producer provide data and consumer play it.

