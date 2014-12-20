// Copyright Luca Clementi, All rights reserved.
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

#include "send_format.h"

#include "base/indented_stream.h"
#include "base/xml_utils.h"

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string.h>

using namespace std;
using namespace thin_provisioning;
using namespace xml_utils;

namespace tp = thin_provisioning;

//----------------------------------------------------------------

namespace {
	//------------------------------------------------
	// XML generator
	//------------------------------------------------
	class send_emitter : public emitter {
	public:
		send_emitter(ostream &out, uint64_t device_id)
		: out_(out) {
			deviceid_ = device_id;
			emitting = false;
		}

		void begin_superblock(string const &uuid,
				      uint64_t time,
				      uint64_t trans_id,
				      uint32_t data_block_size,
				      uint64_t nr_data_blocks,
				      boost::optional<uint64_t> metadata_snap) {
		}

		void end_superblock() {
		}

//we need to undestand when to start to emit from the device_id
		void begin_device(uint32_t dev_id,
				  uint64_t mapped_blocks,
				  uint64_t trans_id,
				  uint64_t creation_time,
				  uint64_t snap_time) {
			if (deviceid_ == dev_id)
				emitting = true;
		}

		void end_device() {
			if (emitting == true)
				emitting = false;
		}

		void begin_named_mapping(string const &name) {
		}

		void end_named_mapping() {
		}

		void identifier(string const &name) {
		}

		void range_map(uint64_t origin_begin, uint64_t data_begin, uint32_t time, uint64_t len) {
			if (emitting) {
				out_ << "<range_mapping origin_begin=\"" << origin_begin << "\""
				     << " data_begin=\"" << data_begin << "\""
				     << " length=\"" << len << "\""
				     << " time=\"" << time << "\""
				     << "/>" << endl;
			}
		}

		void single_map(uint64_t origin_block, uint64_t data_block, uint32_t time) {
			if (emitting) {

				out_ << "<single_mapping origin_block=\"" << origin_block << "\""
				     << " data_block=\"" << data_block << "\""
				     << " time=\"" << time << "\""
				     << "/>" << endl;
			}
		}

	private:
		indented_stream out_;
		uint64_t deviceid_;
		bool emitting;
	};

	//------------------------------------------------
	// XML parser
	//------------------------------------------------
	void parse_superblock(emitter *e, attributes const &attr) {
		e->begin_superblock(get_attr<string>(attr, "uuid"),
				    get_attr<uint64_t>(attr, "time"),
				    get_attr<uint64_t>(attr, "transaction"),
				    get_attr<uint32_t>(attr, "data_block_size"),
				    get_attr<uint64_t>(attr, "nr_data_blocks"),
				    get_opt_attr<uint64_t>(attr, "metadata_snap"));
	}

	void parse_device(emitter *e, attributes const &attr) {
		e->begin_device(get_attr<uint32_t>(attr, "dev_id"),
				get_attr<uint64_t>(attr, "mapped_blocks"),
				get_attr<uint64_t>(attr, "transaction"),
				get_attr<uint64_t>(attr, "creation_time"),
				get_attr<uint64_t>(attr, "snap_time"));
	}

	void parse_range_mapping(emitter *e, attributes const &attr) {
		e->range_map(get_attr<uint64_t>(attr, "origin_begin"),
			     get_attr<uint64_t>(attr, "data_begin"),
			     get_attr<uint32_t>(attr, "time"),
			     get_attr<uint64_t>(attr, "length"));
	}

	void parse_single_mapping(emitter *e, attributes const &attr) {
		e->single_map(get_attr<uint64_t>(attr, "origin_block"),
			      get_attr<uint64_t>(attr, "data_block"),
			      get_attr<uint32_t>(attr, "time"));
	}

	void start_tag(void *data, char const *el, char const **attr) {
		emitter *e = static_cast<emitter *>(data);
		attributes a;

		build_attributes(a, attr);

		if (!strcmp(el, "superblock"))
			parse_superblock(e, a);

		else if (!strcmp(el, "device"))
			parse_device(e, a);

		else if (!strcmp(el, "range_mapping"))
			parse_range_mapping(e, a);

		else if (!strcmp(el, "single_mapping"))
			parse_single_mapping(e, a);

		else
			throw runtime_error("unknown tag type");
	}

	void end_tag(void *data, const char *el) {
		emitter *e = static_cast<emitter *>(data);

		if (!strcmp(el, "superblock"))
			e->end_superblock();

		else if (!strcmp(el, "device"))
			e->end_device();

		else if (!strcmp(el, "range_mapping")) {
			// do nothing

		} else if (!strcmp(el, "single_mapping")) {
			// do nothing

		} else
			throw runtime_error("unknown tag close");
	}
}

//----------------------------------------------------------------

tp::emitter::ptr
tp::create_send_emitter(ostream &out, uint64_t devid)
{
	return emitter::ptr(new send_emitter(out, devid));
}

//----------------------------------------------------------------
