import os
import json
from pathlib import Path

import eth_sandbox
from web3 import Web3

CHALLENGE_MIN_ETHER = int(os.getenv("CHALLENGE_MIN_ETHER", 100))


def deploy(web3: Web3, deployer_address: str, player_address: str) -> str:
    rcpt = eth_sandbox.sendTransaction(web3, {
        "from": deployer_address,
        # CHALLENGE_MIN_ETHER is the minimum amount of ether to start with
        "value": Web3.toWei(CHALLENGE_MIN_ETHER, 'ether'), 
        "data": json.loads(Path("compiled/Setup.sol/Setup.json").read_text())["bytecode"]["object"],
    })

    return rcpt.contractAddress

eth_sandbox.run_launcher([
    eth_sandbox.new_launch_instance_action(deploy),
    eth_sandbox.new_kill_instance_action(),
    # the implementation of this calls isSolved() on Setup contract
    eth_sandbox.new_get_flag_action() 
])