mkdir -p build/dlls
cd build/dlls

[ ! -f "Memento_Windows_x86_64_mpv_unstable.zip" ] \
    && wget https://github.com/ripose-jp/Memento/releases/download/v0.5.0-1-beta/Memento_Windows_x86_64_mpv_unstable.zip \
    && unzip Memento_Windows_x86_64_mpv_unstable.zip \

mkdir -p ../Memento_x86_64
cp ./Memento_x86_64/*.dll ../Memento_x86_64
