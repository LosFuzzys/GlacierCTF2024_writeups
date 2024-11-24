// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "./Challenge.sol";

contract Setup {
    ChallengeContract public immutable TARGET; // Contract the player will hack

    constructor() payable {
        require(msg.value == 100 ether);

        address[] memory stakers = new address[](5);
        stakers[0] = address(1);
        stakers[1] = address(2);
        stakers[2] = address(3);
        stakers[3] = address(4);
        stakers[4] = address(5);

        // Deploy the victim contract
        TARGET = new ChallengeContract{value: stakers.length * 10 ether}(stakers);
    }

    // Our challenge in the CTF framework will call this function to
    // check whether the player has solved the challenge or not.
    function isSolved() public view returns (bool) {
        return address(TARGET).balance == 0;
    }
}