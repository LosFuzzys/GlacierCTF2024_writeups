# Writeup

The problem is that in a multicall (delegatecall) `msg.value` will be reused. So each time you call to deposit, the function will think you newly sent 1 eth, while you actually only sent it once.