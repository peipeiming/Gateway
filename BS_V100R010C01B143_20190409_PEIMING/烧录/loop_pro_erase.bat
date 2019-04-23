@echo off
set APP1_HEX_PATH="1"
for /r ".\firmware\app\" %%a in (*.hex)do (
set APP1_HEX_PATH="%%~a"
)

set APP2_HEX_PATH="2"
for /r ".\firmware\bootlaoder\" %%a in (*.hex)do (
set APP2_HEX_PATH="%%~a"
)


if "%%1"=="" (
	
	set start_loop=100000
) else (

	set start_loop=%1
)

echo si 1 >autopro.txt
echo speed 4000 >>autopro.txt
echo r >>autopro.txt
echo h >>autopro.txt
echo erase >>autopro.txt
if  %APP1_HEX_PATH% neq "1" (echo loadfile %APP1_HEX_PATH% >>autopro.txt)
if  %APP2_HEX_PATH% neq "2" (echo loadfile %APP2_HEX_PATH% >>autopro.txt)
echo r >>autopro.txt
echo g >>autopro.txt
echo q >>autopro.txt

cls


if %%loop_count%% gtr %%start_loop%% goto loop_end
 
echo *** Please prepare the target to be programmed ***	

cls

echo *** Start programming target %loop_count% ***

jlink.exe -device STM32F103ZE -CommandFile autopro.txt

set /a loop_count+=1 > NUL

goto loop_start

:loop_end

endlocal

pause

