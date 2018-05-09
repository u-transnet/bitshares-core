#pragma once
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/atomicswap_utils.hpp>

namespace graphene { namespace chain {



    struct atomicswap_initiate_operation : public base_operation
    {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };

      enum operation_type{
        by_initiator = 0x0,
        by_participant = 0x1
      };

      asset            fee;
      uint8_t type;

      account_id_type owner;
      account_id_type recipient;

      asset amount;

      string secret_hash;
      string metadata;


      void validate() const;
      account_id_type fee_payer()const { return owner; }

    };

    struct atomicswap_redeem_operation : public base_operation
    {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset            fee;

      account_id_type from;
      account_id_type to; // participant or initiator
      string secret;

      void validate() const;
      account_id_type fee_payer()const { return to; }
    };

    struct atomicswap_refund_operation : public base_operation
    {
      struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };

      asset            fee;

      account_id_type initiator;
      account_id_type participant;
      string secret_hash;

      account_id_type fee_payer()const { return initiator; }
      void validate() const;
    };

  }
}

FC_REFLECT( graphene::chain::atomicswap_initiate_operation::fee_parameters_type, (fee) )
FC_REFLECT_ENUM(graphene::chain::atomicswap_initiate_operation::operation_type,(by_initiator)(by_participant))
FC_REFLECT( graphene::chain::atomicswap_initiate_operation, (fee)(type)(owner)(recipient)(amount)(secret_hash)(metadata) )

FC_REFLECT( graphene::chain::atomicswap_redeem_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::atomicswap_redeem_operation, (fee)(from)(to)(secret) )

FC_REFLECT( graphene::chain::atomicswap_refund_operation::fee_parameters_type, (fee) )
FC_REFLECT( graphene::chain::atomicswap_refund_operation, (fee)(initiator)(participant)(secret_hash) )
