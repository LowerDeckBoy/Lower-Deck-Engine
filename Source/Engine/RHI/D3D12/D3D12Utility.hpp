#pragma once

#include <AgilitySDK/d3d12.h>
#include <cassert>
#include <stdexcept>

#include <Core/RefPtr.hpp>
#include <Core/String.hpp>

namespace lde::RHI
{

#define LDE_ASSERT(Condition) assert(Condition)

// Releasing either ComPtr or (custom) Ref pointer.
#define SAFE_RELEASE(_Ref) if (_Ref.Get()) { _Ref.Reset(); _Ref = nullptr; }

#define SAFE_DELETE(_Ptr) if (_Ptr) { _Ptr->Release(); _Ptr = nullptr; }

#define DX_CALL(hResult, ...) VerifyResult(hResult, __FILE__, __LINE__, __VA_ARGS__)

	// TODO: Add checking for OUT_OF_MEMORY
	extern void VerifyResult(HRESULT hResult, const char* File, int Line, std::string_view Message = "");
	
// Sets a name for given ID3D12 interface; casts given name into wide-string.
// Decapsulates Ref<T> pointers.
#define SET_D3D12_NAME(Interface, ...) SetD3D12Name(Interface.Get(), std::string(__VA_ARGS__))

	extern void SetD3D12Name(ID3D12Object* pDxObject, std::string Name);
	
	class D3D12Utility
	{
	public:
		inline static D3D12_HEAP_PROPERTIES HeapDefault	= D3D12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		inline static D3D12_HEAP_PROPERTIES HeapUpload	= D3D12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	};
} // namespace lde::RHI
