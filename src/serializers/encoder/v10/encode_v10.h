/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../../serializers_include.h"

void RdbSaveGraph_v10
(
	RedisModuleIO *rdb,
	void *value
);

void RdbSaveNodes_v10
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t nodes_to_encode
);

void RdbSaveDeletedNodes_v10
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t deleted_nodes_to_encode
);

void RdbSaveEdges_v10
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t edges_to_encode
);

void RdbSaveDeletedEdges_v10
(
	RedisModuleIO *rdb,
	GraphContext *gc,
	uint64_t deleted_edges_to_encode
);

void RdbSaveGraphSchema_v10
(
	RedisModuleIO *rdb,
	GraphContext *gc
);

