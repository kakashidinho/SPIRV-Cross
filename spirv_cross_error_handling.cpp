/*
 * Copyright 2020-2020 Le Hoang Quyen
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "spirv_cross_error_handling.hpp"
#ifdef SPIRV_CROSS_EXCEPTIONS_TO_LONGJMP
#include <stack>

namespace SPIRV_CROSS_NAMESPACE
{

static thread_local std::stack<CompileErrorContext *> g_per_thread_ctxs;

static CompileErrorContext *get_error_ctx()
{
	if (g_per_thread_ctxs.empty())
		return nullptr;
	return g_per_thread_ctxs.top();
}

bool push_error_context(CompileErrorContext *context, int setjmp_return_code)
{
    if (setjmp_return_code != 0)
        return false; // this is an unwinding, return immediately

	if (!context)
		g_per_thread_ctxs.pop();
	else
        g_per_thread_ctxs.push(context);
    
    return true;
}

void pop_error_context()
{
    g_per_thread_ctxs.pop();
}

void unwind_or_abort(const std::string &msg)
{
	auto unwind_ctx = get_error_ctx();
	if (unwind_ctx)
	{
		unwind_ctx->error_msg = msg;
		std::longjmp(unwind_ctx->jmp_ctx, 1);
	}

#ifdef NDEBUG
	(void)msg;
#else
	fprintf(stderr, "There was a compiler error: %s\n", msg.c_str());
#endif
	fflush(stderr);
	abort();
}

} // SPIRV_CROSS_NAMESPACE

#endif // SPIRV_CROSS_EXCEPTIONS_TO_LONGJMP
