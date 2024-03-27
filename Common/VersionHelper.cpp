/**
 * FileName: VersionHelper.cpp
 *
 * Copyright (C) 2024 Maplespe
 *
 * This file is part of MToolBox and DWMBlurGlass.
 * DWMBlurGlass is free software: you can redistribute it and/or modify it under the terms of the
 * GNU Lesser General Public License as published by the Free Software Foundation, either version 3
 * of the License, or any later version.
 *
 * DWMBlurGlass is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with Foobar.
 * If not, see <https://www.gnu.org/licenses/lgpl-3.0.html>.
*/
#include "VersionHelper.h"

namespace MDWMBlurGlass
{
	namespace Utils
	{
		PKUSER_SHARED_DATA get_kernel_shared_info()
		{
			return reinterpret_cast<PKUSER_SHARED_DATA>(0x7FFE0000);
		}
	}
	namespace os
	{
		const ULONG buildNumber = Utils::get_kernel_shared_info()->NtBuildNumber;
		const ULONG minorVersion = Utils::get_kernel_shared_info()->NtMinorVersion;
		const ULONG majorVersion = Utils::get_kernel_shared_info()->NtMajorVersion;
		const NT_PRODUCT_TYPE productType = Utils::get_kernel_shared_info()->NtProductType;
	}
}