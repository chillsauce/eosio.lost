#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/permission.h>

#define USE_KECCAK
#include "sha3/sha3.c"
#include "ecc/uECC.c"

using namespace eosio;
using namespace std;

typedef std::string ethereum_address;

#ifndef WAITING_PERIOD
#define WAITING_PERIOD 60 * 60 * 24 * 30
#endif
#ifndef WHITELIST_CONTRACT
#define WHITELIST_CONTRACT "whitelist111"
#endif


class [[eosio::contract("eosio.lost")]] lostcontract : public contract {

private:

    TABLE verify_info{
            name              claimer;
            time_point_sec    added;
            public_key        new_key;
            uint8_t           updated;

            uint64_t primary_key() const { return claimer.value; }

            EOSLIB_SERIALIZE(verify_info,
                            (claimer)
                            (added)
                            (new_key)
                            (updated))
    };
    typedef multi_index<"verified"_n, verify_info> verifications_table;



#include "whitelist/defs.hpp"


    void assert_unused(name account);
    void assert_whitelisted(name account);
    std::string bytetohex(unsigned char *data, int len);

public:

    using contract::contract;

    ACTION updateauth(name claimer);

    ACTION verify(std::vector<char> sig, name account, public_key newpubkey, name rampayer);

    ACTION reset(name claimer);

    ACTION useaccount(name claimer);

    ACTION notify(name claimer);

    void donotify(name claimer, string msg);

    ACTION clear();

};


//Authority Structs
namespace eosiosystem {

    struct key_weight {
        eosio::public_key key;
        uint16_t weight;

        // explicit serialization macro is not necessary, used here only to improve compilation time
        EOSLIB_SERIALIZE(key_weight, (key)(weight))
    };

    struct permission_level_weight {
        permission_level permission;
        uint16_t weight;

        // explicit serialization macro is not necessary, used here only to improve compilation time
        EOSLIB_SERIALIZE(permission_level_weight, (permission)(weight))
    };

    struct wait_weight {
        uint32_t wait_sec;
        uint16_t weight;

        // explicit serialization macro is not necessary, used here only to improve compilation time
        EOSLIB_SERIALIZE(wait_weight, (wait_sec)(weight))
    };

    struct authority {

        uint32_t threshold;
        vector <key_weight> keys;
        vector <permission_level_weight> accounts;
        vector <wait_weight> waits;

        EOSLIB_SERIALIZE(authority, (threshold)(keys)(accounts)(waits))
    };
}

struct producer_info {
    name owner;
    double total_votes = 0;
    eosio::public_key producer_key; /// a packed public key object
    bool is_active = true;
    std::string url;
    uint32_t unpaid_blocks = 0;
    time_point last_claim_time;
    uint16_t location = 0;

    uint64_t primary_key() const { return owner.value; }

    double by_votes() const { return is_active ? -total_votes : total_votes; }

    bool active() const { return is_active; }

    void deactivate() {
        producer_key = public_key();
        is_active = false;
    }

    // explicit serialization macro is not necessary, used here only to improve compilation time
    EOSLIB_SERIALIZE( producer_info, (owner)(total_votes)(producer_key)(is_active)(url)
            (unpaid_blocks)(last_claim_time)(location)
    )
};

typedef eosio::multi_index<"producers"_n, producer_info,
        indexed_by<"prototalvote"_n, const_mem_fun < producer_info, double, &producer_info::by_votes> >
>
producers_table;
