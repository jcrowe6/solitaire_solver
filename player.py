import gymnasium as gym
import subprocess as sp

process = sp.Popen("./a.exe", stdin=sp.PIPE, stdout=sp.PIPE, text=True, bufsize=0, encoding='ascii')

def proc_read_state(proc):
    draw = process.stdout.readline().split(' ')
    wastes = process.stdout.readline().split(' ')

    f0 = process.stdout.readline().split(' ')
    f1 = process.stdout.readline().split(' ')
    f2 = process.stdout.readline().split(' ')
    f3 = process.stdout.readline().split(' ')

    d0 = int(process.stdout.readline())
    d1 = int(process.stdout.readline())
    d2 = int(process.stdout.readline())
    d3 = int(process.stdout.readline())
    d4 = int(process.stdout.readline())
    d5 = int(process.stdout.readline())
    d6 = int(process.stdout.readline())

    t0 = process.stdout.readline().split(' ')
    t1 = process.stdout.readline().split(' ')
    t2 = process.stdout.readline().split(' ')
    t3 = process.stdout.readline().split(' ')
    t4 = process.stdout.readline().split(' ')
    t5 = process.stdout.readline().split(' ')
    t6 = process.stdout.readline().split(' ')

class SolitaireEnv(gym.Env):
    def __init__(self):
        deck_space = gym.spaces.Sequence(gym.spaces.Discrete(52)) 
        self.observation_space = gym.spaces.Tuple([
            deck_space, # draw
            deck_space, # wastes
            deck_space, # faceup tableau 0
            deck_space, # 1
            deck_space, # 2...
            deck_space,
            deck_space,
            deck_space,
            deck_space, # .. 6
            deck_space, # foundations 0 
            deck_space,
            deck_space,
            deck_space
            # leaving out the count of facedowns for now...
        ])

        # Action space: Draw + Flip + 11 waste moves + 4*7 foundation moves + 7*13*7 + 7*4 tableau moves
        # = 706 actions. many many of which are not legal solitaire moves in any given state
        # this is in the case where each move must be specified exactly...
        # Ideas on training... the agent has to pick one possible action, and while it's learning, if 
        # it tries randomly and likely picks an illegal action, what happens? the state doesn't change?
        # I think it would be better to offer the agent both the state and the legal actions 
        # (not only because I spent hours coding the engine to provide the legal actions in its output)
        # but also, what am I trying to do? have an AI learn to find cards that can be moved on top of eachother?
        # or to, given the entire solitaire board state, decide among some cards that may move, which will likeliest lead to success?
        # but again, how to train this? If we just provide the legal actions as part of the state,
        # it may learn to pick actions based on this part of the state, but I think that's a bit weird.
        # Some ideas from starcraft II AI paper: auto-regressive policy? masking out impossible actions?
        # I think I'll start with masking. Go forward with a DQN, and at action selection, mask out illegal actions
        self.action_space = gym.spaces.Discrete()

    
proc_read_state(process)

process.kill()