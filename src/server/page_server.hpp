/*
 * Copyright 2008 Search Solution Corporation
 * Copyright 2016 CUBRID Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#ifndef _PAGE_SERVER_HPP_
#define _PAGE_SERVER_HPP_

#include "ats_ps_request.hpp"
#include "log_page_async_fetcher.hpp"
#include "request_client_server.hpp"
#include "request_sync_send_queue.hpp"

#include <memory>

// forward declaration
namespace cublog
{
  class replicator;
}
namespace cubpacking
{
  class unpacker;
}
namespace cubthread
{
  class entry;
}

class page_server
{
  public:
    using active_tran_server_conn = cubcomm::request_client_server<ps_to_ats_request, ats_to_ps_request>;

    page_server () = default;
    ~page_server ();

    void set_active_tran_server_connection (cubcomm::channel &&chn);
    void disconnect_active_tran_server ();
    bool is_active_tran_server_connected () const;
    void push_request_to_active_tran_server (ps_to_ats_request reqid, std::string &&payload);

    void start_log_replicator (const log_lsa &start_lsa);
    void finish_replication (cubthread::entry &thread_entry);

    void init_log_page_fetcher ();
    void finalize_log_page_fetcher ();

  private:
    using active_tran_server_request_queue = cubcomm::request_sync_send_queue<active_tran_server_conn, std::string>;
    using active_tran_server_request_autosend = cubcomm::request_queue_autosend<active_tran_server_request_queue>;

    void receive_log_prior_list (cubpacking::unpacker &upk);
    void receive_log_page_fetch (cubpacking::unpacker &upk);
    void receive_data_page_fetch (cubpacking::unpacker &upk);

    void on_log_page_read_result (const LOG_PAGE *log_page, int error_code);

    std::unique_ptr<active_tran_server_conn> m_ats_conn;
    std::unique_ptr<active_tran_server_request_queue> m_ats_request_queue;
    std::unique_ptr<active_tran_server_request_autosend> m_ats_request_autosend;

    std::unique_ptr<cublog::replicator> m_replicator;
    std::unique_ptr<cublog::async_page_fetcher> m_log_page_fetcher;
};

extern page_server ps_Gl;

#endif // !_PAGE_SERVER_HPP_
