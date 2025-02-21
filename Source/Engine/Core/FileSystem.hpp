#pragma once
#include <filesystem>

using Filepath = std::filesystem::path;

namespace lde::Files
{
	inline bool Exists(std::string_view Filename)
	{
		return std::filesystem::exists(Filename);
	}

	inline std::string GetFileName(std::string_view Filepath)
	{
		return std::filesystem::path(Filepath).filename().string();
	}

	inline std::string GetExtension(Filepath File)
	{
		return File.extension().string();
	}

	inline std::string GetFileExtension(std::string_view Filename)
	{
		return std::filesystem::path(Filename).extension().string();
	}

	inline std::string GetRelativePath(std::string_view Filename)
	{
		return std::filesystem::path(Filename).relative_path().string();
	}

	inline std::string GetParentPath(std::string_view Filepath)
	{
		return std::filesystem::path(Filepath).parent_path().string();
	}

	inline std::string GetTexturePath(const std::string& Filename, const std::string& TextureName)
	{
		return std::filesystem::path(Filename).relative_path().parent_path().string() + '/' + TextureName;
	}

	enum class ImageExtension
	{
		eJPG = 0x00,
		eJPEG,
		ePNG,
		eTGA,
		eBMP,
		eDDS,
		eHDR,
		eInvalid
	};

	ImageExtension ImageExtToEnum(std::string_view Filepath);

} // namespace lde::Files
