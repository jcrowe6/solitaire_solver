## Solitaire Solver

I was playing Solitaire one day and thought 

> All I'm doing is repeatedly searching for cards. We have computers to do this

So I began writing an engine in C as a "little" side project...

Which has spun into me discovering playing Solitaire actually does have some nuanced strategy and I'd like to see if I could apply reinforcement learning to it.

`solitaire.c` should compile to `solitaire.exe` and is an engine that outputs the state of the game and available actions.

Cards and actions are encoded as discrete numbers. The gymnasium package I've written executes `solitaire.exe` and should facilitate training an agent to play. 
