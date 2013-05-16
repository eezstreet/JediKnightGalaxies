@echo off
echo ========================================
echo ...::: Generating Music List File :::...
echo ========================================
dir *.ogg *.mp3 *.wav /b /ON > MyMusic.list
dir *.ogg *.mp3 *.wav /b /ON
echo ========================================