#include <graphene/chain/protocol/atomicswap.hpp>

namespace graphene{ namespace chain{

    void atomicswap_initiate_operation::validate() const
    {
      FC_ASSERT( owner != recipient );
      FC_ASSERT(amount > asset(0, amount.asset_id), "Amount must be positive");
      atomicswap::validate_contract_metadata(metadata);
      atomicswap::validate_secret_hash(secret_hash);
    }

    void atomicswap_redeem_operation::validate() const
    {
      atomicswap::validate_secret(secret);
    }

    void atomicswap_refund_operation::validate() const
    {
      atomicswap::validate_secret_hash(secret_hash);
    }

  }
}