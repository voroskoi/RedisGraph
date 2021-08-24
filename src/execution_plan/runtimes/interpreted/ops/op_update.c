/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_update.h"
#include "RG.h"
#include "../../../../errors.h"
#include "../../../../query_ctx.h"
#include "../../../../util/arr.h"
#include "../../../../util/qsort.h"
#include "../../../../util/rmalloc.h"
#include "../../../../util/rax_extensions.h"
#include "../../../../arithmetic/arithmetic_expression.h"

/* Forward declarations. */
static RT_OpResult UpdateInit(RT_OpBase *opBase);
static Record UpdateConsume(RT_OpBase *opBase);
static RT_OpResult UpdateReset(RT_OpBase *opBase);
static RT_OpBase *UpdateClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase);
static void UpdateFree(RT_OpBase *opBase);

static Record _handoff(RT_OpUpdate *op) {
	/* TODO: poping a record out of op->records
	 * will reverse the order in which records
	 * are passed down the execution plan. */
	if(op->records && array_len(op->records) > 0) return array_pop(op->records);
	return NULL;
}

RT_OpBase *RT_NewUpdateOp(const RT_ExecutionPlan *plan, rax *update_exps) {
	RT_OpUpdate *op = rm_calloc(1, sizeof(RT_OpUpdate));
	op->records            =  NULL;
	op->updates            =  NULL;
	op->updates_committed  =  false;
	op->update_ctxs        =  update_exps;
	op->gc                 =  QueryCtx_GetGraphCtx();

	// set our op operations
	RT_OpBase_Init((RT_OpBase *)op, OPType_UPDATE, UpdateInit, UpdateConsume,
				UpdateReset, UpdateClone, UpdateFree, true, plan);

	// iterate over all update expressions
	// set the record index for every entity modified by this operation
	raxStart(&op->it, update_exps);
	raxSeek(&op->it, "^", NULL, 0);
	while(raxNext(&op->it)) {
		EntityUpdateEvalCtx *ctx = op->it.data;
		ctx->record_idx = OpBase_Modifies((OpBase *)op, ctx->alias);
	}

	return (RT_OpBase *)op;
}

static RT_OpResult UpdateInit(RT_OpBase *opBase) {
	RT_OpUpdate *op = (RT_OpUpdate*)opBase;

	op->stats    =    QueryCtx_GetResultSetStatistics();
	op->records  =    array_new(Record, 64);
	op->updates  =    array_new(PendingUpdateCtx, raxSize(op->update_ctxs));

	return OP_OK;
}

static Record UpdateConsume(RT_OpBase *opBase) {
	RT_OpUpdate *op = (RT_OpUpdate *)opBase;
	RT_OpBase *child = op->op.children[0];
	Record r;

	// updates already performed
	if(op->updates_committed) return _handoff(op);

	while((r = RT_OpBase_Consume(child))) {
		Record_PersistScalars(r);

		// evaluate update expressions
		raxSeek(&op->it, "^", NULL, 0);
		while(raxNext(&op->it)) {
			EntityUpdateEvalCtx *ctx = op->it.data;
			EvalEntityUpdates(op->gc, &op->updates, r, ctx, true);
		}

		array_append(op->records, r);
	}

	// done reading; we're not going to call Consume any longer
	// there might be operations like "Index Scan" that need to free the
	// index R/W lock - as such, free all ExecutionPlan operations up the chain.
	RT_OpBase_PropagateFree(child);

	// lock everything
	QueryCtx_LockForCommit();
	{
		CommitUpdates(op->gc, op->stats, op->updates);
	}
	// release lock
	QueryCtx_UnlockCommit(opBase);

	op->updates_committed = true;

	return _handoff(op);
}

static RT_OpBase *UpdateClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase) {
	ASSERT(opBase->type == OPType_UPDATE);
	RT_OpUpdate *op = (RT_OpUpdate *)opBase;

	rax *update_ctxs = raxCloneWithCallback(op->update_ctxs, (void *(*)(void *))UpdateCtx_Clone);
	return RT_NewUpdateOp(plan, update_ctxs);
}

static RT_OpResult UpdateReset(RT_OpBase *ctx) {
	RT_OpUpdate *op = (RT_OpUpdate *)ctx;
	array_free(op->updates);
	op->updates = NULL;
	op->updates_committed = false;
	return OP_OK;
}

static void UpdateFree(RT_OpBase *ctx) {
	RT_OpUpdate *op = (RT_OpUpdate *)ctx;

	if(op->updates) {
		array_free(op->updates);
		op->updates = NULL;
	}

	// Free each update context.
	if(op->update_ctxs) {
		raxFreeWithCallback(op->update_ctxs, (void(*)(void *))UpdateCtx_Free);
		op->update_ctxs = NULL;
	}

	if(op->records) {
		uint records_count = array_len(op->records);
		for(uint i = 0; i < records_count; i++) RT_OpBase_DeleteRecord(op->records[i]);
		array_free(op->records);
		op->records = NULL;
	}

	raxStop(&op->it);
}
