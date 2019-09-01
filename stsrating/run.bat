Pushd "%~dp0"
C:\Python27amd64\python.exe sts_rating_v13.1.py -f sts_all.epd -e E:\ProjectBuilds\tinychess2\Release\tinychess2_uci.exe -t 1 --proto uci -h 128 --getrating --log
REM C:\Python27amd64\python.exe sts_rating_v13.1.py -f sts_all.epd -e E:\Brown\Downloads\stockfish-10-win\Windows\stockfish_10_x64.exe -t 1 --proto uci -h 128 --getrating --log