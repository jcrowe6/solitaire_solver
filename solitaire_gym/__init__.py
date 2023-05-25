from gymnasium.envs.registration import register

register(
    id="solitaire_gym",
    entry_point="solitaire_gym.envs:SolitaireEnv",
    max_episode_steps=300,
)