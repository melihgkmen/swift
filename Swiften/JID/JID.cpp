/*
 * Copyright (c) 2010 Remko Tronçon
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#define SWIFTEN_CACHE_JID_PREP

#include <vector>
#include <list>
#include <iostream>

#include <string>
#ifdef SWIFTEN_CACHE_JID_PREP
#include <boost/unordered_map.hpp>
#endif
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/find_format.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <sstream>
#include <stringprep.h>

#include <Swiften/Base/String.h>
#include "Swiften/JID/JID.h"
#include "Swiften/IDN/StringPrep.h"

#ifdef SWIFTEN_CACHE_JID_PREP
typedef boost::unordered_map<std::string, std::string> PrepCache;

static PrepCache nodePrepCache;
static PrepCache domainPrepCache;
static PrepCache resourcePrepCache;
#endif

static const std::list<char> escapedChars = boost::assign::list_of(' ')('"')('&')('\'')('/')('<')('>')('@')(':');

bool getEscapeSequenceValue(const std::string& sequence, unsigned char& value) {
	std::stringstream s;
	unsigned int v;
	s << std::hex << sequence;
	s >> v;
	value = static_cast<unsigned char>(v);
	return (!s.fail() && !s.bad() && (value == 0x5C || std::find(escapedChars.begin(), escapedChars.end(), value) != escapedChars.end()));
}

struct UnescapedCharacterFinder {
	template<typename Iterator>	boost::iterator_range<Iterator> operator()(Iterator begin, Iterator end) {
		for (; begin != end; ++begin) {
			if (std::find(escapedChars.begin(), escapedChars.end(), *begin) != escapedChars.end()) {
				return boost::iterator_range<Iterator>(begin, begin + 1);
			}
			else if (*begin == '\\') {
				// Check if we have an escaped dissalowed character sequence
				Iterator innerBegin = begin + 1;
				if (innerBegin != end && innerBegin + 1 != end) {
					Iterator innerEnd = innerBegin + 2;
					unsigned char value;
					if (getEscapeSequenceValue(std::string(innerBegin, innerEnd), value)) {
						return boost::iterator_range<Iterator>(begin, begin + 1);
					}
				}
			}
		}
		return boost::iterator_range<Iterator>(end, end);
	}
};

struct UnescapedCharacterFormatter {
	template<typename FindResult>	std::string operator()(const FindResult& match) const {
		std::ostringstream s;
		s << '\\' << std::hex << static_cast<int>(*match.begin());
		return s.str();
	}
};

struct EscapedCharacterFinder {
	template<typename Iterator>	boost::iterator_range<Iterator> operator()(Iterator begin, Iterator end) {
		for (; begin != end; ++begin) {
			if (*begin == '\\') {
				Iterator innerEnd = begin + 1;
				for (size_t i = 0; i < 2 && innerEnd != end; ++i, ++innerEnd) {
				}
				unsigned char value;
				if (getEscapeSequenceValue(std::string(begin + 1, innerEnd), value)) {
					return boost::iterator_range<Iterator>(begin, innerEnd);
				}
			}
		}
		return boost::iterator_range<Iterator>(end, end);
	}
};

struct EscapedCharacterFormatter {
	template<typename FindResult>	std::string operator()(const FindResult& match) const {
		unsigned char value;
		if (getEscapeSequenceValue(std::string(match.begin() + 1, match.end()), value)) {
			return std::string(reinterpret_cast<const char*>(&value), 1);
		}
		return boost::copy_range<std::string>(match);
	}
};


namespace Swift {

JID::JID(const char* jid) {
	initializeFromString(std::string(jid));
}

JID::JID(const std::string& jid) {
	initializeFromString(jid);
}

JID::JID(const std::string& node, const std::string& domain) : hasResource_(false) {
	nameprepAndSetComponents(node, domain, "");
}

JID::JID(const std::string& node, const std::string& domain, const std::string& resource) : hasResource_(true) {
	nameprepAndSetComponents(node, domain, resource);
}

void JID::initializeFromString(const std::string& jid) {
	if (String::beginsWith(jid, '@')) {
		return;
	}

	std::string bare, resource;
	size_t slashIndex = jid.find('/');
	if (slashIndex != jid.npos) {
		hasResource_ = true;
		bare = jid.substr(0, slashIndex);
		resource = jid.substr(slashIndex + 1, jid.npos);
	}
	else {
		hasResource_ = false;
		bare = jid;
	}
	std::pair<std::string,std::string> nodeAndDomain = String::getSplittedAtFirst(bare, '@');
	if (nodeAndDomain.second.empty()) {
		nameprepAndSetComponents("", nodeAndDomain.first, resource);
	}
	else {
		nameprepAndSetComponents(nodeAndDomain.first, nodeAndDomain.second, resource);
	}
}


void JID::nameprepAndSetComponents(const std::string& node, const std::string& domain, const std::string& resource) {
#ifndef SWIFTEN_CACHE_JID_PREP
	node_ = StringPrep::getPrepared(node, StringPrep::NamePrep);
	domain_ = StringPrep::getPrepared(domain, StringPrep::XMPPNodePrep);
	resource_ = StringPrep::getPrepared(resource, StringPrep::XMPPResourcePrep);
#else
	std::pair<PrepCache::iterator, bool> r;

	r = nodePrepCache.insert(std::make_pair(node, std::string()));
	if (r.second) {
		r.first->second = StringPrep::getPrepared(node, StringPrep::NamePrep);
	}
	node_ = r.first->second;

	r = domainPrepCache.insert(std::make_pair(domain, std::string()));
	if (r.second) {
		r.first->second = StringPrep::getPrepared(domain, StringPrep::XMPPNodePrep);
	}
	domain_ = r.first->second;

	r = resourcePrepCache.insert(std::make_pair(resource, std::string()));
	if (r.second) {
		r.first->second = StringPrep::getPrepared(resource, StringPrep::XMPPResourcePrep);
	}
	resource_ = r.first->second;
#endif
}

std::string JID::toString() const {
	std::string string;
	if (!node_.empty()) {
		string += node_ + "@";
	}
	string += domain_;
	if (!isBare()) {
		string += "/" + resource_;
	}
	return string;
}

int JID::compare(const Swift::JID& o, CompareType compareType) const {
	if (node_ < o.node_) { return -1; }
	if (node_ > o.node_) { return 1; }
	if (domain_ < o.domain_) { return -1; }
	if (domain_ > o.domain_) { return 1; }
	if (compareType == WithResource) {
		if (hasResource_ != o.hasResource_) {
			return hasResource_ ? 1 : -1;
		}
		if (resource_ < o.resource_) { return -1; }
		if (resource_ > o.resource_) { return 1; }
	}
	return 0;
}

std::string JID::getEscapedNode(const std::string& node) {
	return boost::find_format_all_copy(node, UnescapedCharacterFinder(), UnescapedCharacterFormatter());
}

std::string JID::getUnescapedNode() const {
	return boost::find_format_all_copy(node_, EscapedCharacterFinder(), EscapedCharacterFormatter());
}

} // namespace Swift

