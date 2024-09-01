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

#include "MqttsnClientFilter.h"

#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QVariant>

#include <cassert>
#include <chrono>
#include <cstdint>
#include <limits>
#include <iostream>
#include <string>

namespace cc_plugin_mqttsn_client_filter
{

namespace 
{

inline MqttsnClientFilter* asThis(void* data)
{
    return reinterpret_cast<MqttsnClientFilter*>(data);
}

const QString& topicProp()
{
    static const QString Str("mqttsn.topic");
    return Str;
}

const QString& aliasTopicProp()
{
    static const QString Str("mqtt.topic");
    return Str;
}

const QString& topicIdProp()
{
    static const QString Str("mqttsn.topic_id");
    return Str;
}

const QString& qosProp()
{
    static const QString Str("mqttsn.qos");
    return Str;
}

const QString& aliasQosProp()
{
    static const QString Str("mqtt.qos");
    return Str;
}

const QString& retainedProp()
{
    static const QString Str("mqttsn.retained");
    return Str;    
}

const QString& aliasRetainedProp()
{
    static const QString Str("mqtt.retained");
    return Str;    
}

const QString& clientProp()
{
    static const QString Str("mqttsn.client");
    return Str;    
}

const QString& aliasClientProp()
{
    static const QString Str("mqtt.client");
    return Str;    
}

const QString& pubTopicProp()
{
    static const QString Str("mqttsn.pub_topic");
    return Str;    
}

const QString& aliasPubTopicProp()
{
    static const QString Str("mqtt.pub_topic");
    return Str;    
}

const QString& pubTopicIdProp()
{
    static const QString Str("mqttsn.pub_topic_id");
    return Str;    
}

const QString& pubQosProp()
{
    static const QString Str("mqttsn.pub_qos");
    return Str;    
}

const QString& aliasPubQosProp()
{
    static const QString Str("mqtt.pub_qos");
    return Str;    
}

const QString& subscribesProp()
{
    static const QString Str("mqttsn.subscribes");
    return Str;    
}

const QString& aliasSubscribesProp()
{
    static const QString Str("mqtt.subscribes");
    return Str;    
}

const QString& subscribesRemoveProp()
{
    static const QString Str("mqttsn.subscribes_remove");
    return Str;    
}

const QString& aliasSubscribesRemoveProp()
{
    static const QString Str("mqtt.subscribes_remove");
    return Str;    
}

const QString& subscribesClearProp()
{
    static const QString Str("mqttsn.subscribes_clear");
    return Str;    
}

const QString& aliasSubscribesClearProp()
{
    static const QString Str("mqtt.subscribes_clear");
    return Str;    
}

const QString& topicSubProp()
{
    static const QString Str("topic");
    return Str;
}

const QString& qosSubProp()
{
    static const QString Str("qos");
    return Str;
}

std::string getOutgoingTopic(const QVariantMap& props, const QString configVal)
{
    if (props.contains(topicProp())) {
        return props[topicProp()].value<QString>().toStdString();
    }

    if (props.contains(aliasTopicProp())) {
        return props[aliasTopicProp()].value<QString>().toStdString();
    }

    return configVal.toStdString();
}

unsigned getOutgoingTopicId(const QVariantMap& props, unsigned configVal)
{
    if (props.contains(topicIdProp())) {
        return props[topicIdProp()].value<unsigned>();
    }    

    return configVal;
}

int getOutgoingQos(const QVariantMap& props, int configVal)
{
    if (props.contains(qosProp())) {
        return props[qosProp()].value<int>();
    }

    if (props.contains(aliasQosProp())) {
        return props[aliasQosProp()].value<int>();
    }

    return configVal;
}

bool getOutgoingRetained(const QVariantMap& props)
{
    if (props.contains(retainedProp())) {
        return props[retainedProp()].value<bool>();
    }

    if (props.contains(aliasRetainedProp())) {
        return props[aliasRetainedProp()].value<bool>();
    }    

    return false;
}

const QString& errorCodeStr(CC_MqttsnErrorCode ec)
{
    static const QString Map[] = {
        /* CC_MqttsnErrorCode_Success */ "Success",
        /* CC_MqttsnErrorCode_InternalError */ "Internal Error",
        /* CC_MqttsnErrorCode_NotIntitialized */ "Not Initialized",
        /* CC_MqttsnErrorCode_Busy */ "Busy",
        /* CC_MqttsnErrorCode_NotConnected */ "Not Connected",
        /* CC_MqttsnErrorCode_BadParam */ "Bad Parameter",
        /* CC_MqttsnErrorCode_InsufficientConfig */ "Insufficient Config",
        /* CC_MqttsnErrorCode_OutOfMemory */ "Out of Memory",
        /* CC_MqttsnErrorCode_BufferOverflow */ "Buffer Overflow",
        /* CC_MqttsnErrorCode_NotSupported */ "Feature is Not Supported",
        /* CC_MqttsnErrorCode_RetryLater */ "Retry later",
        /* CC_MqttsnErrorCode_Disconnecting */ "Disconnecting",
        /* CC_MqttsnErrorCode_NotSleeping */ "Not Sleeping",
        /* CC_MqttsnErrorCode_PreparationLocked */ "Preparation Locked",
    };

    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == CC_MqttsnErrorCode_ValuesLimit);

    auto idx = static_cast<unsigned>(ec);
    if (MapSize <= idx) {
        static const QString UnknownStr("Unknown");
        return UnknownStr;
    }

    return Map[idx];
}

const QString& statusStr(CC_MqttsnAsyncOpStatus status)
{
    static const QString Map[] = {
        /* CC_MqttsnAsyncOpStatus_Complete */ "Complete",
        /* CC_MqttsnAsyncOpStatus_InternalError */ "Internal Error",
        /* CC_MqttsnAsyncOpStatus_Timeout */ "Timeout",
        /* CC_MqttsnAsyncOpStatus_Aborted */ "Aborted",
        /* CC_MqttsnAsyncOpStatus_OutOfMemory */ "Out of Memory",
        /* CC_MqttsnAsyncOpStatus_BadParam */ "Bad Parameter",
        /* CC_MqttsnAsyncOpStatus_GatewayDisconnected */ "Gateway Disconnected",
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == CC_MqttsnAsyncOpStatus_ValuesLimit);

    auto idx = static_cast<unsigned>(status);
    if (MapSize <= idx) {
        static const QString UnknownStr("Unknown");
        return UnknownStr;
    }

    return Map[idx];
}

const QString& returnCodeStr(CC_MqttsnReturnCode value)
{
    static const QString Map[] = {
        /* CC_MqttsnReturnCode_Accepted */ "Accepted",
        /* CC_MqttsnReturnCode_Conjestion */ "Conjestion",
        /* CC_MqttsnReturnCode_InvalidTopicId */ "Invalid Topic ID",
        /* CC_MqttsnReturnCode_NotSupported */ "Not Supported",
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == CC_MqttsnReturnCode_ValuesLimit);

    auto idx = static_cast<unsigned>(value);
    if (MapSize <= idx) {
        static const QString UnknownStr("Unknown");
        return UnknownStr;
    }

    return Map[idx];
}

const QString& disconnectReasonStr(CC_MqttsnGatewayDisconnectReason value)
{
    static const QString Map[] = {
        /* CC_MqttsnGatewayDisconnectReason_DisconnectMsg */ "DISCONNECT Message",
        /* CC_MqttsnGatewayDisconnectReason_NoGatewayResponse */ "No Response",
    };
    static const std::size_t MapSize = std::extent<decltype(Map)>::value;
    static_assert(MapSize == CC_MqttsnGatewayDisconnectReason_ValuesLimit);

    auto idx = static_cast<unsigned>(value);
    if (MapSize <= idx) {
        static const QString UnknownStr("Unknown");
        return UnknownStr;
    }

    return Map[idx];    
}

} // namespace 
    

MqttsnClientFilter::MqttsnClientFilter() :
    m_client(::cc_mqttsn_client_alloc())
{
    m_timer.setSingleShot(true);
    connect(
        &m_timer, &QTimer::timeout,
        this, &MqttsnClientFilter::doTick);

    ::cc_mqttsn_client_set_send_output_data_callback(m_client.get(), &MqttsnClientFilter::sendDataCb, this);
    ::cc_mqttsn_client_set_gw_disconnect_report_callback(m_client.get(), &MqttsnClientFilter::gwDisconnectedCb, this);
    ::cc_mqttsn_client_set_message_report_callback(m_client.get(), &MqttsnClientFilter::messageReceivedCb, this);
    ::cc_mqttsn_client_set_next_tick_program_callback(m_client.get(), &MqttsnClientFilter::nextTickProgramCb, this);
    ::cc_mqttsn_client_set_cancel_next_tick_wait_callback(m_client.get(), &MqttsnClientFilter::cancelTickProgramCb, this);
    ::cc_mqttsn_client_set_error_log_callback(m_client.get(), &MqttsnClientFilter::errorLogCb, nullptr);

    m_config.m_retryPeriod = ::cc_mqttsn_client_get_default_retry_period(m_client.get());
    m_config.m_retryCount = ::cc_mqttsn_client_get_default_retry_count(m_client.get());
}

MqttsnClientFilter::~MqttsnClientFilter() noexcept = default;

bool MqttsnClientFilter::startImpl()
{
    auto ec = ::cc_mqttsn_client_set_default_retry_period(m_client.get(), m_config.m_retryPeriod);
    if (ec != CC_MqttsnErrorCode_Success) {
        reportError(tr("Failed to update MQTT-SN default retry period"));
        return false;
    }  

    ec = ::cc_mqttsn_client_set_default_retry_count(m_client.get(), m_config.m_retryCount);
    if (ec != CC_MqttsnErrorCode_Success) {
        reportError(tr("Failed to update MQTT-SN default retry count"));
        return false;
    }      

    return true; 
}

void MqttsnClientFilter::stopImpl()
{
    if (::cc_mqttsn_client_get_connection_status(m_client.get()) != CC_MqttsnConnectionStatus_Connected) {
        return;
    }

    auto ec = cc_mqttsn_client_disconnect(m_client.get(), &MqttsnClientFilter::disconnectCompleteCb, this);
    if (ec != CC_MqttsnErrorCode_Success) {
        reportError(tr("Failed to send disconnect with error: ") + errorCodeStr(ec));
        return;
    }    
}

QList<cc_tools_qt::DataInfoPtr> MqttsnClientFilter::recvDataImpl(cc_tools_qt::DataInfoPtr dataPtr)
{
    m_recvData.clear();
    m_recvDataPtr = std::move(dataPtr);
    ::cc_mqttsn_client_process_data(m_client.get(), m_recvDataPtr->m_data.data(), static_cast<unsigned>(m_recvDataPtr->m_data.size()), CC_MqttsnDataOrigin_ConnectedGw);
    m_recvDataPtr.reset();
    return std::move(m_recvData);
}

QList<cc_tools_qt::DataInfoPtr> MqttsnClientFilter::sendDataImpl(cc_tools_qt::DataInfoPtr dataPtr)
{
    m_sendData.clear();

    if (!m_socketConnected) {
        reportError(tr("Cannot send MQTTSN data when socket is not connected"));
        return m_sendData;
    }

    if (::cc_mqttsn_client_get_connection_status(m_client.get()) != CC_MqttsnConnectionStatus_Connected) {
        m_pendingData.push_back(std::move(dataPtr));
        return m_sendData;
    }

    auto& props = dataPtr->m_extraProperties;
    std::string topic = getOutgoingTopic(props, QString());
    auto topicId = getOutgoingTopicId(props, 0U);

    if (topic.empty() && (topicId == 0U)) {
        topic = getOutgoingTopic(props, m_config.m_pubTopic);
        topicId = getOutgoingTopicId(props, m_config.m_pubTopicId);        
    }
    
    auto qos = getOutgoingQos(props, m_config.m_pubQos);
    props[qosProp()] = qos;

    auto retained = getOutgoingRetained(props);
    props[retainedProp()] = retained;

    if (2 <= getDebugOutputLevel()) {
        std::cout << '[' << currTimestamp() << "] (" << debugNameImpl() << "): publish: " << topic << std::endl;
    }    

    auto config = CC_MqttsnPublishConfig();
    ::cc_mqttsn_client_publish_init_config(&config);

    if (!topic.empty()) {
        config.m_topic = topic.c_str();
        props[topicProp()] = QString::fromStdString(topic);
    }
    else if (topicId != 0U) {
        config.m_topicId = static_cast<decltype(config.m_topicId)>(topicId);
        props[topicIdProp()] = topicId;
    }
    config.m_data = dataPtr->m_data.data();
    config.m_dataLen = static_cast<decltype(config.m_dataLen)>(dataPtr->m_data.size());
    config.m_qos = static_cast<decltype(config.m_qos)>(qos);    
    config.m_retain = retained;

    m_sendDataPtr = std::move(dataPtr);

    if (2 <= getDebugOutputLevel()) {
        std::cout << '[' << currTimestamp() << "] (" << debugNameImpl() << "): initiating publish" << std::endl;
    }    

    auto ec = ::cc_mqttsn_client_publish(m_client.get(), &config, &MqttsnClientFilter::publishCompleteCb, this);
    if (ec != CC_MqttsnErrorCode_Success) {
        reportError(tr("Failed to send MQTTSN publish with error: ") + errorCodeStr(ec));
        m_sendDataPtr.reset();
        return m_sendData;        
    }

    m_sendDataPtr.reset();
    return std::move(m_sendData);
}

void MqttsnClientFilter::socketConnectionReportImpl(bool connected)
{
    // if ((!connected) && (!m_socketConnected)) {
    //     return;
    // }

    m_socketConnected = connected;
    if (connected) {
        socketConnected();
        return;
    }

    socketDisconnected();
}

void MqttsnClientFilter::applyInterPluginConfigImpl(const QVariantMap& props)
{
    bool updated = false;

    {
        static const QString* ClientProps[] = {
            &aliasClientProp(),
            &clientProp(),
        };

        for (auto* p : ClientProps) {
            auto var = props.value(*p);
            if ((var.isValid()) && (var.canConvert<QString>())) {
                m_config.m_clientId = var.value<QString>();
                updated = true;
            }
        }
    }

    {
        static const QString* PubTopicProps[] = {
            &aliasPubTopicProp(),
            &pubTopicProp(),
        };

        for (auto* p : PubTopicProps) {
            auto var = props.value(*p);
            if ((var.isValid()) && (var.canConvert<QString>())) {
                m_config.m_pubTopic = var.value<QString>();
                if (!m_config.m_pubTopic.isEmpty()) {
                    m_config.m_pubTopicId = 0;
                }
                updated = true;
            }
        }  
    }  

    {
        static const QString* PubTopicIdProps[] = {
            &pubTopicIdProp(),
        };

        for (auto* p : PubTopicIdProps) {
            auto var = props.value(*p);
            if ((var.isValid()) && (var.canConvert<unsigned>())) {
                m_config.m_pubTopicId = var.value<unsigned>();
                if (m_config.m_pubTopicId == 0) {
                    m_config.m_pubTopic.clear();
                }
                updated = true;
            }
        }  
    }      

    {
        static const QString* PubQosProps[] = {
            &aliasPubQosProp(),
            &pubQosProp(),
        };

        for (auto* p : PubQosProps) {
            auto var = props.value(*p);
            if ((var.isValid()) && (var.canConvert<int>())) {
                m_config.m_pubQos = var.value<int>();
                updated = true;
            }
        }  
    }  

    {
        static const QString* SubscribesRemoveProps[] = {
            &aliasSubscribesRemoveProp(),
            &subscribesRemoveProp(),
        };

        for (auto* p : SubscribesRemoveProps) {
            auto var = props.value(*p);
            if ((!var.isValid()) || (!var.canConvert<QVariantList>())) {
                continue;
            }

            auto subList = var.value<QVariantList>();

            for (auto idx = 0; idx < subList.size(); ++idx) {
                auto& subVar = subList[idx];
                if ((!subVar.isValid()) || (!subVar.canConvert<QVariantMap>())) {
                    continue;
                }

                auto subMap = subVar.value<QVariantMap>();
                auto topicVar = subMap.value(topicSubProp());
                if ((!topicVar.isValid()) || (!topicVar.canConvert<QString>())) {
                    continue;
                }

                auto topic = topicVar.value<QString>();

                auto iter = 
                    std::find_if(
                        m_config.m_subscribes.begin(), m_config.m_subscribes.end(),
                        [&topic](const auto& info)
                        {
                            return topic == info.m_topic;
                        });
                
                if (iter != m_config.m_subscribes.end()) {
                    m_config.m_subscribes.erase(iter);
                    updated = true;
                    forceCleanSession();                    
                }
            }
        }  
    }  

    {
        static const QString* SubscribesClearProps[] = {
            &aliasSubscribesClearProp(),
            &subscribesClearProp(),
        };

        for (auto* p : SubscribesClearProps) {
            auto var = props.value(*p);
            if ((!var.isValid()) || (!var.canConvert<bool>())) {
                continue;
            }

            if ((!var.value<bool>()) || (m_config.m_subscribes.empty())) {
                continue;
            }

            m_config.m_subscribes.clear();
            updated = true;
        }  
    }           

    {
        static const QString* SubscribesProps[] = {
            &aliasSubscribesProp(),
            &subscribesProp(),
        };

        for (auto* p : SubscribesProps) {
            auto var = props.value(*p);
            if ((!var.isValid()) || (!var.canConvert<QVariantList>())) {
                continue;
            }

            auto subList = var.value<QVariantList>();

            for (auto idx = 0; idx < subList.size(); ++idx) {
                auto& subVar = subList[idx];
                if ((!subVar.isValid()) || (!subVar.canConvert<QVariantMap>())) {
                    continue;
                }

                auto subMap = subVar.value<QVariantMap>();
                auto topicVar = subMap.value(topicSubProp());
                if ((!topicVar.isValid()) || (!topicVar.canConvert<QString>())) {
                    continue;
                }

                auto topic = topicVar.value<QString>();

                auto iter = 
                    std::find_if(
                        m_config.m_subscribes.begin(), m_config.m_subscribes.end(),
                        [&topic](const auto& info)
                        {
                            return topic == info.m_topic;
                        });
                
                if (iter == m_config.m_subscribes.end()) {
                    iter = m_config.m_subscribes.insert(m_config.m_subscribes.end(), SubConfig());
                    iter->m_topic = topic;
                }

                auto& subConfig = *iter;
                auto qosVar = subMap.value(qosSubProp());
                if (qosVar.isValid() && qosVar.canConvert<int>()) {
                    subConfig.m_maxQos = qosVar.value<int>();
                }
            }
            
            updated = true;
            forceCleanSession();
        }  
    }              

    if (updated) {
        emit sigConfigChanged();
    }
}

const char* MqttsnClientFilter::debugNameImpl() const
{
    return "mqtt-sn client filter";
}

void MqttsnClientFilter::doTick()
{
    assert(m_tickMeasureTs > 0);
    m_tickMeasureTs = 0;

    assert(m_client);
    if (!m_client) {
        return;
    }

    ::cc_mqttsn_client_tick(m_client.get(), m_tickMs);
}

void MqttsnClientFilter::socketConnected()
{
    if (2 <= getDebugOutputLevel()) {
        std::cout << '[' << currTimestamp() << "] (" << debugNameImpl() << "): socket connected report" << std::endl;
    }
        
    auto config = CC_MqttsnConnectConfig();
    ::cc_mqttsn_client_connect_init_config(&config);

    auto clientId = m_config.m_clientId.toStdString();
    
    if (!clientId.empty()) {
        config.m_clientId = clientId.c_str();
    }

    config.m_duration = m_config.m_keepAlive;
    config.m_cleanSession = 
        (m_config.m_forcedCleanSession) ||
        (clientId.empty()) || 
        (clientId != m_prevClientId) ||
        (m_firstConnect);

    auto ec = 
        cc_mqttsn_client_connect(
            m_client.get(), 
            &config, 
            nullptr, 
            &MqttsnClientFilter::connectCompleteCb, 
            this);

    if (ec != CC_MqttsnErrorCode_Success) {
        reportError(tr("Failed to initiate MQTT-SN connection"));
        return;
    }    

    m_prevClientId = clientId;
    m_cleanSession = config.m_cleanSession;
}

void MqttsnClientFilter::socketDisconnected()
{
    if (2 <= getDebugOutputLevel()) {
        std::cout << '[' << currTimestamp() << "] (" << debugNameImpl() << "): socket disconnected report" << std::endl;
    }
}

void MqttsnClientFilter::sendPendingData()
{
    for (auto& dataPtr : m_pendingData) {
        sendDataImpl(std::move(dataPtr));
    }
    m_pendingData.clear();
}

void MqttsnClientFilter::sendDataInternal(const unsigned char* buf, unsigned bufLen, [[maybe_unused]] unsigned broadcastRadius)
{
    if (3 <= getDebugOutputLevel()) {
        std::cout << '[' << currTimestamp() << "] (" << debugNameImpl() << "): sending " << bufLen << " bytes" << std::endl;
    }

    auto dataInfo = cc_tools_qt::makeDataInfoTimed();
    dataInfo->m_data.assign(buf, buf + bufLen);
    if (!m_sendDataPtr) {
        reportDataToSend(std::move(dataInfo));
        return;
    }

    dataInfo->m_extraProperties = m_sendDataPtr->m_extraProperties;
    m_sendData.append(std::move(dataInfo));
}

void MqttsnClientFilter::gwDisconnectedInternal(CC_MqttsnGatewayDisconnectReason reason)
{
    if (2 <= getDebugOutputLevel()) {
        std::cout << '[' << currTimestamp() << "] (" << debugNameImpl() << "): gateway disconnected: " <<  disconnectReasonStr(reason).toStdString() << std::endl;
    }

    auto gatewayDisconnecteError = 
        tr("MQTTSN gateway is disconnected with reason: ") + disconnectReasonStr(reason);

    reportError(gatewayDisconnecteError);
}

void MqttsnClientFilter::messageReceivedInternal(const CC_MqttsnMessageInfo& info)
{
    if (2 <= getDebugOutputLevel()) {
        std::cout << '[' << currTimestamp() << "] (" << debugNameImpl() << "): app message received: " << info.m_topic << std::endl;
    }

    assert(m_recvDataPtr);
    auto dataInfo = cc_tools_qt::makeDataInfoTimed();
    if (info.m_dataLen > 0U) {
        dataInfo->m_data.assign(info.m_data, info.m_data + info.m_dataLen);
    }
    auto& props = dataInfo->m_extraProperties;
    props = m_recvDataPtr->m_extraProperties;
    assert(info.m_topic != nullptr);
    props[topicProp()] = info.m_topic;
    props[qosProp()] = static_cast<int>(info.m_qos);
    props[retainedProp()] = info.m_retained;
    m_recvData.append(std::move(dataInfo));
}

void MqttsnClientFilter::nextTickProgramInternal(unsigned ms)
{
    if (3 <= getDebugOutputLevel()) {
        std::cout << '[' << currTimestamp() << "] (" << debugNameImpl() << "): tick request: " << ms << std::endl;
    }

    assert(!m_timer.isActive());
    m_tickMs = ms;
    m_tickMeasureTs = QDateTime::currentMSecsSinceEpoch();
    m_timer.start(static_cast<int>(ms));
}

unsigned MqttsnClientFilter::cancelTickProgramInternal()
{
    assert(m_tickMeasureTs > 0);
    assert(m_timer.isActive());
    m_timer.stop();
    auto now = QDateTime::currentMSecsSinceEpoch();
    assert(m_tickMeasureTs <= now);
    auto diff = now - m_tickMeasureTs;
    assert(diff < std::numeric_limits<unsigned>::max());
    m_tickMeasureTs = 0U;

    if (3 <= getDebugOutputLevel()) {
        std::cout << '[' << currTimestamp() << "] (" << debugNameImpl() << "): cancel tick: " << diff << std::endl;
    }
        
    return static_cast<unsigned>(diff);
}

void MqttsnClientFilter::connectCompleteInternal(CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info)
{
    if (status != CC_MqttsnAsyncOpStatus_Complete) {
        reportError(tr("Failed to connect to MQTTSN gateway with status: ") + statusStr(status));
        return;
    }

    assert(info != nullptr);
    if (info->m_returnCode != CC_MqttsnReturnCode_Accepted) {
        reportError(tr("MQTT gateway rejected connection with return code: ") + returnCodeStr(info->m_returnCode));
        return;        
    }

    m_firstConnect = false;

    sendPendingData();

    if (!m_cleanSession) {
        return;
    }

    if (m_config.m_subscribes.empty()) {
        return;
    }

    for (auto& sub : m_config.m_subscribes) {
        auto topicStr = sub.m_topic.trimmed().toStdString();

        auto config = CC_MqttsnSubscribeConfig();
        ::cc_mqttsn_client_subscribe_init_config(&config);
        if (!topicStr.empty()) {
            config.m_topic = topicStr.c_str();
        }
        config.m_topicId = static_cast<decltype(config.m_topicId)>(sub.m_topicId);
        config.m_qos = static_cast<decltype(config.m_qos)>(sub.m_maxQos);

        auto ec = cc_mqttsn_client_subscribe(m_client.get(), &config, &MqttsnClientFilter::subscribeCompleteCb, this);
        if (ec != CC_MqttsnErrorCode_Success) {
            reportError(tr("Failed to send MQTTSN SUBSCRIBE message for topic: ") + config.m_topic);
            continue;
        }         
    }
}

void MqttsnClientFilter::subscribeCompleteInternal([[maybe_unused]] CC_MqttsnSubscribeHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info)
{
    if (status != CC_MqttsnAsyncOpStatus_Complete) {
        reportError(tr("Failed to subsribe to MQTTSN topic with status: ") + statusStr(status));
        return;
    }  

    assert (info != nullptr);
    if (info->m_returnCode != CC_MqttsnReturnCode_Accepted) {
        reportError(tr("MQTT gateway rejected subscribe with return code: ") + returnCodeStr(info->m_returnCode));
    }
}

void MqttsnClientFilter::publishCompleteInternal([[maybe_unused]] CC_MqttsnPublishHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnPublishInfo* info)
{
    if (2 <= getDebugOutputLevel()) {
        std::cout << '[' << currTimestamp() << "] (" << debugNameImpl() << "): publish complete with status: " << statusStr(status).toStdString() << std::endl;
    }  

    if (status != CC_MqttsnAsyncOpStatus_Complete) {
        reportError(tr("Failed to publish to MQTTSN gateway with status: ") + statusStr(status));
        return;
    }

    if ((info != nullptr) && (info->m_returnCode != CC_MqttsnReturnCode_Accepted)) {
        reportError(tr("Publish rejected with return code: ") + returnCodeStr(info->m_returnCode));
        return;        
    }
}

void MqttsnClientFilter::sendDataCb(void* data, const unsigned char* buf, unsigned bufLen, unsigned broadcastRadius)
{
    asThis(data)->sendDataInternal(buf, bufLen, broadcastRadius);
}

void MqttsnClientFilter::gwDisconnectedCb(void* data, CC_MqttsnGatewayDisconnectReason reason)
{
    asThis(data)->gwDisconnectedInternal(reason);
}

void MqttsnClientFilter::messageReceivedCb(void* data, const CC_MqttsnMessageInfo* info)
{
    assert(info != nullptr);
    if (info == nullptr) {
        return;
    }

    asThis(data)->messageReceivedInternal(*info);
}

void MqttsnClientFilter::nextTickProgramCb(void* data, unsigned ms)
{
    asThis(data)->nextTickProgramInternal(ms);
}

unsigned MqttsnClientFilter::cancelTickProgramCb(void* data)
{
    return asThis(data)->cancelTickProgramInternal();
}

void MqttsnClientFilter::errorLogCb([[maybe_unused]] void* data, const char* msg)
{
    auto timestamp = std::chrono::high_resolution_clock::now();
    auto sinceEpoch = timestamp.time_since_epoch();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(sinceEpoch).count();
    std::cerr << '[' << milliseconds << "] MQTT ERROR: " << msg << std::endl;
}

void MqttsnClientFilter::connectCompleteCb(void* data, CC_MqttsnAsyncOpStatus status, const CC_MqttsnConnectInfo* info)
{
    asThis(data)->connectCompleteInternal(status, info);
}

void MqttsnClientFilter::disconnectCompleteCb([[maybe_unused]] void* data, [[maybe_unused]] CC_MqttsnAsyncOpStatus status)
{
}

void MqttsnClientFilter::subscribeCompleteCb(void* data, CC_MqttsnSubscribeHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnSubscribeInfo* info)
{
    asThis(data)->subscribeCompleteInternal(handle, status, info);
}

void MqttsnClientFilter::publishCompleteCb(void* data, CC_MqttsnPublishHandle handle, CC_MqttsnAsyncOpStatus status, const CC_MqttsnPublishInfo* info)
{
    asThis(data)->publishCompleteInternal(handle, status, info);
}

}  // namespace cc_plugin_mqttsn_client_filter


