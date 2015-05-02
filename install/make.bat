@echo off
copy /Y ..\win32\release\abook.dll abook32\abook.dll
copy /Y ..\x64\release\abook.dll abook64\abook.dll
xcopy /Y /E ..\default-skins\*.* abook32\skins\
xcopy /Y /E ..\default-skins\*.* abook64\skins\

rm abook32.zip
rm abook64.zip

cd abook32
"C:\Program Files\7-zip\7z" a ..\abook32.zip -r *.*
cd ..\

cd abook64
"C:\Program Files\7-zip\7z" a ..\abook64.zip -r *.*
cd ..\
