#pragma once

#include <graphene/chain/protocol/operations.hpp>
#include <graphene/db/generic_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <fc/fixed_string.hpp>


namespace graphene {
  namespace chain {
    class database;

    using hash_index_type = fc::fixed_string_32;

    enum atomicswap_contract_type {
      atomicswap_contract_initiator = 0x0,
      atomicswap_contract_participant = 0x1
    };


    class atomicswap_contract_object : public abstract_object<atomicswap_contract_object> {

    public:
      static const uint8_t space_id = protocol_ids;
      static const uint8_t type_id = atomicswap_contract_object_type;

      uint8_t type = atomicswap_contract_initiator;

      account_id_type owner;

      account_id_type to;
      asset amount = asset(0, asset_id_type());

      string metadata;

      string secret_hash;
      string secret;
      hash_index_type contract_hash;

      time_point_sec created = time_point_sec::min();
      time_point_sec deadline = time_point_sec::min();


    };


    struct by_owner;
    struct by_recipient;
    struct by_contract_hash;

    typedef multi_index_container <atomicswap_contract_object,
    indexed_by<ordered_unique < tag < by_id>,
    member<object,
        object_id_type,
        &object::id>>,
    ordered_non_unique <tag<by_owner>,
    member<atomicswap_contract_object,
        account_id_type,
        &atomicswap_contract_object::owner>>,
    ordered_non_unique <tag<by_recipient>,
    member<atomicswap_contract_object,
        account_id_type,
        &atomicswap_contract_object::to>>,
    ordered_unique <tag<by_contract_hash>,
    member<atomicswap_contract_object,
        hash_index_type,
        &atomicswap_contract_object::contract_hash>>>>
    atomicswap_contract_index_type;

    typedef generic_index <atomicswap_contract_object, atomicswap_contract_index_type> atomicswap_contract_index;
  }


}

FC_REFLECT_ENUM(graphene::chain::atomicswap_contract_type,
                (atomicswap_contract_initiator)(atomicswap_contract_participant))

FC_REFLECT_DERIVED(graphene::chain::atomicswap_contract_object, (graphene::db::object),
                   (type) (owner)(to)(amount)(metadata)(secret_hash)(secret)(created)(deadline))
