#include "SceneLighting.hpp"
#include <RHI/D3D12/D3D12RHI.hpp>

namespace lde
{
    SceneLighting::SceneLighting(RHI::D3D12RHI* pGfx, World* pWorld)
        : m_Gfx(pGfx), m_World(pWorld)
    {
        Create();
        //m_DirLightBuffer = 
    }

    SceneLighting::~SceneLighting()
    {
        Release();
    }

    void SceneLighting::Create()
    {
        Entity::Create(m_World);
        m_DirLightBuffer = m_Gfx->GetDevice()->CreateConstantBuffer(&m_DirLightData, sizeof(m_DirLightData));
        
    }

    void SceneLighting::Release()
    {
        m_DirLightBuffer->Release();
    }

    void SceneLighting::AddDirectionalLight()
    {
        AddComponent<DirectionalLightComponent>();
        ++m_DirLightsCount;
    }

    void SceneLighting::AddPointLight()
    {
        AddComponent<PointLightComponent>();
    }

    void SceneLighting::AddSpotLight()
    {
        AddComponent<SpotLightComponent>();
    }

} // namespace lde
