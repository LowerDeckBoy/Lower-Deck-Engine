#include "RHI/D3D12/D3D12Buffer.hpp"
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
        m_VertexBuffer->Release();
        m_IndexBuffer->Release();
        m_ConstBuffer->Release();
    }

    void Skybox::Create(RHI::RHI* pRHI, World* pWorld, std::string_view Filepath)
    {
        m_Device = (RHI::D3D12Device*)pRHI->GetDevice();

        Entity::Create(pWorld);
        AddComponent<TransformComponent>();
        
        std::array<DirectX::XMFLOAT3, 8> vertices =
        {
            DirectX::XMFLOAT3(-1.0f, +1.0f, +1.0f),
            DirectX::XMFLOAT3(+1.0f, +1.0f, +1.0f),
            DirectX::XMFLOAT3(+1.0f, -1.0f, +1.0f),
            DirectX::XMFLOAT3(-1.0f, -1.0f, +1.0f),
            DirectX::XMFLOAT3(+1.0f, +1.0f, -1.0f),
            DirectX::XMFLOAT3(-1.0f, +1.0f, -1.0f),
            DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f),
            DirectX::XMFLOAT3(+1.0f, -1.0f, -1.0f),
        };

        std::array<uint16, 36> indices =
        {
             0, 1, 2, 2, 3, 0,   // Front
             1, 4, 7, 7, 2, 1,   // Right
             4, 5, 6, 6, 7, 4,   // Back
             5, 0, 3, 3, 6, 5,   // Left
             5, 4, 1, 1, 0, 5,   // Top
             3, 2, 7, 7, 6, 3    // Bottom
        };

        m_VertexBuffer = pRHI->GetDevice()->CreateBuffer(
            RHI::BufferDesc{
                .eType = RHI::BufferUsage::eStructured,
                .pData = vertices.data(),
                .Count = static_cast<uint32>(vertices.size()),
                .Size = vertices.size() * sizeof(vertices.at(0)),
                .Stride = sizeof(vertices.at(0)),
                .bBindless = true
            }
        );

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

        //m_TextureIndex = TextureManager::GetInstance().Create((RHI::D3D12RHI*)pRHI, Filepath, false);

        // check for texture extension
        // if is .hdr - transform from equi to cube
    }

    void Skybox::Draw(int32 TextureID, SceneCamera* pCamera)
    {
        auto* commandList = m_Device->GetGfxCommandList();
        commandList->BindIndexBuffer(m_IndexBuffer);

        auto& transforms = Entity::GetComponent<TransformComponent>();
        // Set Skybox position to current Camera position
        XMStoreFloat3(&transforms.Translation, pCamera->GetPosition());
        transforms.Update();

        // Bind constant buffers
        m_cbPerObject.WVP = XMMatrixTranspose(transforms.WorldMatrix * pCamera->GetViewProjection());
        m_cbPerObject.World = XMMatrixTranspose(transforms.WorldMatrix);
        m_ConstBuffer->Update(&m_cbPerObject);
        commandList->BindConstantBuffer(0, m_ConstBuffer);
        struct indices { int32 index; } textures{ m_TextureIndex };
        commandList->PushConstants(1, 1, &textures);

        commandList->DrawIndexed(m_IndexBuffer->GetDesc().Count, 0, 0);

    }
} // namespace lde
