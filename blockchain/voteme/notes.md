# Idea

Have whitelisted validators that can register. In order to make some decision (execute arbitrary call), we need a majority vote on a certain calldata that gets proposed.

Some vuln so we can register multiple times (have the same signature/validator in an array multiple times for example)




# Vuln

The index in the votes mapping is taken globally across all proposals

We have an initial state with one proposal on which all stakers voted

## Attack path:

* vote multiple times on proposal with all stakers
* create new proposals and vote [1, 2, ..., 5] times for them
* after each new proposal with the proper amount of votes, try to finalize the fully voted proposal
* since we voted multiple times, we get slashed
* since we manipulated the global index, the wrong staker gets slashed
* this reduces the staker set with every attack, allowing us to slash all stakers and remain the sole staker in the end, draining the protocol



BETTER:
# Vuln second option

Have multiple proposals initially and violate voting on all of them
When we slash, we slash a non-existent staker, causing us to slash index0, removing the first staker from the staker set

## Attack path

* create 5 proposals, vote twice on all of them
* get slashed on one of them, removing staker from the set completely
* MAYBE have function to purge staker from all mappings and arrays
* repeat process, now when slashing we get index0 since we try to slash someone who does not have an entry in the indexes mapping
* slash index0, removing them from the stakerset until there is noone left