# C# Realistic Build — Native/CsGame

## Requirements
- Windows 10/11
- .NET Framework 4.8 (csc.exe at `C:\Windows\Microsoft.NET\Framework\v4.0.30319\csc.exe`) — no SDK needed
- OR .NET SDK 6+ (`dotnet build`)

## Build (this machine — no SDK needed)
```bat
C:\Windows\Microsoft.NET\Framework\v4.0.30319\csc.exe /nologo /target:winexe ^
  /reference:System.Windows.Forms.dll /reference:System.Drawing.dll ^
  /out:CricketCs.exe Program.cs
CricketCs.exe
```

## Build with SDK
```bat
dotnet new console -n CricketCs -f net8.0
copy Program.cs CricketCs\Program.cs
dotnet run --project CricketCs
```

## Features — mirrors 3D web + UE5
- Separate YOU vs BOT scoreboards (`Team you/bot`), 2 innings x 2 overs, maxWickets=5
- Bot bowl/bat AI (easy/medium/hard), Magnus/swing, pitch bounce
- Realistic GDI+ stadium: radial field, pitch texture, creases, stumps, fielders, shadows
- Controls: Mouse line/length, Space bowl, L/R shot, R run, Tab cam, BKSP reset, 1-4 mode
