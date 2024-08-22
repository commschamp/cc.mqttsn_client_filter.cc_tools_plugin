//
// Copyright 2024 - 2025 (C). Alex Robenko. All rights reserved.
//

// This file is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#pragma once

#include <cc_tools_qt/Filter.h>
#include <cc_tools_qt/version.h>

#include <cc_mqttsn_client/client.h>

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>

#include <list>
#include <memory>
#include <string>

static_assert(CC_MQTTSN_CLIENT_MAKE_VERSION(2, 0, 0) <= CC_MQTTSN_CLIENT_VERSION, "The version of the cc_mqttsn_client library is too old");
static_assert(CC_TOOLS_QT_MAKE_VERSION(5, 3, 0) <= CC_TOOLS_QT_VERSION, "The version of the cc_tools_qt library is too old");

namespace cc_plugin_mqttsn_client_filter
{

class MqttsnClientFilter final : public QObject, public cc_tools_qt::Filter
{
    Q_OBJECT

public:
    struct SubConfig
    {
        QString m_topic;
        int m_maxQos = 2;
    };

    // erase the element mustn't invalidate references to other elements, using list.
    using SubConfigsList = std::list<SubConfig>; 

    struct Config
    {
        unsigned m_retryPeriod = 0U;
        unsigned m_retryCount = 0U;
        QString m_clientId;
        QString m_pubTopic;
        unsigned m_pubTopicId = 0U;
        int m_pubQos = 0;
        SubConfigsList m_subscribes;
        unsigned m_keepAlive = 60;
        bool m_forcedCleanSession = false;
    };

    MqttsnClientFilter();
    ~MqttsnClientFilter() noexcept;

    Config& config()
    {
        return m_config;
    }

    void forceCleanSession()
    {
        m_firstConnect = true;
    }

signals:
    void sigConfigChanged();    

protected:
    virtual bool startImpl() override;
    virtual void stopImpl() override;
    virtual QList<cc_tools_qt::DataInfoPtr> recvDataImpl(cc_tools_qt::DataInfoPtr dataPtr) override;
    virtual QList<cc_tools_qt::DataInfoPtr> sendDataImpl(cc_tools_qt::DataInfoPtr dataPtr) override;
    virtual void socketConnectionReportImpl(bool connected) override;
    virtual void applyInterPluginConfigImpl(const QVariantMap& props) override;     
    virtual const char* debugNameImpl() const override;

private slots:
    void doTick();

private:
    struct ClientDeleter
    {
        void operator()(CC_MqttsnClient* ptr)
        {
            ::cc_mqttsn_client_free(ptr);
        }
    };
    
    using ClientPtr = std::unique_ptr<CC_MqttsnClient, ClientDeleter>;

    void socketConnected();
    void socketDisconnected();
    void sendPendingData();

    void sendDataInternal(const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius);
    void gwDisconnectedInternal(CC_MqttsnGatewayDisconnectReason reason);
    void messageReceivedInternal(const CC_MqttsnMessageInfo& info);
    void nextTickProgramInternal(unsigned ms);
    unsigned cancelTickProgramInternal();
    void connectCompleteInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info);
    void subscribeCompleteInternal(CC_MqttsnSubscribeHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info);
    void publishCompleteInternal(CC_MqttsnPublishHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnPublishInfo* info);
    

    static void sendDataCb(void* data, const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius);
    static void gwDisconnectedCb(void* data, CC_MqttsnGatewayDisconnectReason reason);
    static void messageReceivedCb(void* data, const CC_MqttsnMessageInfo* info);
    static void nextTickProgramCb(void* data, unsigned ms);
    static unsigned cancelTickProgramCb(void* data);
    static void errorLogCb(void* data, const char* msg);
    static void connectCompleteCb(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info);
    static void disconnectCompleteCb(void* data, CC_MqttsnAsyncOpStatus status);
    static void subscribeCompleteCb(void* data, CC_MqttsnSubscribeHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info);
    static void publishCompleteCb(void* data, CC_MqttsnPublishHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnPublishInfo* info);

    ClientPtr m_client;
    QTimer m_timer;
    std::list<cc_tools_qt::DataInfoPtr> m_pendingData;
    Config m_config;
    std::string m_prevClientId;
    unsigned m_tickMs = 0U;
    qint64 m_tickMeasureTs = 0;
    cc_tools_qt::DataInfoPtr m_recvDataPtr;
    QList<cc_tools_qt::DataInfoPtr> m_recvData;
    cc_tools_qt::DataInfoPtr m_sendDataPtr;
    QList<cc_tools_qt::DataInfoPtr> m_sendData;
    bool m_firstConnect = true;
    bool m_socketConnected = false;
    bool m_cleanSession = false;
};

using MqttsnClientFilterPtr = std::shared_ptr<MqttsnClientFilter>;

inline
MqttsnClientFilterPtr makeMqttsnClientFilter()
{
    return MqttsnClientFilterPtr(new MqttsnClientFilter());
}

}  // namespace cc_plugin_mqttsn_client_filter


