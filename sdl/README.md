where SDL libs/includes/DLLs,
use download.ps1 from the sdl dir to gen a structure like:

```
├── sdl/
|   |── bin/
|   |   ├── SDL3.dll/
│   │   └── ...
|   |
│   ├── include/
│   │   ├── SDL_assert.h/
│   │   ├── SDL_asyncio.h/
│   │   └── ...
│   └── lib/
│       ├── libSDL3_test.a
│       ├── libSDL3.dll.a
│       └── ...
```

Copy the DLLs next to the exe in the build/ folder for runtime.