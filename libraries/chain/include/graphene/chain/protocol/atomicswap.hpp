#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/atomicswap_utils.hpp>

namespace graphene { namespace chain {



    struct atomicswap_initiate_operation : public base_operation
    {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };

      enum operation_type : bool
      {
        by_initiator = 0,
        by_participant,
      };

      operation_type type = operation_type::by_initiator;

      account_id_type owner;
      account_id_type recipient;

      asset amount = asset(0, asset_id_type());

      string secret_hash;

      string metadata;

      asset            fee;

      void validate() const;
      account_id_type fee_payer()const { return owner; }
      void get_required_active_authorities(flat_set<account_id_type>& a) const
      {
        a.insert(owner);
      }

    };

    struct atomicswap_redeem_operation : public base_operation
    {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };

      account_id_type from;
      account_id_type to; // participant or initiator

      std::string secret;

      asset            fee;

      void validate() const;
      account_id_type fee_payer()const { return to; }
      void get_required_active_authorities(flat_set<account_id_type>& a) const
      {
        a.insert(to);
      }
    };

    struct atomicswap_refund_operation : public base_operation
    {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };

      account_id_type participant;
      account_id_type initiator;
      std::string secret_hash;

      asset            fee;

      account_id_type fee_payer()const { return initiator; }
      void validate() const;
      void get_required_active_authorities(flat_set<account_id_type>& a) const
      {
        a.insert(participant);
      }
    };

  }
}

FC_REFLECT( graphene::chain::atomicswap_initiate_operation, (type)(owner)(recipient)(amount)(secret_hash)(metadata)(fee) )
FC_REFLECT( graphene::chain::atomicswap_initiate_operation::fee_parameters_type, (fee) )
FC_REFLECT_ENUM(graphene::chain::atomicswap_initiate_operation::operation_type,(by_initiator)(by_participant))
FC_REFLECT( graphene::chain::atomicswap_redeem_operation, (from)(to)(secret)(fee) )
FC_REFLECT( graphene::chain::atomicswap_redeem_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::atomicswap_refund_operation, (participant)(initiator)(secret_hash)(fee) )
FC_REFLECT( graphene::chain::atomicswap_refund_operation::fee_parameters_type, (fee) )
