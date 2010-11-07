/*
 * Copyright (c) 2010 Remko Tronçon
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#include "Swiften/TLS/Certificate.h"

namespace Swift {

const char* Certificate::ID_ON_XMPPADDR_OID = "1.3.6.1.5.5.7.8.5";
const char* Certificate::ID_ON_DNSSRV_OID = "1.3.6.1.5.5.7.8.7";

Certificate::~Certificate() {
}

}
