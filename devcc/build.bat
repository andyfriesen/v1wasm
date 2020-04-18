g++ -Wall -Werror main.cpp -o devcc.exe
@if ERRORLEVEL 1 goto error
devcc bumville.map > bumville.vc
@if ERRORLEVEL 1 goto error
devcc startup.vcs > startup.vc
@if ERRORLEVEL 1 goto error
@echo * Success!
@goto done
:error
@echo * Failed.
:done
pause