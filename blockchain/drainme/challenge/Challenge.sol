// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

interface IChallengeContract {
    function depositEth() external payable;
    function withdrawEth(uint256) external;
}

contract ChallengeContract
{
    address owner;
    uint256 public totalShares;
    mapping(address => uint) public balances;

    constructor()
    {
        totalShares = 0;
        owner = msg.sender;
    }

    receive() external payable { revert(); } // no donations

    function depositEth() public payable {
        uint256 value = msg.value;
        uint256 shares = 0;

        require(value > 0, "Value too small");

        if (totalShares == 0) {
            shares = value;
        }
        else {
            shares = totalShares * value / address(this).balance;
        }
        
        totalShares += shares;
        balances[msg.sender] += shares;
    }

    function withdrawEth(uint256 shares) public {
        require(balances[msg.sender] >= shares, "Not enough shares");

        uint256 value = shares * address(this).balance / totalShares;

        totalShares -= shares;
        balances[msg.sender] -= shares;

        (bool success,) = address(msg.sender).call{value: value}("");
        require(success, "ETH transfer failed");
    }
}