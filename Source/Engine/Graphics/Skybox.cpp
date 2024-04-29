#include "Scene/SceneCamera.hpp"
#include "Skybox.hpp"
#include <array>
#include <RHI/D3D12/D3D12RHI.hpp>
#include <Core/CoreTypes.hpp>
#include <Graphics/TextureManager.hpp>
#include <Scene/Components/Components.hpp>

namespace lde
{
    Skybox::~Skybox()
    {
        delete BRDFTexture;
        delete SpecularTexture;
        delete DiffuseTexture;
        delete TextureCube;
        delete Texture;
    }

    void Skybox::Create(RHI::RHI* pRHI, World* pWorld, std::string_view Filepath)
    {
        m_Device = (RHI::D3D12Device*)pRHI->GetDevice();

        Entity::Create(pWorld);
        AddComponent<TransformComponent>();
     
        std::array<uint32, 36> indices =
        {
             0, 1, 2, 2, 3, 0,   // Front
             1, 4, 7, 7, 2, 1,   // Right
             4, 5, 6, 6, 7, 4,   // Back
             5, 0, 3, 3, 6, 5,   // Left
             5, 4, 1, 1, 0, 5,   // Top
             3, 2, 7, 7, 6, 3    // Bottom
        };


        m_IndexBuffer = pRHI->GetDevice()->CreateBuffer(
            RHI::BufferDesc{
                .eType = RHI::BufferUsage::eIndex,
                .pData = indices.data(),
                .Count = static_cast<uint32>(indices.size()),
                .Size = indices.size() * sizeof(indices.at(0)),
                .Stride = sizeof(indices.at(0))
            }
        );

        m_ConstBuffer = pRHI->GetDevice()->CreateConstantBuffer(&m_cbPerObject, sizeof(m_cbPerObject));

    }

    void Skybox::Draw(int32 TextureID, SceneCamera* pCamera)
    {
        auto* commandList = m_Device->GetGfxCommandList();
        auto* indexBuffer = m_Device->Buffers.at(m_IndexBuffer);
        auto* constBuffer = m_Device->ConstantBuffers.at(m_ConstBuffer);

        commandList->Get()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList->BindIndexBuffer(indexBuffer);

        auto& transforms = Entity::GetComponent<TransformComponent>();
        transforms.Scale = XMFLOAT3(50.0f, 50.0f, 50.0f);

        // Set Skybox position to current Camera position
        XMStoreFloat3(&transforms.Translation, pCamera->GetPosition());
        transforms.Update();

        // Bind constant buffers
        m_cbPerObject.WVP = XMMatrixTranspose(transforms.WorldMatrix * pCamera->GetViewProjection());
        m_cbPerObject.World = XMMatrixTranspose(transforms.WorldMatrix);
        constBuffer->Update(&m_cbPerObject);
        commandList->BindConstantBuffer(0, constBuffer);

        struct indices { uint32 index; } textures{ TextureCube->SRV.Index() };
        commandList->PushConstants(1, 1, &textures);
  
        commandList->DrawIndexed(indexBuffer->GetDesc().Count, 0, 0);
        
    }
} // namespace lde
