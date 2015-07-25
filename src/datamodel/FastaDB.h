/***************************************************************************
 *   Copyright (C) 2011   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/
#ifndef __fastareader_H__
#define __fastareader_H__

#include <fstream>
#include <list>
#include <map>
#include <string>

#include "bspcpp/tools/utilities.h"

namespace datamodel {
	template <size_t _bpc>
	class FastaDB {
	public:
		typedef utilities::IntegerVector< _bpc > sequence_string;

		/**
		 * Read fasta file
		 */
		void read(const char * filename) {
			using namespace std;
			using namespace utilities;

			ifstream in(filename);

			if(in.bad() || !in.is_open()) {
				throw "File not found"; // TODO fix with proper exception
			}

			string current_line;
			sequence current_sequence;
			current_sequence.id = "no sequence id specified.";
			current_sequence.seq = "";
			bool previous_was_comment = false;

			while (in.good()) {
				if (getline(in, current_line, '\n')) {
					if(current_line.at(0) == '>' ||
						current_line.at(0) == ';') {
							previous_was_comment = true;
							// comment line. append to sequence id.
							if(previous_was_comment) {
								current_sequence.id += current_line;
							} else {
								if(current_sequence.seq.length() > 0) {
									sequences.push_back(current_sequence);
									current_sequence.id = current_line;
									current_sequence.seq = "";
								}
							}
					} else {
						previous_was_comment = false;
						current_sequence.seq += current_line;
					}
				}
			}

			if(current_sequence.seq.length() > 0) {
				sequences.push_back(current_sequence);
			}
		}

		/**
		 * write fasta file
		 */
		void write (const char * filename) {
			using namespace std;

			ofstream o (filename);
			if (! o.good()) {
				throw "Could not open file for writing.";
			}

			for (vector<string>::iterator it = sequences.begin(), end = sequences.end();
				it != end; ++it) {
				o << ">" << it->id << endl;
				TextIO::print_multiline_string(o, it->seq.c_str());
				o << endl;
			}
		}

		/************************************************************************/
		/* Content management                                                   */
		/************************************************************************/

		void clear() {
			sequences.clear();
		}

		size_t get_number_of_sequences() {
			return sequences.size();
		}

		std::string & get_sequence(size_t num) {
			return *(sequences[num].seq);
		}

		std::string & get_id(size_t num) {
			return sequences[num].id;
		}

		void delete_id(size_t num) {
			sequences.erase(num);
		}

		void add_sequence (const char * id, const char * seq) {
			sequence sq;
			sq.id = id;
			sq.seq = seq;
			sequences.push_back(sq);
		}

	private:
		typedef struct _sequence {
			std::string id;
			std::string seq;
		} sequence;

		std::vector<sequence> sequences;
	};

};


#endif // __fastareader_H__
