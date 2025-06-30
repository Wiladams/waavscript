#pragma once

namespace waavs {
	struct PSVMOps {

		// Calculate the maximum between two values
		const char* op_code_max = R"||(
/.max {2 copy lt { exch } if pop } bind def
)||";

		const char* op_code_min = R"||(
/.min {2 copy gt { exch } if pop } bind def
)||";

	};

}