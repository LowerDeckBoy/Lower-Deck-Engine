#include "Types.hpp"

namespace lde::RHI
{
    std::wstring ShaderEnumToType(ShaderStage eStage)
    {
        switch (eStage)
        {
        case lde::RHI::ShaderStage::eVertex:
            return L"vs_6_6";
        case lde::RHI::ShaderStage::ePixel:
            return L"ps_6_6";
        case lde::RHI::ShaderStage::eCompute:
            return L"cs_6_6";
        case lde::RHI::ShaderStage::eGeometry:
            return L"gs_6_6";
        case lde::RHI::ShaderStage::eAmplification:
            return L"as_6_6";
        case lde::RHI::ShaderStage::eMesh:
            return L"ms_6_6";
        case lde::RHI::ShaderStage::eRaytracing:
            [[fallthrough]];
        case lde::RHI::ShaderStage::eClosestHit:
            [[fallthrough]];
        case lde::RHI::ShaderStage::eMiss:
            return L"lib_6_6";
        case lde::RHI::ShaderStage::eHull:
            return L"hs_6_6";
        case lde::RHI::ShaderStage::eTessellation:
            return L"ts_6_6";
        case lde::RHI::ShaderStage::eDomain:
            return L"ds_6_6";
        case lde::RHI::ShaderStage::eUnspecified:
            return L"Invalid";
        }

        return L"Invalid";
    }

}
