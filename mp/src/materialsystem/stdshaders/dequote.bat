rem Remove any quotes from the input string
for /f "delims=" %%A in ('echo %%%1%%') do set %1=%%~A
