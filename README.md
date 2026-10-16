# Pipecat Room

![Maya, Theo and Juno waving in the hall](docs/hello.jpg)

Walk around a house and talk to the three people who live there: Maya (a
botanist, in the conservatory), Theo (a chef, in the kitchen) and Juno (a
musician, in the music room). It's a [Pipecat](https://github.com/pipecat-ai/pipecat)
voice agent in an Unreal Engine game: one conversation, three characters, each
with their own voice, coming from where they stand. Only those close enough hear
you.

## Requirements

- Windows (x86_64) with a ray tracing GPU (best on NVIDIA RTX).
- Unreal Engine 5.8.
- Visual Studio (or its Build Tools) with "Desktop development with C++",
  "C++ CMake tools for Windows" and the ".NET Framework 4.8 SDK".
- The [Pipecat C++ client](https://github.com/pipecat-ai/pipecat-client-cxx)
  source.
- [uv](https://docs.astral.sh/uv/), and WSL for the bot (`wsl --install`).
- API keys: OpenAI, Cartesia, Deepgram, Jev (TypeSafe) and Daily.
- A microphone. Headphones are best.
- Optional: NVIDIA's DLSS 4.5 plugin for UE 5.8 (unzip `DLSS` and `Streamline*`
  into `Plugins/`). Without it, the game uses TSR.

## Build

In PowerShell, from this directory:

```
$env:UE_ROOT = "C:\Program Files\Epic Games\UE_5.8"
$env:PIPECAT_CLIENT_CXX = "C:\path\to\pipecat-client-cxx"
.\Plugins\Pipecat\ThirdParty\build-windows.ps1
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" PipecatRoomEditor Win64 Development -Project="$PWD\PipecatRoom.uproject"
.\setup.ps1
```

`build-windows.ps1` builds the client and its Daily and WebSocket transports
(downloading Daily's Core SDK the first time). `setup.ps1` copies Unreal's Third
Person template content and creates the game's materials.

## Run

1. Start the bot:

   ```
   cd bot
   copy env.example .env   # then add your API keys
   wsl bash ./run-wsl.sh
   ```

   Without WSL, run `uv run bot.py -t websocket` and start the game with
   `-PipecatTransport=websocket`.

2. Start the game:

   ```
   & "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor.exe" "$PWD\PipecatRoom.uproject" -game
   ```

Walk with WASD and the mouse, or a controller. Find someone and say hello. Press
**E** (X on Xbox, square on DualSense) to use what's near you: the gramophone,
the piano, the dinner bell, the cake, the flowers, the fountain, the sculptures,
or to give someone what you're holding.

Things to say: "follow me", "wait here", "let's meet in the gallery", "go back to
what you were doing", "can you introduce me to Juno?", "could I have some cake?",
"pick me a flower", "put a record on", "who wants to dance?".

## Multiplayer

This is a two-player **listen-server** demo: the host plays and runs the server.
Both machines need the same project version and the normal setup above. Start
the bot on the host as usual, then host the generated house (PowerShell):

```powershell
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor.exe" "$PWD\PipecatRoom.uproject" /Engine/Maps/Entry?listen -game -port=7777 -log -AdvertiseHost=203.0.113.10
```

Replace `203.0.113.10` with the host's public IPv4 address or DNS name for an
Internet guest, or its LAN IPv4 address on the same network. Without
`-AdvertiseHost=`, the game detects a local address, which is normally only
useful on a LAN. The log prints `Join link: unreal://203.0.113.10:7777`;
`Room.JoinLink` in the console prints it again. The link uses the game port
(`-port=` changes it). You can also host from a running standalone game with
`open /Engine/Maps/Entry?listen`.

Send that single link to the guest. They can launch with it:

```powershell
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor.exe" "$PWD\PipecatRoom.uproject" -game -ConnectUrl=unreal://203.0.113.10:7777 -log
```

Or launch normally, press **~** to open the Unreal console, and paste
`Room.Join unreal://203.0.113.10:7777`. `Room.Join 203.0.113.10:7777` also works.
These are IPv4/DNS endpoint links, without a map path or URL options; the
server supplies the map. No browser/OS `unreal://` handler is installed.

**Network/ports:** allow inbound **UDP 7777** (or the selected game port) in
the host's firewall and forward that same UDP port from the router to the host.
There is no matchmaking, NAT traversal, authentication or encryption here;
use trusted guests, preferably over a private VPN. Carrier-grade NAT usually
requires a VPN or a publicly reachable host. The bot's **TCP 7860** serves
`/start` and, with the WebSocket transport, the WebSocket connection on the
same port. Only the host connects to the bot: keep that port local, not
Internet-forwarded. If the host uses a bot on another machine, allow its
configured HTTP/WS port between those machines and pass `-PipecatStartUrl=...`
on the host. Daily still needs its normal outbound Internet access.

**First-slice limits:** player spawning and movement are server-authoritative;
both players see the template mannequins moving with their normal locomotion.
The house and initial furnishings/collision are built locally on each machine.
Voice, captions, NPC simulation and **E** interactions remain **host-only**.
Guests do not start a Pipecat session or capture a microphone, do not see the
host's NPCs, and do not receive changes to items, music, or other room state.
There is no shared Daily/WebSocket session or player-to-player voice. A guest
launched normally may briefly run their old single-player bot until travel
finishes; use `-ConnectUrl=` to skip that startup session entirely.

To check on two machines (or two separate `-game` processes on one machine,
using `127.0.0.1:7777`): host, join, walk in both windows, and confirm each
player sees the other move. On the host, speak to a bot and try **E**. Check
that the guest log does not start a bot connection when using `-ConnectUrl=`.
Repeat joining via `Room.Join`, and test an Internet guest after port forwarding.

## Options

- `-PipecatStartUrl=URL`: the bot's start endpoint (default
  `http://localhost:7860/start`).
- `-PipecatMicrophone=NAME`: the microphone to use, by part of its name.
- `-PipecatNoMicrophone -PipecatSay="Hello?"`: try it without speaking.
- `-PipecatNoLogo`: skip the logo.

`Config/DefaultGame.ini` has the voice and hearing distances. `bot/characters.json`
and `bot/prompts/` define the characters. To use PhoneLLM instead of OpenAI, set
`PHONELLM_API_KEY`, `PHONELLM_BASE_URL` and `LLM_MODEL` in `bot/.env`.

## Checks

```
cd bot
uv run ruff check . ; uv run pyright ; uv run pytest -q
```

## License

BSD 2-Clause: see [LICENSE](LICENSE).
