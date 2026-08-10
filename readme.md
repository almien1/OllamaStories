# LlamaStories

## Running stories (llama.cpp)

The "Run" tab talks directly to a local [llama.cpp](https://github.com/ggml-org/llama.cpp) `llama-server`, in a mode
tuned for in-character roleplay chat (sampling defaults suited to creative dialogue, and `--jinja` so the model's own
chat template is respected).

- Download `llama-server.exe`/`llama-cli.exe` and a `.gguf` roleplay model (see `models/README.md` and
  `models/recommended_models.html` for models that suit this machine, and `models/Download-LlamaModel.ps1` to fetch them)
- On the **Llama.cpp** tab, point "Server executable" at `llama-server.exe` and "Models folder" at the folder holding
  your `.gguf` files, then pick a model. "Reset sampling to roleplay defaults" restores the recommended settings.
- Write your story prompt on the Stories tab and any shared rules on the Global tab
- 'Run' (Ctrl-R) starts (or reuses) the llama.cpp server and chats with it inside the editor
- 'Run detached' (Ctrl-T) opens a command line window running `llama-cli` interactively with the same prompt/settings
- Ctrl-C will copy the story and the global prompt into the clipboard, and could be pasted into a LLM website chat window

## Compiling an Ollama model (optional)

The **Compile (Ollama)** tab and the "Compile" button are unchanged: they still build an [Ollama](https://ollama.com)
model from your prompt via `ollama create`, for anyone who prefers to run their stories through Ollama instead.

- Install Ollama, choose a model name and an installed Ollama model (e.g. `gemma3:12b`), set temperature/context, then
  press Compile. This generates a new model with your prompt that will be visible in `ollama list` and can be run using
  `ollama run story` (if you used "story" as the name).

## Multiple stories

Perhaps you have a successful prompt and would like to explore themes around that idea

Move the "common" part of your prompt into the Global tab, leaving just the theme-specific part in the stories tab. 

Then create another story with a different theme.

The 'Compile' button will combine your selected story with the global prompt.




