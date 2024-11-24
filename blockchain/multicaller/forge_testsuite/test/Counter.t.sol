// Description:
// A forge testcase which you can use to easily debug challenges that were built using the Paradigm CTF framework.


// SPDX-License-Identifier: MIT
pragma solidity ^0.8;

import "forge-std/Test.sol";
import "../src/Challenge.sol";
import "../src/Setup.sol";
import "../src/Attacker.sol";
//Import all needed contracts here (they are usually stored in /src in your foundry directory)

contract ParadigmTest is Test {
    address deployer = makeAddr("deployer");
    address attacker = makeAddr("attacker");
    //Initialize any additional needed variables here
    Setup setup;

    function setUp() public {
	    vm.deal(deployer, 1337 ether);
        vm.startPrank(deployer);

        setup = new Setup{value: 100 ether}();

        vm.stopPrank();
    }

    function test() public {
        //30 eth are the standard for the paradigm framework, but this could be configured differently, you can easily check this by importing the rpc url and private key into metamask and checking the balance of the deployer account
        vm.deal(attacker, 30 ether); 

        //Code your solution here
        vm.startPrank(attacker);

        Attacker attackerContract = new Attacker(address(setup.TARGET()));

        attackerContract.attack{value: 1 ether}();

        vm.stopPrank();
        
        assertEq(isSolved(), true);
    }

    function isSolved() public view returns (bool) {
        //Copy the content of the isSolved() function from the Setup.sol contract here
        return setup.isSolved();
    }
}