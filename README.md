# IceBanelingEscape_Networking
Networked multiplayer game with TCP lobby system and UDP dedicated server

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
