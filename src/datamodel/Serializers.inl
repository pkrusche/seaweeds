/***************************************************************************
 *   Copyright (C) 2012   Peter Krusche, The University of Warwick         *
 *   pkrusche@gmail.com                                                    *
 ***************************************************************************/

#ifndef __Serializers_Inl__
#define __Serializers_Inl__

template <typename _int = int>
class JSONInt : public ValueSerializer<_int> {
public:
	void write(Json::Value & val, std::string const & path, _int const & value) {
		helpers::expand_path (val, path) = (int)value;
	}

	void read( Json::Value & val, std::string const & path, _int & value ) {
		value = helpers::expand_path (val, path).asInt();
	}
};

template <typename _int = int>
class JSONUInt : public ValueSerializer<_int>  {
public:
	void write(Json::Value & val, std::string const & path, _int const & value) {
		helpers::expand_path (val, path) = (unsigned int)value;
	}

	void read(Json::Value & val, std::string const & path, _int & value) {
		value = helpers::expand_path (val, path).asUInt();
	}
};

template <typename _float = double>
class JSONDouble : public ValueSerializer<_float>  {
public:
	void write(Json::Value & val, std::string const & path, _float const & value) {
		helpers::expand_path (val, path) = (double)value;
	}

	void read(Json::Value & val, std::string const & path, _float & value) {
		value = helpers::expand_path (val, path).asDouble();
	}
};

template <class _string = std::string>
class JSONString : public ValueSerializer< _string >  {
public:
	void write(Json::Value & val, std::string const & path, _string const & value) {
		helpers::expand_path (val, path) = std::string ( value );
	}

	void read(Json::Value & val, std::string const & path, _string & value) {
		value = helpers::expand_path (val, path).asString();
	}
};

template <class _binary>
class JSONBinary : public ValueSerializer< _binary >  {
public:
	void write(Json::Value & val, std::string const & path, _binary const & value) {
		helpers::expand_path (val, path) = std::string ( helpers::encode_base64((const char*)&value, sizeof(_binary)) );
	}

	void read(Json::Value & val, std::string const & path, _binary & value) {
		std::string tmp = helpers::decode_base64 ( helpers::expand_path (val, path).asCString() );
		if (tmp.size() != sizeof ( _binary ) ) {
			throw std::runtime_error("Error decoding JSON input. ");
		}
		memcpy (&value, tmp.c_str(), sizeof (_binary));
	}
};

template <class _binary>
class JSONBinaryViaStream : public ValueSerializer< _binary >  {
public:
	void write(Json::Value & val, std::string const & path, _binary const & value) {
		std::ostringstream os;
		os << value;
		std::string osstr(os.str());
		helpers::expand_path (val, path) = helpers::encode_base64(osstr.c_str(), osstr.size());
	}

	void read(Json::Value & val, std::string const & path, _binary & value) {
		const char * base64_string = helpers::expand_path (val, path).asCString();
		helpers::_binary cur(base64_string);
		helpers::_binary end(base64_string + strlen(base64_string));

		std::string tmp;
		size_t len = 0;

		while(cur != end)
		{
			tmp.resize(len+1);
			tmp[len] = *cur;
			++len;
			++cur;
		}

		std::istringstream iss(tmp, std::istringstream::out);
		iss >> value;
	}
};


namespace helpers {
	template <class _t, size_t n_elements>
	class ref_wrapper {
	public:
		ref_wrapper() : val(NULL) {}

		void init ( _t input_array [n_elements] ) {
			val = &(input_array[0]);
		}

		_t const & operator [] (size_t index) const {
			return val [ index ];
		}

		_t & operator [] (size_t index) {
			return val [ index ];
		}
	private:
		_t * val;
	};
};

template <class _wrappedserializer, size_t n_elements>
class JSONStaticArray : public ValueSerializer <
	helpers::ref_wrapper <
		typename _wrappedserializer :: value_type,
		n_elements >
> {
public:
	typedef ValueSerializer <
		helpers::ref_wrapper <
		typename _wrappedserializer :: value_type,
		n_elements >
	> super_type;
	typedef helpers::ref_wrapper < typename _wrappedserializer :: value_type, n_elements > _value_type;

	ValueSerializer < _value_type > * init (
		const char * _membername,
		typename _wrappedserializer::value_type _my_value []) {
		wr.init (_my_value);
		return super_type::init (_membername, wr);
	}

	void write(Json::Value & val, std::string const & path,
		const _value_type & value) {
			Json::Value & v = helpers::expand_path (val, path);

			v.clear();
			for (size_t j = 0; j < n_elements; ++j) {
				Json::Value vv;
				ws.write( vv, "", value[j] );
				v.append( vv );
			}
	}

	void read(Json::Value & val, std::string const & path,
		_value_type & value ) {
		Json::Value & v = helpers::expand_path (val, path);

		if (v.size() != n_elements) {
			throw std::runtime_error ("Invalid number of elements in serialized array field.");
		}
		for (size_t j = 0; j < n_elements; ++j) {
			ws.read (v[(Json::Value::UInt)j], "", value[j] );
		}
	}
private:
	_value_type  wr;
	_wrappedserializer ws;
};

template < class _wrappedserializer, class _container_type = std::vector < typename _wrappedserializer::value_type > >
class JSONArray : public ValueSerializer < _container_type > {
public:
	void write(Json::Value & val, std::string const & path,
		std::vector < typename _wrappedserializer::value_type > const & value) {
			Json::Value & v = helpers::expand_path (val, path);

			v.clear();
			for (size_t j = 0; j < value.size(); ++j) {
				Json::Value vv;
				ws.write( vv, "", value[j] );
				v.append( vv );
			}
	}

	void read(Json::Value & val, std::string const & path,
		std::vector < typename _wrappedserializer::value_type > & value ) {
			Json::Value & v = helpers::expand_path (val, path);

			value.resize(v.size());
			for (size_t j = 0; j < v.size(); ++j) {
				ws.read (v[(Json::Value::UInt)j], "", value[j] );
			}
	}
private:
	_wrappedserializer ws;
};

template < class _container_type >
class JSONEmptyArray : public ValueSerializer < _container_type > {
public:
	void write(Json::Value & val, std::string const & path,
		_container_type const & value) {
	}

	void read(Json::Value & val, std::string const & path,
		_container_type & value ) {
			value.clear();
	}
};

template < class _wrappedserializer >
class JSONMap : public ValueSerializer < std::map<std::string, typename _wrappedserializer::value_type > > {
public:
	typedef std::map<std::string, typename _wrappedserializer::value_type > value_type;
	void write(Json::Value & val, std::string const & path,
		value_type const & value) {
			Json::Value & v = helpers::expand_path (val, path);
			v.clear();

			for (typename value_type::const_iterator j = value.begin(); j != value.end(); ++j) {
				Json::Value vv;
				ws.write( vv, "", j->second );
				v[j->first] = vv;
			}
	}

	void read(Json::Value & val, std::string const & path,
		value_type & value ) {
			Json::Value & v = helpers::expand_path (val, path);

			for (Json::Value::iterator j = v.begin(); j != v.end(); ++j) {;
				ws.read ( *j, "", value[j.memberName()]);
			}
	}
private:
	_wrappedserializer ws;
};


template <class _serializable>
class JSONSerializable : public ValueSerializer< _serializable >  {
public:
	typedef _serializable value_type;
	void write(Json::Value & val, std::string const & path, _serializable const & value) {
		value.archive(helpers::expand_path (val, path));
	}

	void read(Json::Value & val, std::string const & path, _serializable & value) {
		value.unarchive(helpers::expand_path (val, path));
	}
};


#endif // __Serializers_Inl__
