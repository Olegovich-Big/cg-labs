param([string]$Lab = 'all')
$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'Install Visual Studio C++ Desktop workload and Windows SDK.' }
if ($Lab -ne 'all' -and $Lab -notmatch '^(0[1-9]|1[01])$') { throw 'Use -Lab 01 through 11, or all.' }
$ids = if ($Lab -eq 'all') { 1..11 | ForEach-Object { '{0:D2}' -f $_ } } else { @($Lab) }
New-Item -ItemType Directory -Force "$root\bin", "$root\build" | Out-Null
$commands = @('@echo off', ('call "{0}\VC\Auxiliary\Build\vcvars64.bat" >nul' -f $vs), ('cd /d "{0}"' -f $root))
foreach ($id in $ids) {
  $commands += ('cl /nologo /std:c++17 /EHsc /O2 /W4 /utf-8 /DUNICODE /D_UNICODE /DNOMINMAX "src\lab{0}.cpp" /Fo"build\lab{0}.obj" /Fe"bin\lab{0}.exe" /link opengl32.lib glu32.lib gdiplus.lib comdlg32.lib user32.lib gdi32.lib shell32.lib' -f $id)
  $commands += 'if errorlevel 1 exit /b 1'
}
$commands += 'exit /b 0'
[IO.File]::WriteAllLines("$root\build\compile.cmd", $commands, [Text.Encoding]::Default)
& $env:ComSpec /d /c "`"$root\build\compile.cmd`""
if ($LASTEXITCODE -ne 0) { throw 'Compilation failed.' }
