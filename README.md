# S-Key to Touch Mapper
*S stands for special or sidee*

This is a may-duplicated but simple program, just to transform your phone's physical key events into touches.

~Originally designed to map VOL_UP to trigger button or others in FPS game.~

[WIP] The features are currently limited, and most configs are hard-coded.
Feature plans:
[x] Basic mapping feature
[x] Find the correct `input/event` file by EVIOCGNAME
[x] Configurable touch position
- [ ] Read config at startup
[ ] Modular design and configurable key(s)
- [ ] Proper(and well-formatted) config file
[ ] Tidy&lightweight configure frontend
...

# Build

(My bad, I added `libconfig` as dependency but not used currently, you have to install the library to compile)
```bash
mkdir -p out
g++ touchdev.cpp dconfd.cpp ktmap.cpp -lconfig++ -o out/ktmap
```


# Disclaimer
You may have root permission to grab `/dev/input/event*`, and root is required to create uInput device.
The program and the author are NOT responsible for ANY detection/bans or social conflicts/dramas/shame caused by using this program on games or other applications. Whether you hide loopholes, leaks, root clues etc or not, it's all your business.
In short, USE AT YOUR OWN RISK.
