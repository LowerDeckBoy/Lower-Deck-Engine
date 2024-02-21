#include "FileSystem.hpp"

namespace mf::files
{
    ImageExtension ImageExtToEnum(std::string_view Filepath)
    {
		auto extension = GetFileExtension(Filepath);

		if (extension == ".jpg")
			return ImageExtension::eJPG;
		else if (extension == ".jpeg")
			return ImageExtension::eJPEG;
		else if (extension == ".png")
			return ImageExtension::ePNG;
		else if (extension == ".tga")
			return ImageExtension::eTGA;
		else if (extension == ".bmp")
			return ImageExtension::eBMP;
		else if (extension == ".dds")
			return ImageExtension::eDDS;
		else if (extension == ".hdr")
			return ImageExtension::eHDR;
		
		return ImageExtension::eInvalid;
    }
} // namespace mf::files
