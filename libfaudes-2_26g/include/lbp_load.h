/* Auto-generated file to define lua module loading for all enabled plugins */

/* 
The build system appends definitions of load functions for each plugin. 
They are resolved in libluafaudes. There is also a function luaopen_libfaudes() 
to include all supported plugins
*/

// have lua state defined
extern "C" {
#include "lua/lua.h"
}


// over all load
int luaopen_faudes_allplugins(lua_State* L); 

// load known modules 
extern "C" {
extern int luaopen_faudes(lua_State* L); 
 extern int luaopen_synthesis(lua_State* L); 
 extern int luaopen_observer(lua_State* L); 
 extern int luaopen_observability(lua_State* L); 
 extern int luaopen_diagnosis(lua_State* L); 
 extern int luaopen_hiosys(lua_State* L); 
 extern int luaopen_iosystem(lua_State* L); 
 extern int luaopen_multitasking(lua_State* L); 
 extern int luaopen_coordinationcontrol(lua_State* L); 
 extern int luaopen_pushdown(lua_State* L); 

}
