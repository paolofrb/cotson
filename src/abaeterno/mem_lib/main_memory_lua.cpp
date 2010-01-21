#include "abaeterno_config.h"
#include "lua_helpers.h"

using namespace std;
using namespace boost;

namespace {

using namespace boost::assign;
using namespace Memory;

lua* build_memory(luabind::object table)
{
	set<string> required;
	required += "name", "latency";
	Parameters p=Option::from_lua_table(table,required,"Memory");
	p.set("type",     "memory");
	p.track();

	Interface::Shared sh;
	lua* th=new lua(sh=Interface::Shared(Factory<Interface>::create(p)));
	Interface* mp=sh.get();
	TaggedMetrics tm=TaggedMetrics::get();
	tm.add(mp,p.get<string>("name"));
	tm.add(mp,p.get<string>("type"));
	tm.add(mp,"mem");
	if(p.has("tag"))
		tm.add(mp,p.get<string>("tag"));

	lua::destroy().add(th);
	return th;
}

luabind::scope build_memory_binder()
{
	return luabind::def("Memory",&build_memory);
}

register_lua o(build_memory_binder);

}
