# IceBanelingEscape_Networking
Networked multiplayer game with TCP lobby system and UDP dedicated server

Download distribution at: https://www.dropbox.com/scl/fi/8avn5pm1gug6ga1kuheff/IceBanelingNetworkDistribution.zip?rlkey=1v303e60f7dpx84v0pwl82luq&dl=0

Download developer at: https://www.dropbox.com/scl/fi/y110j47n41xocveejq4ie/IceBanelingNetworkDeveloper.zip?rlkey=xkworai3c0i3l4kckwppwuzp3&dl=0

## Description
- This game is a clone of Ice Baneling Escape on the StarCraft 2 Arcade. This project was done to learn and implement networking in games. It uses a dedicated server to connect multiple people. Theres a lobby system using TCP connection, and the game itself uses UDP.
-  Server.exe is a headless application that runs the server that hosts the lobby and the games (all in 1 exe). Set the IPv4 address in Config.txt and port number to be able to use server. Use ipConfig on command prompt to see your ipv4 address.
- Client.exe is the application you will use to actually play the game, you can run multiple instances of it, create multiple lobbies, and play multiple games
-
- Config.txt has variables you can change. If you destroy or change the file incorrectly it will recreate it with the default settings
- In SharedSrc/Config.h in codebase that is where you can hardcode change any config variable, and you will see thats what generates Config.txt
- In SharedSrc/Sockets/PingEmulator you can uncomment #define PING_EMULATOR to run ping variability test. Options are configurable in the file, and in each of the app classes they also can define if they want to use it or not

## Features
- Lobby system using TCP
- data serialization of read/write of packets, using bitBuffer to store single bits for compression
- Implemented light UDP layer: doesnt handle packets that are out of order, has the ability to resend packets that need to be garunteed
- Server-Client model using dedicated server to host game, game is ran with UDP
- WinSock2 is used to handle low level sockets
- hundreds of dynamic objects able to be networked to many clients, audio triggers, object replication, other game state changes
- Reliable network: handles disconnects, stall outs, UDP packets getting sent multiple times, dropping, out of order, high latency, latency variability

## Dependencies
- STBI: used to load images. Its already included in SharedSrc/ThirdParty directory
- GLM (version 0.9.9.4)     : math library
- GLEW (version 2.1.0)      : used to load OpenGL functions
- GLFW (version 3.3)        : used for window handling, and event handling
- SDL2 (version 2.28.5)     : used with SDL2_mixer for audio
- SDL2_mixer (version 2.6.3): is used for audio
- freetype (version 2.13.2) : used to render font
- Winsock2 (version 2.2)    : API used for networking
- OpenGL (version 4.6.0)    : Used for rendering
- 
- *dlls: will need SDL2.dll, SDL2_mixer.dll, the Microsoft Visual C++ Redistributable in the running directory

- This game runs on x64 bit. All dependencies support x64.
- Only runs on Windows platform.

![283235966-3bb5dc8f-98bb-49c7-9feb-e8fc52eba014](https://github.com/GiboDidact/IceBanelingEscape_Networking/assets/33635742/6ef7f4d4-571b-4c47-a1ef-5ade85d8d6e0)

![283235964-bfeb1d40-b352-4aeb-8b7c-327bb4b9967f](https://github.com/GiboDidact/IceBanelingEscape_Networking/assets/33635742/b33f0602-77f5-492b-8f6d-9dcf8b3a6a9e)

![283235957-0be1e410-22aa-49c1-a62b-5dd376c59f50](https://github.com/GiboDidact/IceBanelingEscape_Networking/assets/33635742/a1e1abee-83a6-4714-9b9c-4cacedf55d25)

![283235952-b63b7d45-dc17-4a5e-ba28-c3289b044013](https://github.com/GiboDidact/IceBanelingEscape_Networking/assets/33635742/df366b86-8e93-4d3c-9404-0f1187f10f28)

![283235961-ce86dbbb-9457-4381-b034-eedb2b9a0fe1](https://github.com/GiboDidact/IceBanelingEscape_Networking/assets/33635742/3c5d5b43-8a33-4cc5-a2b5-93177ef83445)




