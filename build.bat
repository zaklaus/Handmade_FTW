@echo off

msvc.bat win32_test.c D "/link /NODEFAULTLIB /SUBSYSTEM:WINDOWS kernel32.lib user32.lib gdi32.lib"

