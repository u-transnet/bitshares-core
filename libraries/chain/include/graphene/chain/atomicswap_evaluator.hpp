#pragma once
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/protocol/atomicswap.hpp>
#include <graphene/chain/protocol/operations.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {

class atomicswap_initiate_evaluator : public evaluator<atomicswap_initiate_evaluator>
{
public:
    typedef atomicswap_initiate_operation operation_type;

    void_result do_evaluate(const operation_type &o);
    void_result do_apply(const operation_type &o);

};

class atomicswap_redeem_evaluator : public evaluator<atomicswap_redeem_evaluator>
{
public:
  typedef atomicswap_redeem_operation operation_type;

  void_result do_evaluate(const operation_type &o);
  void_result do_apply(const operation_type &o);
};


class atomicswap_refund_evaluator : public evaluator<atomicswap_refund_evaluator>
{
public:
  typedef atomicswap_refund_operation operation_type;

  void_result do_evaluate(const operation_type &o);
  void_result do_apply(const operation_type &o);
};


  }
}