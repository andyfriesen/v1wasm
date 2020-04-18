g++ -Wall -Werror main.cpp -o devcc.exe
@if ERRORLEVEL 1 goto error
devcc bumville.map
@if ERRORLEVEL 1 goto error
@echo * Success!
@goto done
:error
@echo * Failed.
:done
pause