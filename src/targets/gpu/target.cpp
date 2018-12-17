#include <migraphx/gpu/target.hpp>
#include <migraphx/gpu/lowering.hpp>
#include <migraphx/memory_coloring.hpp>
#include <migraphx/gpu/write_literals.hpp>
#include <migraphx/gpu/context.hpp>
#include <migraphx/gpu/eliminate_workspace.hpp>
#include <migraphx/eliminate_allocation.hpp>
#include <migraphx/gpu/fuse_ops.hpp>
#include <migraphx/check_context.hpp>
#include <migraphx/auto_contiguous.hpp>
#include <migraphx/dead_code_elimination.hpp>
#include <migraphx/simplify_reshapes.hpp>
#include <migraphx/simplify_algebra.hpp>
#include <migraphx/constant_propagate.hpp>
#include <migraphx/eliminate_contiguous.hpp>
#include <migraphx/common_subexpression_elimination.hpp>
#include <migraphx/fwd_conv_batchnorm_rewrite.hpp>
#include <migraphx/eliminate_concat.hpp>
#include <migraphx/gpu/concat_gpu_opt.hpp>
#include <migraphx/pre_scheduling.hpp>
#include <migraph/gpu/machine_model.hpp>

namespace migraphx {
inline namespace MIGRAPH_INLINE_NS {
namespace gpu {

std::vector<pass> target::get_passes(migraphx::context& gctx) const
{
    auto& ctx = any_cast<context>(gctx);
    std::function<std::pair<int,int>(std::string&)> weight_func = op_info();
    int num_of_streams = stream_info().num_of_streams();
    // clang-format off
    return
    {
        dead_code_elimination{},
        fwd_conv_batchnorm_rewrite{},
        dead_code_elimination{},
        common_subexpression_elimination{},
        dead_code_elimination{},
        simplify_algebra{},
        dead_code_elimination{},
        constant_propagate{},
        dead_code_elimination{},
        auto_contiguous{},
        simplify_reshapes{},
        dead_code_elimination{},
        pre_scheduling{weight_func, num_of_streams},                        
        lowering{ctx},
        eliminate_concat{concat_gpu_optimization{}},
        dead_code_elimination{},
        eliminate_contiguous{},
        dead_code_elimination{},
            //            fuse_ops{&ctx},
        dead_code_elimination{},
        write_literals{&ctx},
        memory_coloring{"hip::allocate", num_of_streams},
        eliminate_workspace{},
        eliminate_allocation{"hip::allocate"},
        check_context<context>{},
        dead_code_elimination{}
    };
    // clang-format on
}

std::string target::name() const { return "miopen"; }

migraphx::context target::get_context() const { return context{}; }
} // namespace gpu
} // namespace MIGRAPH_INLINE_NS
} // namespace migraphx
