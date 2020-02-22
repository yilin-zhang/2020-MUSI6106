# Assignment 2

## Usage
`MUSI6106Exec /path/to/wavfile delayModTimeInS VibFreqInHz`

```
|            Vibrato Range          |
[-----------------*-----------------]
                  | delayModTimeInS |
```

## Real-world examples

MATLAB code, audio, image are in [matlab](./matlab) folder.

The plots are based on:
- Delay modulation time: 0.01 s
- Vibrato frequency: 5 Hz

![](./matlab/img/plots.png)


## Updates

There are 5 major changes:
- Support multi-channel in `CVibrato` class.
- Remove the if statement in `CVibrato::process` to `CVibrato::init`.
- Add an in-place test case.
- Add command-line arguments for vibrato parameters.
- A modification in the given MATLAB code in order to match my cpp
  implementation, with an update of the plots.

Specifically, I changed the line in `vibrato.m`

```matlab
MOD=sin(M*2*pi*n);
```

to 

```matlab
MOD=sin(M*2*pi*(n-1));
```

to make the sine wave start from 0.
