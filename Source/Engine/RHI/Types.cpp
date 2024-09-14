#include "Types.hpp"

namespace lde
{
    std::wstring ShaderEnumToType(ShaderStage eStage)
    {
        switch (eStage)
        {
        case lde::ShaderStage::eVertex:
            return L"vs_6_6";
        case lde::ShaderStage::ePixel:
            return L"ps_6_6";
        case lde::ShaderStage::eCompute:
            return L"cs_6_6";
        case lde::ShaderStage::eGeometry:
            return L"gs_6_6";
        case lde::ShaderStage::eAmplification:
            return L"as_6_6";
        case lde::ShaderStage::eMesh:
            return L"ms_6_6";
        case lde::ShaderStage::eRaytracing:
            [[fallthrough]];
        case lde::ShaderStage::eClosestHit:
            [[fallthrough]];
        case lde::ShaderStage::eMiss:
            return L"lib_6_6";
        case lde::ShaderStage::eUnspecified:
            [[fallthrough]];
        default:
            return L"Invalid";
        }
    }
    
} // namespace lde
