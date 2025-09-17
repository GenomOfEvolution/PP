@echo off
echo Starting benchmark with 4 core and threads from 1 to 16...

for /L %%i in (1,1,16) do (
    echo Running with %%i threads, 4 core...
    BlurBMP.exe input.bmp output.bmp %%i 4
    echo.
)

echo Benchmark completed!
pause