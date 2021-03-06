/*
 * Copyright (c) 2010-2019 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#pragma once

#include <memory>

#include <boost/noncopyable.hpp>

#include <Swiften/Parser/XMLParser.h>

namespace Swift {
    /**
     * Warning: This constructor is not thread-safe, because it depends on global state to
     * check whether it is initialized.
     */
    class LibXMLParser : public XMLParser, public boost::noncopyable {
        public:
            LibXMLParser(XMLParserClient* client, bool allowComments = false);
            virtual ~LibXMLParser();

            bool parse(const std::string& data, bool finalData = false);

            void stopParser();

        private:
            static bool initialized;
            bool stopped_ = false;

            struct Private;
            const std::unique_ptr<Private> p;
    };
}
