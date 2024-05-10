# Kosmic Hunters

A multiplayer first-person shooter built with Unreal Engine.

---
Note: the original repository for this project had to be deleted because it used Git LFS and quickly exceeded the monthly bandwidth. That's why this repository doesn't have commits older than <a href="https://github.com/xaviervitor/kosmichunters/commit/dd973a36ae636c00a5f6f21ea3e0cb3b7bda3a4c">14 Mar 2024</a>, despite initial work having started about December 2023.

## Instructions 
1. Clone the repository;
2. Open the `MultiplayerFPS.uproject` file;
3. (vscode) Select `Tools` > `Generate Visual Studio Code project`;
4. (vscode) Open the `MultiplayerFPS.code-workspace` file.

Lightmass built data can't be included in the repository. To bake the lighting of the levels:

5. (unreal) Select `Build` > `Lighting Quality` > `Production`;
6. (unreal) Select `Build` > `Build Lighting Only`;
