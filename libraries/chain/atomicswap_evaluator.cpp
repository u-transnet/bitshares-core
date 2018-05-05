
#include "graphene/chain/atomicswap_evaluator.hpp"
#include <graphene/chain/is_authorized_asset.hpp>
#include <graphene/chain/atomicswap_object.hpp>

namespace graphene { namespace chain{

    void_result atomicswap_initiate_evaluator::do_evaluate( const atomicswap_initiate_operation& op )
    {
      try {

        const database& d = db();

        const account_object& owner_account    = op.owner(d);
        const account_object& recipient_account      = op.recipient(d);
        const asset_object&   asset_type      = op.amount.asset_id(d);

        GRAPHENE_ASSERT(
            is_authorized_asset( d, owner_account, asset_type ),
            transfer_from_account_not_whitelisted,
            "'from' account ${from} is not whitelisted for asset ${asset}",
            ("from",op.owner)
                ("asset",op.amount.asset_id)
        );
        GRAPHENE_ASSERT(
            is_authorized_asset( d, recipient_account, asset_type ),
            transfer_to_account_not_whitelisted,
            "'to' account ${to} is not whitelisted for asset ${asset}",
            ("to",op.recipient)
                ("asset",op.amount.asset_id)
        );

        if( asset_type.is_transfer_restricted() )
        {
          GRAPHENE_ASSERT(
              owner_account.id == asset_type.issuer || recipient_account.id == asset_type.issuer,
              transfer_restricted_transfer_asset,
              "Asset {asset} has transfer_restricted flag enabled",
              ("asset", op.amount.asset_id)
          );
        }

        bool insufficient_balance = d.get_balance( owner_account, asset_type ).amount >= op.amount.amount;
        FC_ASSERT( insufficient_balance,
                   "Insufficient Balance: ${balance}, unable to reserve for atomic swap '${total_transfer}' from account '${a}' to '${t}'",
                   ("a",owner_account.name)("t",recipient_account.name)("total_transfer",d.to_pretty_string(op.amount))("balance",d.to_pretty_string(d.get_balance(owner_account, asset_type))) );


        FC_ASSERT(!op.secret_hash.empty(), "Empty secret hash.");

        const atomicswap::hash_index_type contract_hash = atomicswap::get_contract_hash(owner_account, recipient_account, op.secret_hash);

        auto& atc_index = d.get_index_type<atomicswap_contract_index>();

        auto current_contract_itr = atc_index.indices().get<by_contract_hash>().find(contract_hash);
        FC_ASSERT( current_contract_itr == atc_index.indices().get<by_contract_hash>().end(), "Contract with same hash already exists");

        FC_ASSERT(op.type != op.by_initiator
                  && op.type != op.by_participant, "Invalid operation type.");

        return void_result();
      }FC_CAPTURE_AND_RETHROW( (op) );
    }

    void_result atomicswap_initiate_evaluator::do_apply(const atomicswap_initiate_operation& op)
    {
      try {

        database& d = db();

        const account_object &owner_account = op.owner(d);
        const account_object &recipient_account = op.recipient(d);
        const asset_object&   asset_type      = op.amount.asset_id(d);
        const atomicswap::hash_index_type contract_hash = atomicswap::get_contract_hash(owner_account, recipient_account, op.secret_hash);

        optional <std::string> metadata;
        if (!op.metadata.empty()) {
          metadata = op.metadata;
        }

        time_point_sec start = d.head_block_time();
        time_point_sec deadline = start;
        deadline += (op.type == op.by_initiator)
                    ?fc::seconds(GRAPHENE_ATOMICSWAP_INITIATOR_REFUND_LOCK_SECS)
                    :fc::seconds(GRAPHENE_ATOMICSWAP_PARTICIPANT_REFUND_LOCK_SECS);


        atomicswap_contract_object contract = d.create<atomicswap_contract_object>(
            [&](atomicswap_contract_object& contract){
              contract.type = op.type == op.by_initiator
                              ?atomicswap_contract_type::atomicswap_contract_initiator
                              :atomicswap_contract_type::atomicswap_contract_participant;
              contract.owner = owner_account.id;
              contract.to = recipient_account.id;
              contract.amount = op.amount;
              contract.created = start;
              contract.deadline = deadline;
              contract.secret_hash.assign(op.secret_hash.begin(), op.secret_hash.end());
              contract.contract_hash = contract_hash;
              if (metadata.valid())
              {
                string value = *metadata;
                contract.metadata.assign(value.begin(), value.end());
              }
            }
        );

        db().adjust_balance( owner_account.id, -op.amount );

        return void_result();
      }FC_CAPTURE_AND_RETHROW( (op) );
    }

    void_result atomicswap_redeem_evaluator::do_evaluate(const atomicswap_redeem_operation &op)
    {
      try {

        const database &d = db();

        const account_object &owner_account = op.from(d);
        const account_object &recipient_account = op.to(d);

        std::string secret_hash = atomicswap::get_secret_hash(op.secret);
        const atomicswap::hash_index_type contract_hash = atomicswap::get_contract_hash(owner_account, recipient_account, secret_hash);

        auto& atc_index = d.get_index_type<atomicswap_contract_index>();

        auto current_contract_itr = atc_index.indices().get<by_contract_hash>().find(contract_hash);
        FC_ASSERT( current_contract_itr != atc_index.indices().get<by_contract_hash>().end(),
                   "Contract for this secret does not exists");

        return void_result();
      }FC_CAPTURE_AND_RETHROW( (op) );
    }

    void_result atomicswap_redeem_evaluator::do_apply(const atomicswap_redeem_operation& op)
    {
      try {

        const database &d = db();

        const account_object &owner_account = op.from(d);
        const account_object &recipient_account = op.to(d);

        std::string secret_hash = atomicswap::get_secret_hash(op.secret);
        const atomicswap::hash_index_type contract_hash = atomicswap::get_contract_hash(owner_account, recipient_account, secret_hash);

        auto& atc_index = d.get_index_type<atomicswap_contract_index>();
        const atomicswap_contract_object& contract = *atc_index.indices().get<by_contract_hash>().find(contract_hash);

        db().adjust_balance(recipient_account.id, contract.amount);

        if (contract.type == atomicswap_contract_initiator)
        {
          db().remove(contract);
        }
        else
        {
          // save secret for participant
          db().modify<atomicswap_contract_object>(contract, [&](atomicswap_contract_object& contract){
            contract.secret.assign(op.secret.begin(), op.secret.end());
            contract.amount.amount = 0; // asset moved to 'to' balance
          });
        }

        return void_result();
      }FC_CAPTURE_AND_RETHROW( (op) );
    }


    void_result atomicswap_refund_evaluator::do_evaluate(const atomicswap_refund_operation &op)
    {
      try {

        const database &d = db();

        const account_object &owner_account = op.initiator(d);
        const account_object &recipient_account = op.participant(d);
        const std::string secret_hash = op.secret_hash;
        const atomicswap::hash_index_type contract_hash = atomicswap::get_contract_hash(owner_account, recipient_account, secret_hash);

        auto& atc_index = d.get_index_type<atomicswap_contract_index>();
        auto current_contract_itr = atc_index.indices().get<by_contract_hash>().find(contract_hash);
        FC_ASSERT( current_contract_itr != atc_index.indices().get<by_contract_hash>().end(),
                   "Contract for this secret does not exists");



        FC_ASSERT(d.head_block_time() > current_contract_itr->deadline,
                  "Can't refund contract. It is locked on ${h} hours.",
                  ("h", (current_contract_itr->deadline - current_contract_itr->created).to_seconds() / 3600));


        return void_result();
      }FC_CAPTURE_AND_RETHROW( (op) );
    }

    void_result atomicswap_refund_evaluator::do_apply(const atomicswap_refund_operation& op)
    {
      try {

        const database &d = db();

        const account_object &owner_account = op.initiator(d);
        const account_object &recipient_account = op.participant(d);
        const std::string secret_hash = op.secret_hash;
        const atomicswap::hash_index_type contract_hash = atomicswap::get_contract_hash(owner_account, recipient_account, secret_hash);

        auto& atc_index = d.get_index_type<atomicswap_contract_index>();
        const atomicswap_contract_object& contract = *atc_index.indices().get<by_contract_hash>().find(contract_hash);

        db().adjust_balance(owner_account.id, contract.amount);
        db().remove(contract);

        return void_result();
      }FC_CAPTURE_AND_RETHROW( (op) );
    }


  }
}