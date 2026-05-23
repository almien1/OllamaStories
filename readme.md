# Ollama stories

## Basic steps

- Install Ollama
- Install and test a model - for example `ollama run gemma3:12b` (search for the model which fits your graphics card)
- Run this program
  - Create a prompt for a story
  - Save it to a project file
  - Set the options
    - Choose a name, e.g. "stories" - this will become the name inside ollama
    - Enter the name of the model, e.g. `gemma3:12b`
    - Set the temperature to at least 50% to get some creative output
    - Set the context size
  - Press compile. 
  
This generates a new model with your prompt that will be visible in `ollama list` and can be run using `ollama run story` (if you used story as the name)

## Multiple stories

Perhaps you have a successful prompt and would like to explore themes around that idea

Move the "common" part of your prompt into the Global tab, leaving just the theme-specific part in the stories tab. 

Then create another story with a different theme.

The 'Compile' button will combine your selected story with the global prompt.


