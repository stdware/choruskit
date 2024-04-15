@echo off
setlocal enabledelayedexpansion

: Initialize new path list
set "NEW_PATH="

: Command list
set "CMD="

: Traverse command line arguments
:collect_paths
if "%~1"=="" goto end_parse_args
if "%~1"=="--" (
    shift
    goto collect_cmd
)

: Check if the argument is a file
if exist "%~1" (
    if not "%~x1"=="" (
        : File, read every line in the file as a path
        for /f "usebackq delims=" %%a in ("%~1") do (
            set "NEW_PATH=!NEW_PATH!%%a;"
        )
    ) else (
        : Directory, add to the path list
        set "NEW_PATH=!NEW_PATH!%~1;"
    )
) else (
    echo Warning: "%~1" does not exist.
)
shift
goto collect_paths

:collect_cmd
if "%~1"=="" goto execute_cmd
set "CMD=!CMD!%1 "
shift
goto collect_cmd

:end_parse_args

:execute_cmd
: If there is no command, exit
if "!CMD!"=="" (
    echo Usage: %~n0 ^<path^>... -- ^<cmd^> [^<arg^>...]
    exit /b
)

: Insert new paths in front of the environment variable PATH
set "PATH=!NEW_PATH!!PATH!"

: Execute commands
echo !PATH!
!CMD!

endlocal
