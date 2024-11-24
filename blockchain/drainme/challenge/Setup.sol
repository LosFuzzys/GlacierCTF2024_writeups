// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

import "./Challenge.sol";
import "./SB.sol";

contract Setup {
    ChallengeContract public immutable TARGET; // Contract the player will hack
    SharesBuyer public immutable SB;


    constructor() payable {
        require(msg.value == 100 ether, "Not enough intial funds");

        // Deploy the victim contract
        TARGET = new ChallengeContract();
        SB = new SharesBuyer(address(TARGET));

        (bool success,) = address(SB).call{value: 100 ether}("");
        require(success, "Sending ETH to SB failed");

    }

    // Our challenge in the CTF framework will call this function to
    // check whether the player has solved the challenge or not.
    function isSolved() public view returns (bool) {
        return (address(TARGET).balance == 0 && address(SB).balance == 0);
    }
}