@ECHO OFF 
if "%~1"=="" goto blank

set pth=distrib\MGen %1
echo Building distrib %pth%
rem mkdir "%pth%"

rem CLEAN
rmdir /s /q "%pth%"
del "%pth%.zip"

rem COPY
xcopy /y x64\Release\MGen.exe "%pth%\x64\"
xcopy /y Release\MGen.exe "%pth%\"
xcopy /y Release\MGenServer.exe "%pth%\server\"
xcopy /y Release\MGenClean.exe "%pth%\server\"
xcopy /y Release\MGenTest.exe "%pth%\server\"
robocopy /e /mir "MGen\configs" "%pth%\configs" *
robocopy /e /mir "MGen\instruments" "%pth%\instruments" *
robocopy /e /mir "MGen\music" "%pth%\music" *
robocopy /e /mir "MGen\autotest\configs" "%pth%\autotest\configs" *
xcopy /y MGen\autotest\test.bat "%pth%\autotest\"
xcopy /y MGen\autotest\test.csv "%pth%\autotest\"
xcopy /y MGen\db\expect.csv "%pth%\db\"

rem REMOVE UNNECESSARY
rmdir /s /q "%pth%\music\Music\private"

rem MAKE ZIP
"c:\program files\winrar\winrar" a -afzip -r -ep1 "%pth%.zip" "%pth%"

goto DONE
:BLANK
echo Please specify version in command line
:DONE
