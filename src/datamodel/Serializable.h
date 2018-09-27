/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __Serializable_H__
#define __Serializable_H__

#include "autoconfig.h"

#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <sstream>
#include <map>
#include <stdexcept>

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

#include "util/TypeList.h"
#include "datamodel/TextIO.h"

#include "json/json.h"

#include "boost/mpl/string.hpp"

namespace datamodel {

	namespace helpers {
		inline Json::Value & expand_path (Json::Value & val, std::string const & path) {
			std::vector<std::string> paths;
			TextIO::split(path, paths, ".");
			Json::Value & node (val);
			while (paths.size() > 1) {
				node = node.get (paths[0], Json::Value ()) ;
				paths.erase(paths.begin());
			}
			if (paths.size() == 1 && path != "") {
				return node [paths[0]];
			}
			return node;
		}

		typedef boost::archive::iterators::base64_from_binary<
			boost::archive::iterators::transform_width<const char *, 6, 8 > >
			_base64;
		typedef boost::archive::iterators::transform_width<
			boost::archive::iterators::binary_from_base64< const char * >,
			8, 6 >
			_binary;

		inline std::string encode_base64 (const char * value, size_t len) {
			std::ostringstream os;

			std::copy( _base64 (value),
				_base64 (value + len),
				boost::archive::iterators::ostream_iterator<char>(os)
				);
			return os.str();
		}

		inline std::string decode_base64 ( const char * value ) {
			std::ostringstream os;

			std::copy( _binary (value),
				_binary (value + (strlen(value)-1)),
				boost::archive::iterators::ostream_iterator<char>(os)
				);
			return os.str();
		}

	};

	// untemplated serializer interface
	class ItemSerializer {
	public:
		virtual ~ItemSerializer () {}

		virtual void write(Json::Value & val) = 0;
		virtual void read(Json::Value & val) = 0;
	};

	template < class _t >
	class ValueSerializer : public ItemSerializer {
	public:
		typedef _t value_type;

		virtual ValueSerializer<_t> * init (const char * _membername, _t & _my_value) {
			membername = _membername;
			my_value = &_my_value;
			return this;
		}

		virtual void write(Json::Value & val) {
			write (val, membername, *my_value);
		}

		virtual void read(Json::Value & val){
			read (val, membername, *my_value);
		}

	protected:
		virtual void write(Json::Value & val,
			std::string const & path, _t const & value)  = 0;
		virtual void read( Json::Value & val,
			std::string const & path, _t & value ) = 0;
	private:
		_t * my_value;
		std::string membername;
	};

#include "Serializers.inl"

	class JSONVersion : public ItemSerializer  {
	public:
		JSONVersion (const char * classname, size_t version) {
			std::ostringstream s;
			s << classname << ":" << version;
			myversion = s.str();
		}

		void write(Json::Value & val) {
			helpers::expand_path (val, "SERIAL_VERSIONID") =
				myversion.c_str();
		}

		void read(Json::Value & val) {
			std::string value = helpers::expand_path (val, "SERIAL_VERSIONID").asString();
			if (value.find(myversion) != 0) {
//			if (myversion != value) {
				std::cerr << "WARNING: Serialization version mismatch! Expected "
					<< myversion << " and got " << value << std::endl;
			}
		}
	private:
		std::string myversion;
	};

#define JSONIZE(_class, version, ...) JSONIZE_AS(#_class, _class, version, __VA_ARGS__)

#define JSONIZE_AS(_perl_class, _class, version, ...) public: _class () {  \
	make_serializers () ; \
	} \
	_class (_class const & c) {  \
	make_serializers () ; \
	*this = c; \
	} \
	void make_serializers () { \
		this->my_serializers.push_back( \
		new datamodel::JSONVersion (_perl_class, version) \
	); \
	__VA_ARGS__ ; \
	defaults(); \
} private:

#define S_STORE(member, ...) do { \
		using namespace datamodel; \
		this->my_serializers.push_back( \
			(new __VA_ARGS__) -> init ( #member, member ) ); \
		} while (0);

	/************************************************************************/
	/* C++ equivalent of APPLES Serializable class                          */
	/************************************************************************/
	class Serializable {
	public:

		/**
		 * Slow defaults. Override if necessary.
		 */
		virtual size_t byte_size() {
			std::ostringstream oss;
			archive(oss);
			std::string s = oss.str();
			return s.size();
		}

		/**
		 * Slow defaults. Override if necessary.
		 */
		virtual void byte_serialize ( void * buffer, size_t size ) {
			ASSERT (size >= sizeof(size_t));
			std::ostringstream oss;
			archive(oss);
			std::string s = oss.str();
			ASSERT(s.size() + sizeof(size_t) <= size);
			*((size_t *)buffer) = s.size();
			memcpy(((size_t *)buffer) + 1, s.c_str(), s.size());
		}

		/**
		 * Slow defaults. Override if necessary.
		 */
		virtual void byte_deserialize(void * source, size_t nbytes) {
			ASSERT (nbytes >= sizeof(size_t) );
			size_t len = *((size_t*)source);
			ASSERT (nbytes >= sizeof(size_t) + len );
			std::string s( (char*) ( ((size_t*)source) + 1), len);
			std::istringstream i (s);
			unarchive(i);
		}

	protected:

		Serializable const & operator= (Serializable const & rhs) {
			if (this == &rhs) {
				return *this;
			}
			reset_serializers();
			this->make_serializers();
			json_value = rhs.json_value;
			return *this;
		}

	public:
		/**
		 * since we generate the default constructor (Serializable objects must
		 * have one so we can generate classes automatically, we supply a virtual
		 * function to provide default values.
		 */
		virtual void defaults () {}
		virtual ~Serializable () {
			reset_serializers();
		}

		void archive (Json::Value & root) const {
			pre_serialize ();
			size_t itemcount = my_serializers.size();
			root = json_value;
			for (size_t j = 0; j < itemcount; ++j) {
				my_serializers[j]->write(root);
			}
		}

		void archive ( std::ostream& os ) const {
			size_t itemcount = my_serializers.size();
			Json::Value root;
			archive(root);
			Json::StyledStreamWriter w;
			w.write(os, root);
		}

		void unarchive ( Json::Value & root ) {
			size_t itemcount = my_serializers.size();
			for (size_t j = 0; j < itemcount; ++j)
				my_serializers[j]->read(root);
			json_value = root;
			post_serialize ();
		}

		void unarchive ( std::istream& is ) {
			size_t itemcount = my_serializers.size();
			Json::Value root;
			Json::Reader r;
			r.parse(is, root);
			unarchive(root);
		}

	protected:
		virtual void pre_serialize () const {}
		virtual void post_serialize () {}

		virtual void make_serializers() = 0;

		void reset_serializers () {
			for (std::vector < ItemSerializer * >::iterator it = my_serializers.begin();
				it != my_serializers.end(); ++it) {
					delete  (*it);
			}
			my_serializers.clear();
		}

		std::vector<ItemSerializer*> my_serializers; ///< filled by JSONIZE macro
		Json::Value json_value;  ///< we keep a copy of our JSON representation

	};

	inline std::ostream & operator<< (std::ostream& os, const Serializable & val) {
		val.archive(os);
		return os;
	}

	inline std::istream & operator>> (std::istream& is, Serializable & val) {
		val.unarchive(is);
		return is;
	}

}

/************************************************************************/
/* Make Serializable objects BSP binary-serializable                    */
/*                                                                      */
/* This will be really slow. Override if used heavily.                  */
/************************************************************************/

#include "bsp_cpp/Shared/SharedVariable.h"

namespace bsp {

	template <>
	inline size_t SharedSerializable<datamodel::Serializable>::serialized_size() {
		datamodel::Serializable * thiz = (datamodel::Serializable*)valadr;
		return thiz->byte_size();
	}

	template <>
	inline void SharedSerializable<datamodel::Serializable>::serialize (void * target, size_t nbytes) {
		datamodel::Serializable * thiz = (datamodel::Serializable*)valadr;
		thiz->byte_serialize(target, nbytes);
	}

	template <>
	inline void SharedSerializable<datamodel::Serializable>::deserialize(void * source, size_t nbytes) {
		datamodel::Serializable * thiz = (datamodel::Serializable*)valadr;
		thiz->byte_deserialize(source, nbytes);
	}

};


#endif // __Serializable_H__
