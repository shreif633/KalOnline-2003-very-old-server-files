#include "auth/AuthServer.hpp"
#include "common/crypto/Crypto.hpp"

namespace kal::auth {

std::optional<Session> AuthServer::create_session(uint32_t user_id, 
                                                   const std::string& username,
                                                   const std::string& ip) {
    auto cfg = config::Config::instance().get_int("auth.session_timeout_min", 30);
    
    Session session{
        .session_id = m_next_session_id++,
        .user_id = user_id,
        .username = username,
        .created_at = std::chrono::steady_clock::now(),
        .expires_at = std::chrono::steady_clock::now() + std::chrono::minutes(cfg),
        .remote_ip = ip
    };
    
    {
        std::unique_lock<std::shared_mutex> lock(m_sessions_mutex);
        m_sessions[session.session_id] = session;
    }
    
    return session;
}

void AuthServer::cleanup_expired_sessions() {
    std::unique_lock<std::shared_mutex> lock(m_sessions_mutex);
    
    auto now = std::chrono::steady_clock::now();
    
    for (auto it = m_sessions.begin(); it != m_sessions.end();) {
        if (it->second.expires_at < now) {
            it = m_sessions.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace kal::auth
