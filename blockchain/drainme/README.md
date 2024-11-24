# Challengetempalte Solidity

## Things to change as a challenge autor
The following things have to be adepted by the challenge autor:

## Setup.sol
In this file, the challenge author must update:
* The amount of etherium that shall be mined.
* The "WIN"-Condition in the function `isSolved`. This function will be called by the paradigm-framework

## Error-Codes

The Framework will return error-codes to the user.
| Error-Code |    Meaning                                                  |
| ---------- | ----------------------------------------------------------- |
|      1     | The provided action is not a number                         |
|      2     | The ticket provided does not fullfill our POW-Requirements  |
|      3     | No instance was found for this ticket                       |