#include "LuaDocument.h"
#include <RmlUi/Core/Stream.h>
#include <RmlUi/Lua/IncludeLua.h>
#include <RmlUi/Lua/Interpreter.h>
#include "Document.h"

namespace Rml {
namespace Lua {

namespace {

class DocumentGlobalScope {
public:
	DocumentGlobalScope(lua_State* lua_state, ElementDocument* document) : L(lua_state)
	{
		lua_getglobal(L, "document");
		previous_document_ref = luaL_ref(L, LUA_REGISTRYINDEX);
		LuaType<Document>::push(L, document, false);
		lua_setglobal(L, "document");
	}

	~DocumentGlobalScope()
	{
		if (previous_document_ref == LUA_REFNIL)
			lua_pushnil(L);
		else
			lua_rawgeti(L, LUA_REGISTRYINDEX, previous_document_ref);
		lua_setglobal(L, "document");
		luaL_unref(L, LUA_REGISTRYINDEX, previous_document_ref);
	}

private:
	lua_State* L;
	int previous_document_ref;
};

} // namespace

LuaDocument::LuaDocument(const String& tag) : ElementDocument(tag) {}

void LuaDocument::LoadInlineScript(const String& context, const String& source_path, int source_line)
{
	String buffer;
	buffer += "--";
	buffer += source_path;
	buffer += ":";
	buffer += Rml::ToString(source_line);
	buffer += "\n";
	buffer += context;
	DocumentGlobalScope document_scope(Interpreter::GetLuaState(), this);
	Interpreter::DoString(buffer, buffer);
}

void LuaDocument::LoadExternalScript(const String& source_path)
{
	DocumentGlobalScope document_scope(Interpreter::GetLuaState(), this);
	Interpreter::LoadFile(source_path);
}

} // namespace Lua
} // namespace Rml
