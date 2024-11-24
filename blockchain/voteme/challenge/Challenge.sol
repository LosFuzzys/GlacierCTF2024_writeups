// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

contract ChallengeContract
{
    address owner;

    // Contains whitelisted addresses which can register as a staker
    mapping(address => bool) public whitelist;

    struct Proposal {
        address target;
        bytes data;
        uint256 value;
    }

    // makes it possible to retrieve proposed proposals
    Proposal[] public proposals;

    // needed to retrieve proposal for hash
    mapping(bytes32 => Proposal) proposalsMapping;
    mapping(bytes32 => bool) executable;
    mapping(bytes32 => address[]) public votesMapping;

    address[] public stakers;
    // Mapping of staker to index in "stakers" (0 = nonexistent)
    mapping(address => uint256) public stakerMapping;

    // Indicates if a staker is currently involved in a vote -> cannot claimStake in that case
    mapping(address => uint256) public activeVote;

    uint256 slashed;
    
    uint256 MAX_256 = 2**256 - 1;
    uint256 STAKE_VALUE = 10 ether;

    constructor(address[] memory initialStakers) payable
    {
        require(msg.value >= initialStakers.length * 10 ether, "Not enough ETH provided");
        
        // whitelisting stakers
        stakers.push(address(0)); // the first element is a placeholder (stakerMapping holds indexes [1:])
        
        for (uint256 i = 0; i < initialStakers.length; ++i) {
            whitelist[initialStakers[i]] = true;
            stakerMapping[initialStakers[i]] = stakers.length;
            stakers.push(initialStakers[i]);
        }
        owner = msg.sender;
    }


    // == ADMIN FUNCTIONS ==

    function changeWhitelist(address[] calldata whitelisted, bool action) public {
        require(msg.sender == owner, "Not the owner");

        for (uint256 i = 0; i < whitelisted.length; ++i) {
            whitelist[whitelisted[i]] = action;
        }
    }

    function withdrawSlashed() public {
        require(msg.sender == owner, "Not the owner");
        
        require(slashed > 0, "Nothing to withdraw");

        uint256 _slashed = slashed;
        slashed = 0;
        (bool success, ) = msg.sender.call{value: _slashed}("");
        require(success, "Could not send slashed funds");
    }


    // == PUBLIC FUNCTIONS ==

    function bribe() public payable {
        require(msg.value >= 10 ether, "Come on, help me out a little bit");
        whitelist[msg.sender] = true;
    }

    function getProposals() public view returns (Proposal[] memory) {
        return proposals;
    }

    function getStakers() public view returns (address[] memory) {
        return stakers;
    }

    function executeProposal(bytes32 proposalHash) public {
        // all stakers need to have voted on it and no double votes!
        require(executable[proposalHash], "Not executable!");

        Proposal memory prop = proposalsMapping[proposalHash];

        // don't allow double-execution
        delete votesMapping[proposalHash];
        executable[proposalHash] = false;

        (bool success, ) = prop.target.call{value: prop.value}(prop.data);
        require(success, "Failed to execute call");
    }

    // This checks whether we have a 100% majority vote amongst the registered stakers
    function checkConsensus(bytes32 proposalHash) public {

        // We slash all possible duplicate voters
        for (address violator = hasDuplicates(proposalHash); violator != address(0); violator = hasDuplicates(proposalHash)) {
            slash(violator);
            purgeVotes(proposalHash, violator);
        }

        uint256 registeredStakers = stakers.length - 1; // account for address(0) placeholder!
        uint256 votedStakers = votesMapping[proposalHash].length;

        // We need a 100% consensus
        if (registeredStakers == votedStakers) {
            executable[proposalHash] = true;
        }
    }

    function numOfVotes(bytes32 proposalHash) public view returns (uint256) {
        return votesMapping[proposalHash].length;
    }

    function withdrawVote(bytes32 proposalHash) public {
        require(stakerMapping[msg.sender] != 0, "Not a staker");
        
        uint256 index = getIndex(votesMapping[proposalHash], msg.sender);
        require(index != MAX_256, "Did not vote for proposal");

        removeElement(votesMapping[proposalHash], index);

        activeVote[msg.sender] -= 1;
    }

    // Staker can unstake when having an active vote but claiming their stake is only possible after all votes are done or withdrawn
    function unstake() public {
        require(stakerMapping[msg.sender] != 0, "Not a staker");
        require(activeVote[msg.sender] == 0, "Cannot claim stake while having an active vote");

        removeElement(stakers, stakerMapping[msg.sender]);
        delete stakerMapping[msg.sender];

        (bool success, ) = msg.sender.call{value: STAKE_VALUE}("");
        require(success, "Could not refund stake");
    }

    // Registers a new staker
    function stake() public payable {
        require(whitelist[msg.sender], "Not whitelisted");
        require(msg.value == STAKE_VALUE, "Incorrect staking amount");
        
        // Can only stake once
        require(stakerMapping[msg.sender] == 0, "Already staked");

        stakerMapping[msg.sender] = stakers.length;
        stakers.push(msg.sender);
    }

    // This allows a staker to propose a calldata and other stakers to vote for it
    function vote(Proposal calldata proposal) public {
        require(stakerMapping[msg.sender] != 0, "Not a staker");
        
        // we don't want stakers to DoS checkConsensus!
        require(activeVote[msg.sender] <= 5, "Can only have five votes at the same time");

        bytes32 proposalHash = keccak256(abi.encode(proposal.target, proposal.data, proposal.value));

        // add proposal if it does not exist yet
        if (votesMapping[proposalHash].length == 0) {
            proposals.push(proposal);
            proposalsMapping[proposalHash] = proposal;
            executable[proposalHash] = false;
        }

        votesMapping[proposalHash].push(msg.sender);
        activeVote[msg.sender] += 1;
    }


    // == INTERNAL FUNCTIONS ==

    function removeElement(address[] storage list, uint256 index) internal {
        require(list.length > index, "Invalid index");
        list[index] = list[list.length - 1];
        list.pop();
    }

    function purgeVotes(bytes32 proposalHash, address violator) internal {
        // removes all occurences of the violator from the list of voters
        for (uint256 index = getIndex(votesMapping[proposalHash], violator); index != MAX_256; index = getIndex(votesMapping[proposalHash], violator)) {
            removeElement(votesMapping[proposalHash], index);
        }
    }

    function slash(address violator) internal {

        removeElement(stakers, stakerMapping[violator]);
        delete stakerMapping[violator];
        delete activeVote[msg.sender];

        // slashed funds are just kept in the contract and can be withdrawn by the owner
        slashed += STAKE_VALUE;
    }

    function hasDuplicates(bytes32 proposalHash) internal view returns (address) {
        address[] memory voters = votesMapping[proposalHash];
        for (uint256 i = 0; i < voters.length; ++i) {
            uint256 foundCount = 0;
            for (uint256 j = 0; j < voters.length; ++j) {
                if (voters[i] == voters[j]) {
                    foundCount++;
                }
            }
            // we will get one found count, namely the element itself, any more means we have a dupe
            if (foundCount > 1) {
                return voters[i];
            } 
        }
        return address(0);
    }

    // Used to handle votesMapping (can have duplicate entries)
    function getIndex(address[] storage list, address element) internal view returns (uint256) {
        for (uint256 i = 0; i < list.length; ++i) {
            if (list[i] == element) {
                return i;
            }
        }
        return MAX_256;
    }
}