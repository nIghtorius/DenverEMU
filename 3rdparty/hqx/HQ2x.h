/*
 * Copyright 2016 Bruno Ribeiro
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cstdint>
#include "HQx.h"


class HQ2x : public HQx
{
	public:
		HQ2x();

		~HQ2x();

		std::uint32_t *resize(
			const std::uint32_t *image,
			std::uint32_t width,
			std::uint32_t height,
			std::uint32_t *output,
			std::uint32_t trY = 0x30,
			std::uint32_t trU = 0x07,
			std::uint32_t trV = 0x06,
			std::uint32_t trA = 0x50,
			bool wrapX = false,
			bool wrapY = false ) const;
};
