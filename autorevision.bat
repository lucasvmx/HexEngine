:: autorevision.bat
::
:: * Script para fazer a revisão automática do código-fonte
::
:: * Autor
:: 	- Lucas Vieira de Jesus <lucas.engen.cc@gmail.com>
::
:: > Necessita do script autorevision e do git instalado
::
:: As mensagens estão em inglês para evitar a impressão incorreta de certos tipos de caracteres
::

@echo off
setlocal

if not defined PASTA_GIT_BIN (
	echo Environment variable PASTA_GIT_BIN is not set
	goto error
)

set project_file=hexengine.pro

if not exist %project_file% (
	echo We need to be in the same path of project folder
	echo Current path: %cd%
	goto error
)

set sh_exe="%PASTA_GIT_BIN%\sh.exe"
set bash_exe="%PASTA_GIT_BIN%\bash.exe"

if not exist %sh_exe% (
	echo %sh_exe% not found
	goto error
) 

if not exist %bash_exe% (
	echo %bash_exe% not found
	goto error
)

echo Current folder: %cd%
set command=%sh_exe% %cd%\autorevision.sh
echo Running command: %command%
%command% -t h > %cd%/src/autorevision.h
if %ERRORLEVEL% equ 0 (
	goto ok
) else (
	goto error
)

:error
echo Revision failed
exit /B 1

:ok
echo Revision completed successfully
exit /B 0
