#ifndef CONFIG_UTILITIES_H
#define CONFIG_UTILITIES_H

#pragma once

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableDouble.h"
#include "configVariableString.h"
#include "configVariableInt.h"


NotifyCategoryDecl(utilities, EXPORT_CLASS, EXPORT_TEMPL);

extern void init_libutilities();

#endif
