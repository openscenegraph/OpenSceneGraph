/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef IFFPARSER_
#define IFFPARSER_

#include <vector>
#include <iostream>

namespace iff
{

	typedef std::vector<struct Chunk *> Chunk_list;

	struct Chunk {

		virtual ~Chunk()
		{ 
			Chunk_list *fod = free_on_destroy();
			if (fod) {
				for (Chunk_list::iterator i=fod->begin(); i!=fod->end(); ++i) {
					delete *i;
				}
			}
		}

		virtual Chunk_list *free_on_destroy() { return 0; }
	};	

	template<typename Iter>
	class GenericParser {
	public:
		GenericParser();
		GenericParser(std::ostream &os);

		virtual ~GenericParser();

		void clear();
		void parse(Iter begin, Iter end);

		inline const Chunk_list &chunks() const;

	protected:
		virtual Chunk *parse_chunk_data(const std::string &tag, const std::string &context, Iter it, Iter end) = 0;
		Chunk *parse_chunk(Iter &it, const std::string &context);

		inline std::ostream &os() { return os_; }

	private:		
		Chunk_list chunks_;
		std::ostream &os_;
	};

	/////////////////////////////////////////////////////////////////////////
	// IMPLEMENTATION OF TEMPLATE FUNCTIONS

#	define IP_TMP	template<class Iter>

	IP_TMP GenericParser<Iter>::GenericParser()
		: os_(std::cout)
	{
	}

	IP_TMP GenericParser<Iter>::GenericParser(std::ostream &os)
		: os_(os)
	{
	}

	IP_TMP GenericParser<Iter>::~GenericParser()
	{
	}

	IP_TMP void GenericParser<Iter>::clear()
	{
		chunks_.clear();
	}

	IP_TMP void GenericParser<Iter>::parse(Iter begin, Iter end)
	{
		Iter it = begin;
		while (it < end) {
			Chunk *chk = parse_chunk(it, "");
			if (chk) chunks_.push_back(chk);
		}
	}

	IP_TMP Chunk *GenericParser<Iter>::parse_chunk(Iter &it, const std::string &context)
	{
		std::string tag;
		for (int i=0; i<4; ++i) tag += *(it++);
		unsigned int len = ((static_cast<unsigned int>(*(it)) & 0xFF) << 24) |
			((static_cast<unsigned int>(*(it+1)) & 0xFF) << 16) |
			((static_cast<unsigned int>(*(it+2)) & 0xFF) << 8) |
			(static_cast<unsigned int>(*(it+3)) & 0xFF);
                it += 4;
		os_ << "DEBUG INFO: iffparser: reading chunk " << tag << ", length = " << len << ", context = " << context << "\n";
		Chunk *chk = parse_chunk_data(tag, context, it, it+len);
		if (!chk) os_ << "DEBUG INFO: iffparser: \tprevious chunk not handled\n";
		it += len;
		if ((len % 2) != 0) ++it;
		return chk;
	}

	IP_TMP const Chunk_list &GenericParser<Iter>::chunks() const
	{
		return chunks_;
	}

}

#endif
