#pragma once
#include <cstddef>

// All offsets were taken from the Evelion source code
// https://github.com/3a1/Evelion

namespace Offsets
{
	const size_t view = 0xEC9780;
	const size_t ent_list = 0x120461C;
	const size_t ent_stride = 0x250;

	const size_t pos_x = 0x0184;
	const size_t pos_y = 0x0188;
	const size_t pos_z = 0x018C;

	const size_t name = 0x0100;
	const size_t model = 0x012C;
	const size_t state = 0x017D;

	const size_t local_team = 0x100DF4;
	const size_t lobby = 0x105CFC8;
}
	