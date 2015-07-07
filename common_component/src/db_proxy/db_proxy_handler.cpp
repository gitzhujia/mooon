// Writed by yijian (eyjian@qq.com, eyjian@gmail.com)
#include "db_proxy_handler.h"
#include "config_loader.h"
#include <mooon/sys/log.h>
#include <mooon/sys/simple_db.h>
#include <mooon/utils/format_string.h>
#include <mooon/utils/string_utils.h>
#include <thrift/TApplicationException.h>
namespace mooon { namespace db_proxy {

void CDbProxyHandler::query(DBTable& _return, const std::string& sign, const int32_t seq, const int32_t query_index, const std::vector<std::string> & tokens, const int32_t limit, const int32_t limit_start)
{
    CConfigLoader* config_loader = CConfigLoader::get_singleton();
    struct QueryInfo query_info;

    if (!config_loader->get_query_info(query_index, &query_info))
    {
        MYLOG_ERROR("query_index[%d] not exists\n", query_index);
        throw apache::thrift::TApplicationException("query_index not exists");
    }

    try
    {
        sys::DBConnection* db_connection = config_loader->get_db_connection(query_info.database_index);
        if (NULL == db_connection)
        {
            MYLOG_ERROR("database_index[%d] not exists\n", query_info.database_index);
            throw apache::thrift::TApplicationException("database_index not exists");
        }
        else if (tokens.size() > utils::FORMAT_STRING_SIZE)
        {
            MYLOG_ERROR("%d\n", seq);
            throw apache::thrift::TApplicationException("tokens too many");
        }
        else
        {
            std::string sql = utils::format_string(query_info.sql_template.c_str(), tokens);

            MYLOG_DEBUG("%s LIMIT %d,%d\n", sql.c_str(), limit_start, limit);
            if (limit_start > 0)
                db_connection->query(_return, "%s LIMIT %d,%d", sql.c_str(), limit_start, limit);
            else
                db_connection->query(_return, "%s LIMIT %d", sql.c_str(), limit);
        }
    }
    catch (sys::CDBException& db_ex)
    {
        MYLOG_ERROR("%s\n", db_ex.str().c_str());
        throw apache::thrift::TApplicationException(db_ex.str());
    }
}

int CDbProxyHandler::update(const std::string& sign, const int32_t seq, const int32_t update_index, const std::vector<std::string> & tokens)
{
    CConfigLoader* config_loader = CConfigLoader::get_singleton();
    struct UpdateInfo update_info;

    if (!config_loader->get_update_info(update_index, &update_info))
    {
        MYLOG_ERROR("update_index[%d] not exists\n", update_index);
        throw apache::thrift::TApplicationException("update_index not exists");
    }

    try
    {
        sys::DBConnection* db_connection = config_loader->get_db_connection(update_info.database_index);
        if (NULL == db_connection)
        {
            MYLOG_ERROR("database_index[%d] not exists\n", update_info.database_index);
            throw apache::thrift::TApplicationException("database_index not exists");
        }
        else if (tokens.size() > utils::FORMAT_STRING_SIZE)
        {
            MYLOG_ERROR("%d\n", seq);
            throw apache::thrift::TApplicationException("tokens too many");
        }
        else
        {
            std::string sql = utils::format_string(update_info.sql_template.c_str(), tokens);

            MYLOG_DEBUG("%s\n", sql.c_str());
            int affected_rows = db_connection->update("%s", sql.c_str());
            return affected_rows;
        }
    }
    catch (sys::CDBException& db_ex)
    {
        MYLOG_ERROR("%s\n", db_ex.str().c_str());
        throw apache::thrift::TApplicationException(db_ex.str());
    }
}

} // namespace mooon
} // namespace db_proxy