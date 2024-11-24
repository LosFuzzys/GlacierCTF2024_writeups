Typical inflation attack:

* donate funds to contract (by selfdestructing since receive reverts) -> deploy Solve contract
* buy 1 share for 1 wei -> call depositEth with 1wei
* let victim buy shares -> call SB.buyShares which triggers a buy from the victim
* => since we donated, the victim's received shares gets rounded to 0
* call withdrawEth, getting the whole balance of the contract for 1 share