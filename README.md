# tape2wav

This utility generates a wave file from a tape file. It uses [`libspectrum`](http://fuse-emulator.sourceforge.net/libspectrum.php) to handle **TZX** tapes, and code derived from [`p2raw`](https://odemar.home.xs4all.nl/zx81/zx81.htm) to handle **T81** tapes. It seems that `libspectrum` doesn't handle ZX81 data inside **TZX** tapes very well, but let me know otherwise (even better: submit a pull request).

## Usage

    $ ./tape2wav -h
    Usage: tape2wav options...

    -i filename        Input file name
    -o filename        Output file name (default is tape2wav.wav)
    -r sample_rate     Output sample rate (default is 44100)
    -s                 Stereo output (default is mono)
    -8                 Unsigned 8-bits per sample (default signed 16-bits)
    -h                 This page

# playtape

This utility uses SDL2 to play tapes directly to an audio device.

## Usage

    $ ./playtape -h
    Usage: playtape options...

    -i filename        Input file name
    -h                 This page

# TZX

**TZX** is a tape format created to allow the faithful reproduction of ZX Spectrum tapes, even those that use custom loading routines. The format is documented [here](http://www.worldofspectrum.org/TZXformat.html).

# T81

**T81** is a tape format used by the [EightyOne](https://sourceforge.net/projects/eightyone-sinclair-emulator/) ZX81 emulator. There's a small utility in the `etc` folder which can create **T81** files from a list of **P** files.

# Tests

The utilities were tested only on the [Fuse](http://fuse-emulator.sourceforge.net/fuse.php) and EightyOne emulators. I've tested files from 8-bit, mono, 11,025 Hz to 16-bit, stereo, 192,000 Hz.

Please let me know your experiences loading the audio into the actual machines, including which device was used to play the wave files.

# Coroutines

To generate wave data in chunks, which would be somewhat difficult specially in the **T81** case, I decided to use soft coroutines to make my life easier, and ended up with the macros in `src/coro.h`.

The end result is something that resembles BASIC (with GOTOs and GOSUBs), see usage examples in `tzx_play` and `t81_play`.

`src/coro.h` is released under the [CC0](https://creativecommons.org/publicdomain/zero/1.0/).
