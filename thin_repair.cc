#include <iostream>

#include "metadata.h"

using namespace persistent_data;
using namespace std;
using namespace thin_provisioning;

namespace {
	void check(string const &path) {
		metadata md(path);

		md.check();
	}

	void usage(string const &cmd) {
		cerr << "Usage: " << cmd << " <metadata device>" << endl;
	}
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		usage(argv[0]);
		exit(1);
	}

	check(argv[1]);

	return 0;
}