import hashlib
import json
import os
import random
import string
import time
from dataclasses import dataclass
from typing import Callable
from typing import Dict
from typing import List
from typing import Optional
from uuid import UUID

import requests
from eth_account import Account
from eth_sandbox import get_shared_secret
from web3 import Web3
from web3.exceptions import TransactionNotFound
from web3.types import TxReceipt

HTTP_PORT = os.getenv("HTTP_PORT", "8545")
PUBLIC_IP = os.getenv("PUBLIC_IP", "127.0.0.1")

CHALLENGE_ID = os.getenv("CHALLENGE_ID", "challenge")
ENV = os.getenv("ENV", "dev")
FLAG_PATH = os.getenv("CHALLENGE_FLAG_PATH", "/flag.txt")
Account.enable_unaudited_hdwallet_features()


def check_ticket(ticket: str) -> int:
    m = hashlib.sha256()
    m.update(ticket.encode('ascii'))
    digest1 = m.digest()
    m = hashlib.sha256()
    m.update(digest1 + ticket.encode('ascii'))

    if m.hexdigest().startswith('0000000'):
        m = hashlib.sha256()
        m.update(f"{ticket}_glacierctf_random_seed_stumble_reptile_starless_shakable".encode("ascii"))
        return str(int(m.hexdigest()[3:17], 16))

    return None


@dataclass
class Action:
    name: str
    handler: Callable[[], int]


def sendTransaction(web3: Web3, tx: Dict) -> Optional[TxReceipt]:
    if "gas" not in tx:
        tx["gas"] = 10_000_000

    if "gasPrice" not in tx:
        tx["gasPrice"] = 0

    # web3.provider.make_request("anvil_impersonateAccount", [tx["from"]])
    txhash = web3.eth.sendTransaction(tx)
    # web3.provider.make_request("anvil_stopImpersonatingAccount", [tx["from"]])

    while True:
        try:
            rcpt = web3.eth.getTransactionReceipt(txhash)
            break
        except TransactionNotFound:
            time.sleep(0.1)

    if rcpt.status != 1:
        raise Exception("failed to send transaction")

    return rcpt


def new_launch_instance_action(do_deploy: Callable[[Web3, str], str]):
    def action() -> int:
        ticket = check_ticket(input("Ticket: "))
        if not ticket:
            print("Invalid ticket (Errorcode 2)")
            return 1

        data = requests.post(
            f"http://127.0.0.1:{HTTP_PORT}/new",
            headers={
                "Authorization": f"Bearer {get_shared_secret()}",
                "Content-Type": "application/json",
            },
            data=json.dumps(
                {
                    "team_id": ticket,
                }
            ),
        ).json()

        if data["ok"] == False:
            print(data["message"])
            return 1

        uuid = data["uuid"]
        mnemonic = data["mnemonic"]
        
        deployer_acct = Account.from_mnemonic(mnemonic, account_path=f"m/44'/60'/0'/0/0")
        player_acct = Account.from_mnemonic(mnemonic, account_path=f"m/44'/60'/0'/0/1")

        web3 = Web3(Web3.HTTPProvider(
            f"http://127.0.0.1:{HTTP_PORT}/{uuid}",
            request_kwargs={
                "headers": {
                    "Authorization": f"Bearer {get_shared_secret()}",
                    "Content-Type": "application/json",
                },
            },
        ))

        setup_addr = do_deploy(web3, deployer_acct.address, player_acct.address)

        with open(f"/tmp/{ticket}", "w") as f:
            f.write(
                json.dumps(
                    {
                        "uuid": uuid,
                        "mnemonic": mnemonic,
                        "address": setup_addr,
                    }
                )
            )

        print()
        print(f"Your private blockchain has been deployed,")
        print(f"it will automatically terminate in 30 minutes")
        print(f"Please remember your ticket, you might need it later")
        print(f"Here's some useful information")
        print(f"UUID:           {uuid}")
        print(f"RPC-Endpoint:   http://{PUBLIC_IP}:{HTTP_PORT}/{uuid}")
        print(f"Private key:    {player_acct.privateKey.hex()}")
        print(f"Setup contract: {setup_addr}")
        return 0

    return Action(name="launch new instance", handler=action)


def new_kill_instance_action():
    def action() -> int:
        ticket = check_ticket(input("Ticket: "))
        if not ticket:
            print("Invalid ticket (Errorcode 2)")
            return 1

        data = requests.post(
            f"http://127.0.0.1:{HTTP_PORT}/kill",
            headers={
                "Authorization": f"Bearer {get_shared_secret()}",
                "Content-Type": "application/json",
            },
            data=json.dumps(
                {
                    "team_id": ticket,
                }
            ),
        ).json()

        print(data["message"])
        return 1

    return Action(name="kill instance", handler=action)

def is_solved_checker(web3: Web3, addr: str) -> bool:
    result = web3.eth.call(
        {
            "to": addr,
            "data": web3.sha3(text="isSolved()")[:4],
        }
    )
    return int(result.hex(), 16) == 1


def new_get_flag_action(checker: Callable[[Web3, str], bool] = is_solved_checker):
    def action() -> int:
        ticket = check_ticket(input("Ticket: "))
        if not ticket:
            print("Invalid ticket (Errorcode 2)")
            return 1

        try:
            with open(f"/tmp/{ticket}", "r") as f:
                data = json.loads(f.read())
        except:
            print("Bad ticket (Errorcode 3)")
            return 1

        web3 = Web3(Web3.HTTPProvider(f"http://127.0.0.1:{HTTP_PORT}/{data['uuid']}"))

        if not checker(web3, data['address']):
            print("Are you sure you solved it?")
            return 1

        # Print the flag from the file
        with open(FLAG_PATH, "r") as flag_file:
            print(flag_file.read().strip())
        return 0

    return Action(name="get flag", handler=action)


def run_launcher(actions: List[Action]):
    print("In order to spawn a instance, you first need to create a tickt using solve-pow.py")
    print("Please choose on of the following options:")
    for i, action in enumerate(actions):
        print(f"{i+1} - {action.name}")
    print("What do you want to do?")

    try:
        action = int(input("> ")) - 1
        if action < 0 or action >= len(actions):
            print("can you not")
            exit(1)
    except:
        print("Error, please try again (Errorcode 1)")

    exit(actions[action].handler())
