/** 
 * @file ecu_lua_script.cpp
 * 
 * This file contains the class which represents the corresponding Lua script.
 */

#include "ecu_lua_script.h"
#include "utilities.h"
#include <iostream>

using namespace std;

/**
 * Constructor. 
 * 
 * @param ecuIdent: the identifier name for the ECU (e.g. "PCM")
 * @param luaScript: the path to the Lua script
 */
EcuLuaScript::EcuLuaScript(const string& ecuIdent, const string& luaScript)
{
    if (utils::existsFile(luaScript))
    {
        lua_state_.Load(luaScript);
        ecu_ident_ = ecuIdent;
    }
    else
    {
        throw;
    }
}

/**
 * Gets the UDS request ID according to the loaded Lua script.
 * 
 * @return the request ID or 0 on error
 */
uint16_t EcuLuaScript::getRequestId() const
{
    return int(lua_state_[ecu_ident_.c_str()][REQ_ID_FIELD]);
}

/**
 * Gets the UDS response ID according to the loaded Lua script.
 * 
 * @return the response ID or 0 on error
 */
uint16_t EcuLuaScript::getResponseId() const
{
    return int(lua_state_[ecu_ident_.c_str()][RES_ID_FIELD]);
}

/**
 * Reads the data according to `ReadDataByIdentifier`-table in the Lua script.
 * 
 * @param identifier: the identifier to access the field in the Lua table
 * @return the identifier field on success, otherwise an empty std::string
 */
string EcuLuaScript::getDataByIdentifier(uint16_t identifier) const
{
    auto val = lua_state_[ecu_ident_.c_str()][READ_DATA_BY_IDENTIFIER_TABLE][identifier];
    if (val.exists())
    {
        return val;
    }
    return "";
}

std::string EcuLuaScript::getSeed(uint8_t seed_level) const
{
    auto val = lua_state_[ecu_ident_.c_str()][READ_SEED][seed_level];
    if (val.exists())
    {
        return val;
    }
    return "";
}
