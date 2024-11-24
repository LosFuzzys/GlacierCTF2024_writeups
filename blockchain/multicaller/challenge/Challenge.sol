// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

// --------------- ArcticVault.sol ---------------
// The first artic vault, even safer than swiss banking ;)
// Deposit your funds, and the tax collector will never find them.
// We even offer the coldest flash loans in the world!

contract ArcticVault
{
    address owner;
    mapping(address => uint256) balances;
    bool paused;
    bool reentrancyGuard;

    modifier notPaused()
    {
        require(!paused, "Contract is paused");
        _;
    }

    event Donation(address indexed from, uint256 amount);

    event MyEvent(bytes data);

    constructor()
    {
        owner = msg.sender;
    }

    //Users can deposit funds into the contract
    function deposit() public payable notPaused
    {
        require(!reentrancyGuard, "Reentrancy guard is active");
        require(msg.value > 0, "Amount must be greater than 0");

        balances[msg.sender] += msg.value;
    }

    // Donate to the glacier
    function donate() public payable notPaused
    {
        require(!reentrancyGuard, "Reentrancy guard is active");
        require(msg.value > 0, "Amount must be greater than 0");

        owner.call{value: msg.value}("");
        
        emit Donation(msg.sender, msg.value);
    }

    //Users can withdraw funds from the vault
    function withdraw() public notPaused
    {
        require(!reentrancyGuard, "Reentrancy guard is active");
        require(balances[msg.sender] > 0, "You have no funds to withdraw");

        uint256 amount = balances[msg.sender];
        balances[msg.sender] = 0;

        payable(msg.sender).transfer(amount);
    }

    //Pause contract (in case the glacier gets infiltrated)
    function pause() public
    {
        require(!reentrancyGuard, "Reentrancy guard is active");
        require(msg.sender == owner, "You are not the owner of this contract");
        paused = true;
    }

    //Unpause contract (in case the glacier gets cleared)
    function unpause() public
    {
        require(!reentrancyGuard, "Reentrancy guard is active");
        require(msg.sender == owner, "You are not the owner of this contract");
        paused = false;
    }

    function flashLoan(uint256 amount) public notPaused
    {
        require(address(this).balance >= amount, "Owner has insufficient funds");

        uint256 balanceBefore = address(this).balance;

        //Do the flash loan
        reentrancyGuard = true;
        msg.sender.call{value: amount}("");
        reentrancyGuard = false;

        require(address(this).balance == balanceBefore, "Flash loan failed");
    }

    // ------------------ Utils to make your life easier ------------------


    //Multicall for other contracts (saves gas)
    function multicallOthers(address[] memory _targets, bytes[] memory _data) public payable 
    {
        require(!reentrancyGuard, "Reentrancy guard is active");
        require(_targets.length == _data.length, "Arrays must be the same length");

        for(uint256 i = 0; i < _targets.length; i++)
        {
            (bool success, ) = _targets[i].call(_data[i]);
            require(success, "Transaction failed");
        }
    }

    //Multicall for this contract (saves gas)
    function multicallThis(bytes[] memory _data) public payable
    {
        require(!reentrancyGuard, "Reentrancy guard is active");

        for(uint256 i = 0; i < _data.length; i++)
        {
            (bool success, ) = address(this).delegatecall(_data[i]);
            require(success, "Transaction failed");
        }
    }

    //Carve your personalized event into the ice
    function emitEvent(bytes memory _data) public
    {
        require(!reentrancyGuard, "Reentrancy guard is active");

        emit MyEvent(_data);
    }
}