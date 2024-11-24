// SPDX-License-Identifier: MIT
pragma solidity ^0.8.18;

/**
 * @title BettingMarket
 * @dev This struct represents a betting market with details about the market owner, name, resolution time, and betting outcomes.
 * @param owner The address of the owner who created the betting market.
 * @param name The name of the betting market.
 * @param resolvesBy The timestamp by which the market will be resolved.
 * @param resolved A boolean indicating whether the market has been resolved.
 * @param winner A boolean indicating the outcome of the market (true for outcome A, false for outcome B).
 * @param totalBetsA The total amount of bets placed on outcome A.
 * @param totalBetsB The total amount of bets placed on outcome B.
 */
struct BettingMarket
{
    address owner;
    string name;
    uint256 resolvesBy;
    bool resolved;
    bool winner;

    uint256 totalBetsA;
    uint256 totalBetsB;
}


// ------------------------------ Frozy Market ------------------------------
//
// The very first ice cold betting market on the blockchain.
// Bet on the outcome of a market and win big if you are right.

contract Frozymarket
{
    address owner;
    mapping(uint marketindex => mapping(address user => mapping(bool AorB => uint256 amount))) bets;
    BettingMarket[] public markets;

    uint256 constant BPS = 10_000;

    /**
     * @dev Initializes the contract setting the deployer as the initial owner.
     */
    constructor()
    {
        owner = msg.sender;
    }

    /**
     * @dev Modifier to make a function callable only by the owner.
     * Reverts with a custom error message if the caller is not the owner.
     */
    modifier onlyOwner()
    {
        require(msg.sender == owner, "Only owner can call this function");
        _;
    }
    
    /**
     * @notice Creates a new market with the specified name and resolution time.
     * @param name The name of the market to be created.
     * @param resolvesBy The timestamp by which the market should resolve.
     * @return The unique identifier of the newly created market.
     */
    function createMarket(string memory name, uint256 resolvesBy) public returns (uint256)
    {
        BettingMarket memory newMarket = BettingMarket(msg.sender, name, resolvesBy, false, false, 0, 0);
        markets.push(newMarket);
        return markets.length - 1;
    }


    /**
     * @notice Places a bet on a specified market with a chosen outcome.
     * @dev This function allows users to place bets on a market's outcome.
     *      The market must be active and not resolved.
     * @param marketIndex The index of the market to bet on.
     * @param outcome The chosen outcome to bet on (true or false).
     */
    function bet(uint256 marketIndex, bool outcome) public payable
    {
        require(marketIndex < markets.length, "Invalid market index");
        require(!markets[marketIndex].resolved, "Market has already resolved");

        if (outcome)
        {
            markets[marketIndex].totalBetsA += msg.value;
            bets[marketIndex][msg.sender][true] += msg.value;
        }
        else
        {
            markets[marketIndex].totalBetsB += msg.value;
            bets[marketIndex][msg.sender][false] += msg.value;
        }
    }

    /**
     * @notice Resolves a market by setting its resolved status and winner.
     * @param marketIndex The index of the market to resolve.
     * @param winner The outcome of the market (true or false).
     * @dev The market can only be resolved by its owner and after the resolve time has passed.
     * @dev Emits no events.
     * @dev Reverts if the market index is invalid, the market is already resolved, or the caller is not the market owner.
     */
    function resolveMarket(uint256 marketIndex, bool winner) public
    {
        require(marketIndex < markets.length, "Invalid market index");
        require(markets[marketIndex].resolvesBy < block.timestamp, "Market can not be resolved yet");
        require(!markets[marketIndex].resolved, "Market has already resolved");
        require(msg.sender == markets[marketIndex].owner, "Only the market owner can resolve the market");

        markets[marketIndex].resolved = true;
        markets[marketIndex].winner = winner;
    }

    /**
     * @notice Allows users to claim their winnings from a resolved market.
     * @param marketIndex The index of the market from which to claim winnings.
     * @dev The function checks if the market index is valid and if the market has been resolved.
     *      Depending on the outcome of the market, it calculates the user's share of the pot and transfers the winnings.
     *      The function follows the Checks-Effects-Interactions (CEI) pattern to prevent reentrancy attacks.
     *      If the user bet on the winning outcome, their bet amount is reset to zero before transferring the winnings.
     */
    function claimWinnings(uint256 marketIndex) public
    {
        require(marketIndex < markets.length, "Invalid market index");
        require(markets[marketIndex].resolved, "Market has not resolved yet");

        uint bpsOfPot;
    
        if (markets[marketIndex].winner)
        {
            require(bets[marketIndex][msg.sender][true] > 0, "You did not bet on the winning outcome");

            //Calc user share, in BPS for less rounding errors
            bpsOfPot = BPS * bets[marketIndex][msg.sender][true] / markets[marketIndex].totalBetsA;

            //Reset bet, we follow CEI pattern
            bets[marketIndex][msg.sender][true] = 0;
        }
        else
        {
            require(bets[marketIndex][msg.sender][false] > 0, "You did not bet on the winning outcome");

            //Calc user share, in BPS for less rounding errors
            bpsOfPot = BPS * bets[marketIndex][msg.sender][false] / markets[marketIndex].totalBetsB;

            //Reset bet, we follow CEI pattern
            bets[marketIndex][msg.sender][false] = 0;
        }

        uint256 payout = address(this).balance * bpsOfPot / BPS;

        //Transfer win to user
        (msg.sender).call{value: payout}("");
    }
}