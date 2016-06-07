
#include "config_module.h"
#include "dconfig.h"
#include "utOctree.h"


Configure( config_utilities );
NotifyCategoryDef( utilities , "");

ConfigureFn( config_utilities ) {
  init_libutilities();
}

void
init_libutilities() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  // Init your dynamic types here, e.g.:
  // MyDynamicClass::init_type();
  UTOctree::init_type();
  UTOctree::register_with_read_factory();

  return;
}

