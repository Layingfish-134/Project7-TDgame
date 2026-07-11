SQLite runtime integration

This project loads sqlite3.dll dynamically at runtime. Put sqlite3.dll next to Game.exe
to enable real SQLite-backed save.db storage. If the DLL is missing, SaveManager keeps
the same save.db filename but falls back to a lightweight text format so the game can
still run during development.
