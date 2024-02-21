#include "Types.hpp"

namespace mf::RHI
{
    std::wstring ShaderEnumToType(ShaderStage eStage)
    {
        switch (eStage)
        {
        case mf::RHI::ShaderStage::eVertex:
            return L"vs_6_6";
        case mf::RHI::ShaderStage::ePixel:
            return L"ps_6_6";
        case mf::RHI::ShaderStage::eCompute:
            return L"cs_6_6";
        case mf::RHI::ShaderStage::eAmplification:
            return L"as_6_6";
        case mf::RHI::ShaderStage::eMesh:
            return L"ms_6_6";
        case mf::RHI::ShaderStage::eRaytracing:
            return L"lib_6_6";
        case mf::RHI::ShaderStage::eUnspecified:
            return L"Invalid";
        }

        return L"Invalid";
    }

}
