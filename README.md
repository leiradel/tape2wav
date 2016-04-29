# tape2wav

This utility generates a wave file from a tape file. It uses [`libspectrum`](http://fuse-emulator.sourceforge.net/libspectrum.php) to handle **TZX** tapes, and code derived from [`p2raw`](https://odemar.home.xs4all.nl/zx81/zx81.htm) to handle **T81** tapes. It seems that `libspectrum` doesn't handle ZX81 data inside **TZX** tapes very well, but let me know otherwise (even better: submit a push request).

## Usage

    $ ./tape2wav -h
    Usage: tape2wav options...

    -i filename        Input file name
    -o filename        Output file name (default is tape2wav.wav)
    -r sample_rate     Output sample rate (default is 44100)
    -s                 Stereo output (default is mono)
    -8                 Unsigned 8-bits per sample (default signed 16-bits)
    -h                 This page

## T81

**T81** is a tape format used by the [EightyOne](https://sourceforge.net/projects/eightyone-sinclair-emulator/) ZX81 emulator. There's a small utility in the `etc` folder which can create **T81** files from a list of **P** files.

## Tests

The generated wave files were tested only on the [Fuse](http://fuse-emulator.sourceforge.net/fuse.php) and EightyOne emulators. I've tested files from 8-bit, mono, 11,025 Hz to 16-bit, stereo, 192,000 Hz.

Please let me know your experiences loading the audio into the actual machines, including which device was used to play the wave files.
