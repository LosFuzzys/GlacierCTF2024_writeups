// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;


import "./Challenge.sol";

contract SharesBuyer {

  IChallengeContract target;

  constructor(address _target) {
    target = IChallengeContract(_target);
  }

  receive() external payable {}

  function buyShares() public {
    target.depositEth{value: address(this).balance}();
  }
}