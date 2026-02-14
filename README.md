# supported os: linux (with framebuffer enabled (default))

user needs groups `video` and `input`

# compile

`gcc *.c`

# configure input:

## absolute (trackpad/tablet)

!! this is for absolute inputs. for it, go to the edges of the device for calibration then click ctrl-c !!
./a.out configinput /dev/input/event[number] >> input.conf

## relative

!! seperated by tabs !!
echo "/dev/input/event[number]      [multx] [multy] 0       0"

## keyboard

!! seperated by tabs !!
echo "/dev/input/event[number]      0       0       0       0"

# run:

!! there needs to be an input.conf for this !!
./a.out player 15 koishi/*
