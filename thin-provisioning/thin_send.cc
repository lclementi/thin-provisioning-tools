// Copyright (C) 2011 Red Hat, Inc. All rights reserved.
//
// This file is part of the thin-provisioning-tools source.
//
// thin-provisioning-tools is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// thin-provisioning-tools is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with thin-provisioning-tools.  If not, see
// <http://www.gnu.org/licenses/>.

#include <fstream>
#include <iostream>
#include <getopt.h>
#include <libgen.h>

#include "human_readable_format.h"
#include "metadata_dumper.h"
#include "metadata.h"
#include "version.h"
#include "thin-provisioning/commands.h"
#include "send_format.h"
#include "xml_format.h"

using namespace persistent_data;
using namespace std;
using namespace thin_provisioning;

struct flags {
	uint32_t  devid;
};

namespace {
	int send_(string const &path, ostream &out, struct flags &flags){
		emitter::ptr e;
		try {

			metadata::ptr md(new metadata(path, 0));

			e = create_xml_emitter(out);

			metadata_dump(md, e, false);//flags.repair);

		} catch (std::exception &e) {
			cerr << "bb" << e.what() << endl;
			return 1;
		}
		// maker end of xml section
		out.put('\0');

		if (flags.devid) 
			out << "" << flags.devid;
		// maker end of devid section
		out.put('\0');
		try {
			metadata::ptr md1(new metadata(path, 0));
			e = create_send_emitter(out, flags.devid);
			metadata_dump(md1, e, false);//flags.repair);
		} catch (std::exception &e) {
                        cerr << "aaaa" << e.what() << endl;
                        return 1;
		}
		return 0;
	}

	int send(string const &path, char const *output, struct flags &flags){
		if (output) {
			ofstream out(output);
			return send_(path, out, flags);
		} else
			return send_(path, cout, flags);
	}

	void usage(ostream &out, string const &cmd) {
		out << "Usage: " << cmd << " [options] {device|file}" << endl
		    << "Options:" << endl
		    << "  {-h|--help}" << endl
		    << "  {-d|--devid} [block#]" << endl
		    << "  {-o <output file>}" << endl
		    << "  {-V|--version}" << endl;
	}
}

int thin_send_main(int argc, char **argv)
{
	int c;
	char const *output = NULL;
	const char shortopts[] = "hd:o:V";
	char *end_ptr;
	struct flags flags;

	const struct option longopts[] = {
		{ "help", no_argument, NULL, 'h'},
		{ "devid", optional_argument, NULL, 'd' },
		{ "output", required_argument, NULL, 'o'},
		{ "version", no_argument, NULL, 'V'},
		{ NULL, no_argument, NULL, 0 }
	};

	while ((c = getopt_long(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch(c) {
		case 'h':
			usage(cout, basename(argv[0]));
			return 0;

		case 'd':

			if (optarg) {
				flags.devid = strtoul(optarg, &end_ptr, 10);
				if (end_ptr == optarg) {
					cerr << "couldn't parse device ID" << endl;
					usage(cerr, basename(argv[0]));
					return 1;
				}
			}

			break;

		case 'o':
			output = optarg;
			break;

		case 'V':
			cout << THIN_PROVISIONING_TOOLS_VERSION << endl;
			return 0;

		default:
			usage(cerr, basename(argv[0]));
			return 1;
		}
	}

	if (argc == optind) {
		cerr << "No input file provided." << endl;
		usage(cerr, basename(argv[0]));
		return 1;
	}
	return send(argv[optind], output, flags);
}

base::command thin_provisioning::thin_send_cmd("thin_send", thin_send_main);

//----------------------------------------------------------------
