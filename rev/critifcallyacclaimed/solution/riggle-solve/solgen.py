import json

moves = json.load(open("player_movement.json"))

x,y = 15,15
for i, move in enumerate(moves):
    actions = ""
    pos = move[0]
    a = move[1]
    o = move[2]

    if pos[0] < x:
        actions += "l" * (x - pos[0])
    elif pos[0] > x:
        actions += "r" * (pos[0] - x)
    if pos[1] < y:
        actions += "d" * (y - pos[1])
    elif pos[1] > y:
        actions += "u" * (pos[1] - y)

    if o == 1 and actions[-1] != "u":
        actions += "du"
    elif o == 2 and actions[-1] != "d":
        actions += "ud"
    elif o == 3 and actions[-1] != "l":
        actions += "rl"
    elif o == 4 and actions[-1] != "r":
        actions += "lr"

    if a != "n":
        actions += a

    x,y = pos

    if i == 4:
        x, y = 29, 15
    if i == 48:
        x, y = 29, 25

    print(actions)
