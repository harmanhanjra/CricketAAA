# Windows Build

## Prerequisites

- Windows 10/11 64-bit
- Unreal Engine 5.4+
- Visual Studio 2022
- Workload: **Game development with C++**
- Windows SDK installed through Visual Studio Installer

## Build editor target

```bat
set UE_ROOT=C:\Program Files\Epic Games\UE_5.4
Tools\BuildEditor.bat
```

Or generate Visual Studio files from the `.uproject` and build `CricketGameEditor` in `Development Editor | Win64`.

## Package later

Do not package a release build until the editor compile, automation tests and manual vertical-slice acceptance pass. The current scaffold uses `/Engine/Maps/Entry` as a startup map and constructs the prototype world at runtime, avoiding a binary `.umap` requirement in this generated source-only starter.
