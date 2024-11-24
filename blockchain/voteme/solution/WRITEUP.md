The problem is that when we slash a violator, they do not get removed from all proposals they voted for. If now a staker has double-voted for two proposals and get slashed for one of them, when they later get slashed for the second one, the first staker in the stakers array gets slashed instead.

* bribe() to get on the whitelist
1) stake() to become a staker
2) vote() four times, twice for the same proposal
3) checkConsensus() on the first proposal -> we get slashed
4) checkConsensus() on the second proposal -> another staker gets slashed
5) repeat from 1) until all stakers have been slashed
6) stake() again
7) vote() for a new proposal which drains the contract
8) checkConsensus() -> passes because we are now the sole staker
9) executeProposal() -> get all funds --profit--