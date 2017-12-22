@echo off
cd %~dp0
:intro
CLS
ECHO Woohoo! I'm Test-RIIIIIICK ~buurrp~
ECHO Here's what you need to do, Morty:
ECHO You need to - are you listening to me, Morty? - 
ECHO you need to ~burrrp~ give the following settings.
ECHO Otherwise.. Otherwise what's the point of having a
ECHO Test-Rick, huh Morty?

ECHO Actually, let me just remove any old files
ECHO which might have been left over for some reason...

DEL /Q *.csv

ECHO Ok,now here are the ~buuurp~ settings:

SET /p "cubeStartDim=How many cubes to start with (NxNxN)? "
SET /p "cubeInc=How many cubes shall each test increment with? "
SET /p "seconds=How many seconds shall the program run for? "
SET /p "threadCount=How many threads will be used? "
ECHO And, Morty, this is very important
SET /p "outputFolder=What folder shall the collected data be put in? "
ECHO OK, last one:
SET /p "testCount=How many times are we running this test? "

ECHO Now, next time you press ENTER, the tests will begin.
ECHO Are you ~burp~ ready for that, are you Morty?
PAUSE

CALL :repeat %testCount% for_body
ECHO Test complete! You did it, Morty!
PAUSE

GOTO :EOF
:for_body
REM %1 = index
REM %2 = cubeStartDim
REM %3 = cubeInc
REM %4 = seconds
REM %5 = threadCount
REM %6 = outputFolder

SET /A cubeDim=%1*%3+%2%

START "Test #%1" /WAIT AutoRecord.bat %4 %cubeDim% %5 %6
echo test #%1 done
REM subroutine for old-fashioned for-loop

GOTO :EOF
:repeat
SET count=%1
SET func=%2
SET i=0

:for_loop
IF %i% LSS %count% (
	CALL :%func% %i% %cubeStartDim% %cubeInc% %seconds% %threadCount% %outputFolder%
	SET /A i=%i%+1
	GOTO :for_loop
) ELSE (
	GOTO :EOF
)