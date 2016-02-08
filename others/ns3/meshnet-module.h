
#ifdef NS3_MODULE_COMPILATION
# error "Do not include ns3 module aggregator headers from other modules; these are meant only for end user scripts."
#endif

#ifndef NS3_MODULE_MESHNET
    

// Module headers:
#include "hu11s-installer.h"
#include "hu11s-mac-header.h"
#include "huper-link-frame.h"
#include "huper-link.h"
#include "huper-management-protocol.h"
#include "ie-hu11s-beacon-timing.h"
#include "ie-hu11s-configuration.h"
#include "ie-hu11s-huper-management.h"
#include "ie-hu11s-hupering-protocol.h"
#include "ie-hu11s-id.h"
#include "ie-hu11s-metric-report.h"
#include "ie-hu11s-perr.h"
#include "ie-hu11s-prep.h"
#include "ie-hu11s-preq.h"
#include "ie-hu11s-rann.h"
#include "meshnet-helper.h"
#include "meshnet-information-element-vector.h"
#include "meshnet-information-element.h"
#include "meshnet-l2-routing-protocol.h"
#include "meshnet-point-device.h"
#include "meshnet-stack-installer.h"
#include "meshnet-wifi-beacon.h"
#include "meshnet-wifi-interface-mac-plugin.h"
#include "meshnet-wifi-interface-mac.h"
#include "mgmp-protocol.h"
#include "mgmp-rtable.h"
#endif
