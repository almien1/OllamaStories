# Models

This folder is a convenient default for the "Models folder" setting on the app's Llama.cpp tab.

- `recommended_models.html` — open in a browser for roleplay-suited GGUF models picked for a 12GB-VRAM machine,
  with sizes, VRAM fit, and download commands.
- `Download-LlamaModel.ps1` — fetches those models (or any direct `.gguf` URL) from Hugging Face.
  Run `.\Download-LlamaModel.ps1 -List` to see the curated list.

llama.cpp itself (`llama-server.exe` / `llama-cli.exe`) isn't downloaded by this script — grab a CUDA build from
[ggml-org/llama.cpp releases](https://github.com/ggml-org/llama.cpp/releases) and point the Llama.cpp tab's
"Server executable" at it.
