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

#include "ui_MqttsnClientFilterConfigWidget.h"

#include "MqttsnClientFilter.h"

#include <QtWidgets/QWidget>


namespace cc_plugin_mqttsn_client_filter
{

class MqttsnClientFilterConfigWidget : public QWidget
{
    Q_OBJECT
    using Base = QWidget;

public:
    explicit MqttsnClientFilterConfigWidget(MqttsnClientFilter& filter, QWidget* parentObj = nullptr);
    ~MqttsnClientFilterConfigWidget() noexcept;

private slots:
    void refresh();
    void retryPeriodUpdated(int val);
    void retryCountUpdated(int val);
    void clientIdUpdated(const QString& val);
    void keepAliveUpdated(int val);
    void forcedCleanSessionUpdated(int val);
    void pubTopicUpdated(const QString& val);
    void pubTopicIdUpdated(int val);
    void pubQosUpdated(int val);
    void addSubscribe();

private:
    using SubConfig = MqttsnClientFilter::SubConfig;

    void refreshPubTopic();
    void refreshSubscribes();
    void addSubscribeWidget(SubConfig& config);    

    MqttsnClientFilter& m_filter;
    Ui::MqttsnClientFilterConfigWidget m_ui;
};

}  // namespace cc_plugin_mqttsn_client_filter


