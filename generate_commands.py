#!/usr/bin/python3

import sys
import random

COMMANDS_TYPES=["bid","offer"]
SYMBOLS=["msft","aapl","aegr"]
MIN_PRICE=1.0
MAX_PRICE=1000.0
MIN_VOLUME=1
MAX_VOLUME=100000

if __name__ == '__main__':
    num_commands = 100
    if len(sys.argv) >= 2:
        num_commands = int(sys.argv[1])
    
    for i in range(num_commands):
        print("%s %s %.2f %d" % (
            random.choice(COMMANDS_TYPES),
            random.choice(SYMBOLS),
            random.uniform(MIN_PRICE, MAX_PRICE),
            int(random.uniform(MIN_VOLUME, MAX_VOLUME))))
    print("exit")