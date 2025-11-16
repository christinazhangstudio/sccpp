where ffmpeg libs/includes/DLLs,
use download.ps1 to gen a structure like:

```
├── ffmpeg/
|   |── bin/
|   |   ├── avcodec-62.dll/
|   |   ├── avdevice-62.dll/
│   │   └── ...
|   |
│   ├── include/
│   │   ├── libavcodec/
│   │   ├── libavutil/
│   │   └── ...
│   └── lib/
│       ├── libavcodec.a
│       ├── libavutil.a
│       └── ...
```