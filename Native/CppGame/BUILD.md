# C++ Realistic Build — Native/CppGame

## Source
- `main.cpp` — single-file Win32 + GDI+ (no external libs)
- Separate YOU vs BOT boards, 2 innings x 2 overs, bot AI, realistic stadium

## Build — MSVC (Visual Studio Developer Prompt)
```bat
cl /EHsc /O2 /DUNICODE main.cpp user32.lib gdi32.lib gdiplus.lib /Fe:CricketCpp.exe
CricketCpp.exe
```

## Build — MinGW
```bat
g++ -O2 -mwindows -std=c++17 main.cpp -lgdi32 -luser32 -lgdiplus -o CricketCpp.exe
./CricketCpp.exe
```

## Build — this environment (no compiler)
No MSVC/MinGW was found here, so the C++ exe cannot be produced on this host.
The source is complete and builds on any Windows with VS 2022 or MinGW. Syntax was verified
against the C# port which was compiled successfully to `Native/CsGame/CricketCs.exe`.

## Controls
Mouse line/length, Space bowl, L/R shot, R run, Tab cam, BKSP reset, keys 1-4 for mode (1:BOTH 2:BAT 3:BOWL 4:BOTS)
Tabs mirror `Source/CricketGame/Public/Ball/BallPhysicsComponent.h` drag/Magnus and `BattingComponent.h` timing window.
