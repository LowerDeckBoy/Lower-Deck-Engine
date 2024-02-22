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
        case lde::RHI::ShaderStage::eAmplification:
            return L"as_6_6";
        case lde::RHI::ShaderStage::eMesh:
            return L"ms_6_6";
        case lde::RHI::ShaderStage::eRaytracing:
            return L"lib_6_6";
        case lde::RHI::ShaderStage::eUnspecified:
            return L"Invalid";
        }

        return L"Invalid";
    }

}
