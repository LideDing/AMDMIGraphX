#ifndef MIGRAPH_GUARD_RTGLIB_VERIFY_ARGS_HPP
#define MIGRAPH_GUARD_RTGLIB_VERIFY_ARGS_HPP

#include <migraph/verify.hpp>
#include <migraph/argument.hpp>

namespace migraph {

inline void verify_args(const std::string& name,
                        const argument& cpu_arg,
                        const argument& gpu_arg,
                        double tolerance = 80)
{
    visit_all(cpu_arg, gpu_arg)([&](auto cpu, auto gpu) {
        double error;
        if(not verify_range(cpu, gpu, tolerance, &error))
        {
            // TODO: Check for nans
            std::cout << "FAILED: " << name << std::endl;
            std::cout << "error: " << error << std::endl;
            if(cpu.size() < 32)
                std::cout << "cpu:" << cpu << std::endl;
            if(gpu.size() < 32)
                std::cout << "gpu:" << gpu << std::endl;
            if(range_zero(cpu))
                std::cout << "Cpu data is all zeros" << std::endl;
            if(range_zero(gpu))
                std::cout << "Gpu data is all zeros" << std::endl;

            auto idx = mismatch_idx(cpu, gpu, float_equal);
            if(idx < range_distance(cpu))
            {
                std::cout << "Mismatch at " << idx << ": " << cpu[idx] << " != " << gpu[idx]
                          << std::endl;
            }

            auto cpu_nan_idx = find_idx(cpu, not_finite);
            if(cpu_nan_idx >= 0)
                std::cout << "Non finite number found in cpu at " << cpu_nan_idx << ": "
                          << cpu[cpu_nan_idx] << std::endl;

            auto gpu_nan_idx = find_idx(gpu, not_finite);
            if(gpu_nan_idx >= 0)
                std::cout << "Non finite number found in gpu at " << gpu_nan_idx << ": "
                          << gpu[gpu_nan_idx] << std::endl;
            std::cout << std::endl;
        }
    });
}

} // namespace migraph

#endif
