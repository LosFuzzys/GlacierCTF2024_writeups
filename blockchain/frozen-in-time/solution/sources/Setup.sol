// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "./Challenge.sol";

contract Setup {
    Frozymarket public immutable TARGET; // Contract the player will hack

    constructor() payable {
        require(msg.value == 100 ether);

        // Deploy the contract
        TARGET = new Frozymarket();

        //Create market on the contract
        TARGET.createMarket("Will the price of ETH be higher than $2000 on 1st January 2022?", 1640995200);

        // Users place bets on the contract
        TARGET.bet{value: 10 ether}(0, true);
        TARGET.bet{value: 10 ether}(0, false);
    }

    // Our challenge in the CTF framework will call this function to
    // check whether the player has solved the challenge or not.
    function isSolved() public view returns (bool) {
        return address(TARGET).balance == 0;
    }
}