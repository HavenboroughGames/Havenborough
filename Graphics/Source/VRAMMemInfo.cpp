#include "VRAMMemInfo.h"

VRAMMemInfo *VRAMMemInfo::m_Instance = nullptr;

VRAMMemInfo *VRAMMemInfo::getInstance(void)
{
	if(!m_Instance)
		m_Instance = new VRAMMemInfo();

	return m_Instance;
}

void VRAMMemInfo::shutdown(void)
{
	SAFE_DELETE(m_Instance);
}

int VRAMMemInfo::getUsage(void) const
{
	return m_Usage / MB;
}

void VRAMMemInfo::updateUsage(int size)
{
	m_Usage += size;
}

unsigned int VRAMMemInfo::calculateFormatUsage(DXGI_FORMAT p_Format, int p_Width, int p_Height)
{
	unsigned int result;

	switch (p_Format)
	{
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		{
			result = Size::R32G32B32A32_FLOAT * p_Width * p_Height;
			break;
		}
	case DXGI_FORMAT_R32G32B32A32_UINT:
		{
			result = Size::R32G32B32A32_UINT * p_Width * p_Height;
			break;
		}
	case DXGI_FORMAT_R32G32B32_FLOAT:
		{
			result = Size::R32G32B32_FLOAT * p_Width * p_Height;
			break;
		}
	case DXGI_FORMAT_R32G32B32_UINT:
		{
			result = Size::R32G32B32_UINT * p_Width * p_Height;
			break;
		}
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		{
			result = Size::R16G16B16A16_FLOAT * p_Width * p_Height;
			break;
		}
	case DXGI_FORMAT_R32G32_FLOAT:
		{
			result = Size::R32G32_FLOAT * p_Width * p_Height;
			break;
		}
	case DXGI_FORMAT_R32G32_UINT:
		{
			result = Size::R32G32_UINT * p_Width * p_Height;
			break;
		}
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		{
			result = Size::R8G8B8A8_UNORM * p_Width * p_Height;
			break;
		}
	default:
		throw MemoryUsageException("Error when determining memory size of texture.", __LINE__, __FILE__);
		break;
	}

	return result;
}

VRAMMemInfo::VRAMMemInfo(void)
{
	m_Usage = OVERHEAD_USAGE;
}

VRAMMemInfo::~VRAMMemInfo(void)
{
}