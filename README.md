
## prereqs

```
gcc -v -E -x c++ -
```
to list directories to include (see c_cpp_properties.json).

Sometimes restarting Visual Studio Code will fix Intellisense.

Download ffmpeg and SDL; see ffmpeg/ and sdl/ respectively for more instructions. Ran in dev with ffmpeg `v8.0` and SDL `v3.2.26`.

## building
`CMake > Delete Cache and Reconfigure > Build`

![alt text](docs/imgs/adb.png)
`Developer Options > enable USB Debugging`
After connecting android device, check connection with `adb devices` and run prog.

## running
See SccppServer for running the Activity on the device that will capture and encode device video.

## internals

- adb forwards e.g. `adb -s "39090DLJH002RY" forward tcp:27183 localabstract:sccpp`
- connects to Sccpp Server (Java) via Socket
- sends handshake; receives back device name
- closes Socket and cleans up adb forward 
