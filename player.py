import gymnasium as gym
import subprocess as sp

class SolitaireEnv(gym.Env):
    def __init__(self):
        deck_space = gym.spaces.Sequence(gym.spaces.Discrete(52)) 
        self.observation_space = gym.spaces.Dict({
            "draw": deck_space, 
            "wastes": deck_space, 
            "t0": deck_space, # faceup tableaus 
            "t1": deck_space, 
            "t2": deck_space, 
            "t3": deck_space,
            "t4": deck_space,
            "t5": deck_space,
            "t6": deck_space, 
            "f0": deck_space, # foundations 
            "f1": deck_space,
            "f2": deck_space,
            "f3": deck_space
        })

        # 615 discrete actions. these are encoded/translated by the solitaire engine exe.
        self.action_space = gym.spaces.Discrete(615)

        self.process = None

    def readline_to_list(self):
        return list(map(int,self.process.stdout.readline().split(' ')[0:-1]))

    def proc_read_state(self): # this should match exactly the amount of lines output by output_state in solitaire.c
        return (
            {
                "draw": self.readline_to_list(), 
                "wastes": self.readline_to_list(), 
                "t0": self.readline_to_list(), # faceup tableaus 
                "t1": self.readline_to_list(), 
                "t2": self.readline_to_list(), 
                "t3": self.readline_to_list(),
                "t4": self.readline_to_list(),
                "t5": self.readline_to_list(),
                "t6": self.readline_to_list(), 
                "f0": self.readline_to_list(), # foundations 
                "f1": self.readline_to_list(),
                "f2": self.readline_to_list(),
                "f3": self.readline_to_list()
            },
            {
                "actions": self.readline_to_list()
            } # actions
        )

    def reset(self, seed=None, options=None):
        self.process = sp.Popen("./solitaire.exe", stdin=sp.PIPE, stdout=sp.PIPE, text=True, bufsize=0, encoding='ascii')



    def close(self):
        self.process.kill()
senv = SolitaireEnv()

senv.reset()

state,actions = senv.proc_read_state()
print(state)
print(actions)
